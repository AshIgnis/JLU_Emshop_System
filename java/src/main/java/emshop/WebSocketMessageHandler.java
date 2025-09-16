package emshop;

import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.SimpleChannelInboundHandler;
import io.netty.handler.codec.http.websocketx.*;
import io.netty.handler.timeout.IdleState;
import io.netty.handler.timeout.IdleStateEvent;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.JsonNode;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

/**
 * WebSocket 消息处理器
 * 处理来自 Qt 客户端的 WebSocket 消息
 */
public class WebSocketMessageHandler extends SimpleChannelInboundHandler<WebSocketFrame> {
    
    private static final ObjectMapper objectMapper = new ObjectMapper();
    private static final ExecutorService businessExecutor = Executors.newFixedThreadPool(10);
    
    @Override
    public void channelActive(ChannelHandlerContext ctx) {
        System.out.println("WebSocket client connected: " + ctx.channel().remoteAddress());
        // 初始化连接属性
        ctx.channel().attr(EmshopWebSocketServer.AUTHENTICATED_ATTR).set(false);
        // 发送欢迎消息
        sendMessage(ctx, createResponse("welcome", true, "Connected to Emshop WebSocket Server", null));
    }
    
    @Override
    public void channelInactive(ChannelHandlerContext ctx) {
        Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
        String username = ctx.channel().attr(EmshopWebSocketServer.USERNAME_ATTR).get();
        
        System.out.println("WebSocket client disconnected: " + ctx.channel().remoteAddress());
        
        if (userId != null) {
            EmshopWebSocketServer.removeUserChannel(userId, ctx.channel());
            System.out.println("User " + username + " (ID: " + userId + ") disconnected");
            
            // 通知其他用户该用户下线（可选功能）
            broadcastUserStatus(userId, username, "offline");
        }
    }
    
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, WebSocketFrame frame) {
        if (frame instanceof TextWebSocketFrame) {
            handleTextFrame(ctx, (TextWebSocketFrame) frame);
        } else if (frame instanceof PingWebSocketFrame) {
            // 响应 ping 帧
            ctx.writeAndFlush(new PongWebSocketFrame(frame.content().retain()));
        } else if (frame instanceof CloseWebSocketFrame) {
            ctx.close();
        }
    }
    
    /**
     * 处理文本消息帧
     */
    private void handleTextFrame(ChannelHandlerContext ctx, TextWebSocketFrame frame) {
        String message = frame.text();
        
        try {
            JsonNode jsonNode = objectMapper.readTree(message);
            String type = jsonNode.get("type").asText();
            
            System.out.println("Received message type: " + type + " from " + ctx.channel().remoteAddress());
            
            // 使用异步处理业务逻辑，避免阻塞 I/O 线程
            CompletableFuture.supplyAsync(() -> processMessage(ctx, type, jsonNode), businessExecutor)
                .thenAccept(response -> {
                    if (response != null) {
                        sendMessage(ctx, response);
                    }
                })
                .exceptionally(throwable -> {
                    System.err.println("Error processing message: " + throwable.getMessage());
                    sendMessage(ctx, createResponse(type, false, 
                        "Internal server error: " + throwable.getMessage(), null));
                    return null;
                });
                
        } catch (Exception e) {
            System.err.println("Failed to parse message: " + e.getMessage());
            sendMessage(ctx, createResponse("error", false, "Invalid JSON format", null));
        }
    }
    
    /**
     * 处理业务消息
     */
    private String processMessage(ChannelHandlerContext ctx, String type, JsonNode jsonNode) {
        Boolean authenticated = ctx.channel().attr(EmshopWebSocketServer.AUTHENTICATED_ATTR).get();
        
        // 检查认证状态（除了认证相关的消息）
        if (!Boolean.TRUE.equals(authenticated) && !isPublicMessage(type)) {
            return createResponse(type, false, "Authentication required", null);
        }
        
        switch (type) {
            case "auth":
                return handleAuth(ctx, jsonNode);
            case "ping":
                return createResponse("pong", true, "pong", 
                    "{\"timestamp\":" + System.currentTimeMillis() + "}");
            case "get_products":
                return handleGetProducts(jsonNode);
            case "search_products":
                return handleSearchProducts(jsonNode);
            case "add_to_cart":
                return handleAddToCart(ctx, jsonNode);
            case "get_cart":
                return handleGetCart(ctx, jsonNode);
            case "remove_from_cart":
                return handleRemoveFromCart(ctx, jsonNode);
            case "create_order":
                return handleCreateOrder(ctx, jsonNode);
            case "get_user_orders":
                return handleGetUserOrders(ctx, jsonNode);
            case "get_order_detail":
                return handleGetOrderDetail(jsonNode);
            case "process_payment":
                return handleProcessPayment(jsonNode);
            case "get_user_addresses":
                return handleGetUserAddresses(ctx, jsonNode);
            case "add_address":
                return handleAddAddress(ctx, jsonNode);
            default:
                return createResponse(type, false, "Unknown message type", null);
        }
    }
    
    /**
     * 检查是否为公共消息（不需要认证）
     */
    private boolean isPublicMessage(String type) {
        return "auth".equals(type) || "ping".equals(type) || "register".equals(type);
    }
    
    /**
     * 处理认证消息
     */
    private String handleAuth(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            String username = jsonNode.get("username").asText();
            String password = jsonNode.get("password").asText();
            
            // 调用 JNI 接口进行认证
            String loginResult = EmshopNativeInterface.login(username, password);
            JsonNode loginResponse = objectMapper.readTree(loginResult);
            
            if (loginResponse.get("success").asBoolean()) {
                // 认证成功，设置用户信息
                long userId = loginResponse.get("data").get("user_id").asLong();
                String role = "user"; // 默认角色
                
                // 尝试获取角色信息
                if (loginResponse.get("data").has("user_info") && 
                    loginResponse.get("data").get("user_info").has("role")) {
                    role = loginResponse.get("data").get("user_info").get("role").asText();
                }
                
                // 设置连接属性
                ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).set(userId);
                ctx.channel().attr(EmshopWebSocketServer.USERNAME_ATTR).set(username);
                ctx.channel().attr(EmshopWebSocketServer.ROLE_ATTR).set(role);
                ctx.channel().attr(EmshopWebSocketServer.AUTHENTICATED_ATTR).set(true);
                
                // 添加到用户连接管理
                EmshopWebSocketServer.addUserChannel(userId, ctx.channel());
                
                System.out.println("User " + username + " (ID: " + userId + ", Role: " + role + ") authenticated successfully");
                
                // 通知其他用户该用户上线（可选功能）
                broadcastUserStatus(userId, username, "online");
                
                // 返回认证成功响应
                String userData = String.format("{\"user_id\":%d,\"username\":\"%s\",\"role\":\"%s\"}", 
                    userId, username, role);
                return createResponse("auth", true, "Authentication successful", userData);
            } else {
                return createResponse("auth", false, "Authentication failed", null);
            }
            
        } catch (Exception e) {
            System.err.println("Authentication error: " + e.getMessage());
            return createResponse("auth", false, "Authentication error", null);
        }
    }
    
    /**
     * 处理获取商品列表
     */
    private String handleGetProducts(JsonNode jsonNode) {
        try {
            String category = jsonNode.has("category") ? jsonNode.get("category").asText() : "all";
            int page = jsonNode.has("page") ? jsonNode.get("page").asInt() : 1;
            int pageSize = jsonNode.has("pageSize") ? jsonNode.get("pageSize").asInt() : 10;
            
            String result = EmshopNativeInterface.getProductList(category, page, pageSize);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("get_products", true, "Products retrieved successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("get_products", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("get_products", false, "Error retrieving products", null);
        }
    }
    
    /**
     * 处理搜索商品
     */
    private String handleSearchProducts(JsonNode jsonNode) {
        try {
            String keyword = jsonNode.get("keyword").asText();
            int page = jsonNode.has("page") ? jsonNode.get("page").asInt() : 1;
            int pageSize = jsonNode.has("pageSize") ? jsonNode.get("pageSize").asInt() : 10;
            String sortBy = jsonNode.has("sortBy") ? jsonNode.get("sortBy").asText() : "id";
            double minPrice = jsonNode.has("minPrice") ? jsonNode.get("minPrice").asDouble() : 0.0;
            double maxPrice = jsonNode.has("maxPrice") ? jsonNode.get("maxPrice").asDouble() : 99999.0;
            
            String result = EmshopNativeInterface.searchProducts(keyword, page, pageSize, sortBy, minPrice, maxPrice);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("search_products", true, "Search completed successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("search_products", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("search_products", false, "Error searching products", null);
        }
    }
    
    /**
     * 处理添加到购物车
     */
    private String handleAddToCart(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            long productId = jsonNode.get("productId").asLong();
            int quantity = jsonNode.get("quantity").asInt();
            
            String result = EmshopNativeInterface.addToCart(userId, productId, quantity);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                // 可以向其他相关用户推送购物车更新通知
                return createResponse("add_to_cart", true, "Product added to cart successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("add_to_cart", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("add_to_cart", false, "Error adding to cart", null);
        }
    }
    
    /**
     * 处理获取购物车
     */
    private String handleGetCart(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            String result = EmshopNativeInterface.getCart(userId);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("get_cart", true, "Cart retrieved successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("get_cart", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("get_cart", false, "Error retrieving cart", null);
        }
    }
    
    /**
     * 处理从购物车移除商品
     */
    private String handleRemoveFromCart(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            long productId = jsonNode.get("productId").asLong();
            
            String result = EmshopNativeInterface.removeFromCart(userId, productId);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("remove_from_cart", true, "Product removed from cart successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("remove_from_cart", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("remove_from_cart", false, "Error removing from cart", null);
        }
    }
    
    /**
     * 处理创建订单
     */
    private String handleCreateOrder(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            long addressId = jsonNode.get("addressId").asLong();
            String couponCode = jsonNode.has("couponCode") ? jsonNode.get("couponCode").asText() : null;
            String remark = jsonNode.has("remark") ? jsonNode.get("remark").asText() : "";
            
            String result = EmshopNativeInterface.createOrderFromCart(userId, addressId, couponCode, remark);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                // 可以推送订单创建通知给管理员
                return createResponse("create_order", true, "Order created successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("create_order", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("create_order", false, "Error creating order", null);
        }
    }
    
    /**
     * 处理获取用户订单
     */
    private String handleGetUserOrders(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            String result = EmshopNativeInterface.getOrderList(userId);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("get_user_orders", true, "Orders retrieved successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("get_user_orders", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("get_user_orders", false, "Error retrieving orders", null);
        }
    }
    
    /**
     * 处理获取订单详情
     */
    private String handleGetOrderDetail(JsonNode jsonNode) {
        try {
            long orderId = jsonNode.get("orderId").asLong();
            String result = EmshopNativeInterface.getOrderDetail(orderId);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("get_order_detail", true, "Order detail retrieved successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("get_order_detail", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("get_order_detail", false, "Error retrieving order detail", null);
        }
    }
    
    /**
     * 处理支付处理
     */
    private String handleProcessPayment(JsonNode jsonNode) {
        try {
            long orderId = jsonNode.get("orderId").asLong();
            String paymentMethod = jsonNode.get("paymentMethod").asText();
            double amount = jsonNode.get("amount").asDouble();
            String paymentDetails = jsonNode.has("paymentDetails") ? 
                jsonNode.get("paymentDetails").toString() : "{}";
            
            String result = EmshopNativeInterface.processPayment(orderId, paymentMethod, amount, paymentDetails);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                // 支付成功后可以推送通知给相关用户
                return createResponse("process_payment", true, "Payment processed successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("process_payment", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("process_payment", false, "Error processing payment", null);
        }
    }
    
    /**
     * 处理获取用户地址
     */
    private String handleGetUserAddresses(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            String result = EmshopNativeInterface.getUserAddresses(userId);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("get_user_addresses", true, "Addresses retrieved successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("get_user_addresses", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("get_user_addresses", false, "Error retrieving addresses", null);
        }
    }
    
    /**
     * 处理添加地址
     */
    private String handleAddAddress(ChannelHandlerContext ctx, JsonNode jsonNode) {
        try {
            Long userId = ctx.channel().attr(EmshopWebSocketServer.USER_ID_ATTR).get();
            String receiverName = jsonNode.get("receiverName").asText();
            String phone = jsonNode.get("phone").asText();
            String province = jsonNode.get("province").asText();
            String city = jsonNode.get("city").asText();
            String district = jsonNode.get("district").asText();
            String detailAddress = jsonNode.get("detailAddress").asText();
            String postalCode = jsonNode.has("postalCode") ? jsonNode.get("postalCode").asText() : "000000";
            boolean isDefault = jsonNode.has("isDefault") ? jsonNode.get("isDefault").asBoolean() : false;
            
            String result = EmshopNativeInterface.addUserAddress(userId, receiverName, phone, 
                province, city, district, detailAddress, postalCode, isDefault);
            JsonNode resultNode = objectMapper.readTree(result);
            
            if (resultNode.get("success").asBoolean()) {
                return createResponse("add_address", true, "Address added successfully", 
                    resultNode.get("data").toString());
            } else {
                return createResponse("add_address", false, 
                    resultNode.get("message").asText(), null);
            }
        } catch (Exception e) {
            return createResponse("add_address", false, "Error adding address", null);
        }
    }
    
    /**
     * 广播用户状态变更
     */
    private void broadcastUserStatus(Long userId, String username, String status) {
        String message = String.format(
            "{\"type\":\"user_status\",\"data\":{\"userId\":%d,\"username\":\"%s\",\"status\":\"%s\",\"timestamp\":%d}}", 
            userId, username, status, System.currentTimeMillis());
        
        // 这里可以选择只向特定用户推送，或者向所有用户推送
        // EmshopWebSocketServer.broadcast(message);
    }
    
    /**
     * 处理空闲状态事件（心跳检测）
     */
    @Override
    public void userEventTriggered(ChannelHandlerContext ctx, Object evt) throws Exception {
        if (evt instanceof IdleStateEvent) {
            IdleStateEvent event = (IdleStateEvent) evt;
            if (event.state() == IdleState.READER_IDLE) {
                System.out.println("Client idle timeout, closing connection: " + ctx.channel().remoteAddress());
                ctx.close();
            }
        }
        super.userEventTriggered(ctx, evt);
    }
    
    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        System.err.println("WebSocket handler exception: " + cause.getMessage());
        cause.printStackTrace();
        ctx.close();
    }
    
    /**
     * 发送消息到客户端
     */
    private void sendMessage(ChannelHandlerContext ctx, String message) {
        if (ctx.channel().isActive()) {
            ctx.writeAndFlush(new TextWebSocketFrame(message));
        }
    }
    
    /**
     * 创建响应消息
     */
    private String createResponse(String type, boolean success, String message, String data) {
        StringBuilder response = new StringBuilder();
        response.append("{\"type\":\"").append(type).append("\"");
        response.append(",\"success\":").append(success);
        response.append(",\"message\":\"").append(message).append("\"");
        response.append(",\"timestamp\":").append(System.currentTimeMillis());
        
        if (data != null) {
            response.append(",\"data\":").append(data);
        }
        
        response.append("}");
        return response.toString();
    }
}