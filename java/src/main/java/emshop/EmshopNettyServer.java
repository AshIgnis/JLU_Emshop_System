package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.DelimiterBasedFrameDecoder;
import io.netty.handler.codec.Delimiters;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.codec.string.StringEncoder;
import io.netty.util.CharsetUtil;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 基于Netty的Emshop服务器端
 * 负责接收客户端请求并分发到业务处理模块
 */
public class EmshopNettyServer {
    private static final Logger logger = LoggerFactory.getLogger(EmshopNettyServer.class);
    private static final ObjectMapper JSON_MAPPER = new ObjectMapper();
    private final int port;
    private Channel channel;
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;
    
    // 会话管理：存储连接的用户信息
    private static final Map<ChannelId, UserSession> userSessions = new ConcurrentHashMap<>();
    
    // 用户会话信息类
    private static class UserSession {
        private long userId;
        private String username;
        private String role;
        private boolean isLoggedIn;

        static String normalizeRole(String value) {
            if (value == null) {
                return "";
            }
            return value.trim().toLowerCase();
        }

        public UserSession() {
            this.userId = -1;
            this.username = "";
            this.role = "";
            this.isLoggedIn = false;
        }

        public UserSession(long userId, String username, String role) {
            this.userId = userId;
            this.username = username;
            this.role = normalizeRole(role);
            this.isLoggedIn = true;
        }

        // Getters and setters
        public long getUserId() { return userId; }
        public String getUsername() { return username; }
        public String getRole() { return role; }
        public boolean isLoggedIn() { return isLoggedIn; }
        public boolean isAdmin() { return "admin".equals(role); }
    }

    // 轻量文本化工具，避免引入第三方JSON库；使用简单查找拼接
    static class HumanReadable {
        static String extract(String json, String key) {
            String pattern = "\"" + key + "\":";
            int idx = json.indexOf(pattern);
            if (idx < 0) return null;
            int start = idx + pattern.length();
            // 跳过空格和引号
            while (start < json.length() && (json.charAt(start) == ' ')) start++;
            if (start < json.length() && json.charAt(start) == '"') {
                start++;
                int end = json.indexOf('"', start);
                if (end > start) return json.substring(start, end);
            } else {
                // 数字
                int end = start;
                while (end < json.length() && "0123456789.-".indexOf(json.charAt(end)) >= 0) end++;
                if (end > start) return json.substring(start, end);
            }
            return null;
        }

        static String formatOrderDetail(String json) {
            // 仅在成功时处理，提取主要字段
            if (json == null || !json.contains("\"success\":true")) return null;
            String id = extract(json, "order_id");
            if (id == null) id = extract(json, "id");
            String status = extract(json, "status");
            String total = extract(json, "total_amount");
            String discount = extract(json, "discount_amount");
            String finalAmt = extract(json, "final_amount");
            String addr = extract(json, "shipping_address");
            StringBuilder sb = new StringBuilder();
            sb.append("订单编号: ").append(id != null ? id : "-").append('\n');
            if (status != null) sb.append("状态: ").append(status).append('\n');
            if (addr != null) sb.append("收货地址: ").append(addr).append('\n');
            if (total != null) sb.append("商品总额: ").append(total).append(" 元\n");
            if (discount != null) sb.append("优惠金额: ").append(discount).append(" 元\n");
            if (finalAmt != null) sb.append("应付金额: ").append(finalAmt).append(" 元\n");
            return sb.toString();
        }
    }

    public EmshopNettyServer(int port) {
        this.port = port;
    }

    /**
     * 启动服务器
     */
    public void start() throws InterruptedException {
        // 创建事件循环组
        bossGroup = new NioEventLoopGroup(1); // 负责接收连接
        workerGroup = new NioEventLoopGroup(); // 负责处理I/O操作

        try {
            ServerBootstrap bootstrap = new ServerBootstrap();
            bootstrap.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .option(ChannelOption.SO_BACKLOG, 1024)
                    .childOption(ChannelOption.SO_KEEPALIVE, true)
                    .childOption(ChannelOption.TCP_NODELAY, true)
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel ch) {
                            ChannelPipeline pipeline = ch.pipeline();
                            
                            // 添加解码器，以换行符为分隔符
                            pipeline.addLast("framer", new DelimiterBasedFrameDecoder(8192, Delimiters.lineDelimiter()));
                            // 字符串解码器和编码器
                            pipeline.addLast("decoder", new StringDecoder(CharsetUtil.UTF_8));
                            pipeline.addLast("encoder", new StringEncoder(CharsetUtil.UTF_8));
                            // 业务处理器
                            pipeline.addLast("handler", new EmshopServerHandler());
                        }
                    });

            // 绑定端口并开始接收连接
            ChannelFuture future = bootstrap.bind(port).sync();
            channel = future.channel();
            logger.info("Emshop Netty Server started successfully - port={}", port);
            
            // 等待服务器关闭
            channel.closeFuture().sync();
            
        } finally {
            shutdown();
        }
    }

    /**
     * 关闭服务器
     */
    public void shutdown() {
        if (channel != null) {
            channel.close();
        }
        if (bossGroup != null) {
            bossGroup.shutdownGracefully();
        }
        if (workerGroup != null) {
            workerGroup.shutdownGracefully();
        }
        logger.info("Emshop Netty Server shutdown completed");
    }

    /**
     * 服务器业务处理器
     */
    private static class EmshopServerHandler extends SimpleChannelInboundHandler<String> {
        private static final Logger handlerLogger = LoggerFactory.getLogger(EmshopServerHandler.class);
        
        @Override
        public void channelActive(ChannelHandlerContext ctx) {
            // 初始化trace ID
            TraceIdUtil.initTraceId();
            
            try {
                String remoteAddr = ctx.channel().remoteAddress().toString();
                handlerLogger.info("Client connected - remoteAddress={}", remoteAddr);
                
                // 初始化用户会话
                userSessions.put(ctx.channel().id(), new UserSession());
                ctx.writeAndFlush("Welcome to Emshop Server! Please login to access features.\n");
            } finally {
                TraceIdUtil.clear();
            }
        }

        @Override
        public void channelInactive(ChannelHandlerContext ctx) {
            // 初始化trace ID
            TraceIdUtil.initTraceId();
            
            try {
                String remoteAddr = ctx.channel().remoteAddress().toString();
                handlerLogger.info("Client disconnected - remoteAddress={}", remoteAddr);
                
                // 清理用户会话
                UserSession session = userSessions.remove(ctx.channel().id());
                if (session != null && session.isLoggedIn()) {
                    handlerLogger.info("User logged out - username={}, userId={}", 
                        session.getUsername(), session.getUserId());
                    // 注意: BusinessLogger没有logLogout方法,使用logLogin记录登出
                }
            } finally {
                TraceIdUtil.clear();
            }
        }

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, String request) {
            // 初始化trace ID (每个请求独立的trace ID)
            TraceIdUtil.initTraceId();
            
            try {
                String remoteAddr = ctx.channel().remoteAddress().toString();
                handlerLogger.debug("Received request - remoteAddress={}, request={}", remoteAddr, request);
                
                // 调用JNI接口处理请求
                String response = processRequest(ctx, request.trim());
                handlerLogger.debug("Sending response - remoteAddress={}, response={}", remoteAddr, response);
                ctx.writeAndFlush(response + "\n");
            } catch (Exception e) {
                handlerLogger.error("Error processing request - error={}", e.getMessage(), e);
                ctx.writeAndFlush("ERROR: " + e.getMessage() + "\n");
            } finally {
                TraceIdUtil.clear();
            }
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
            handlerLogger.error("Server exception caught - remoteAddress={}, error={}", 
                ctx.channel().remoteAddress(), cause.getMessage(), cause);
            ctx.close();
        }
        
        /**
         * 处理客户端请求，调用C++实现的JNI接口
         */
        private String processRequest(ChannelHandlerContext ctx, String request) {
            try {
                // 检查是否是JSON格式的消息
                if (request.startsWith("{") && request.endsWith("}")) {
                    return processJsonRequest(ctx, request);
                }
                
                // 解析请求格式：METHOD PARAM1 PARAM2 ... (支持引号包围的参数)
                String[] parts = parseCommand(request);
                if (parts.length < 1) {
                    return "{\"success\":false,\"message\":\"Invalid request format\"}";
                }
                
                String method = parts[0].toUpperCase();
                
                // 获取当前用户会话
                UserSession session = userSessions.get(ctx.channel().id());
                if (session == null) {
                    session = new UserSession();
                    userSessions.put(ctx.channel().id(), session);
                }
                
                // 检查是否需要登录验证（除了LOGIN, REGISTER, PING, INIT, STATUS命令）
                if (!session.isLoggedIn() && !isPublicCommand(method)) {
                    return "{\"success\":false,\"message\":\"Please login first\",\"error_code\":401}";
                }
                
                // 根据方法名调用相应的JNI接口
                switch (method) {
                    // === User Authentication ===
                    case "LOGIN":
                        if (parts.length >= 3) {
                            String username = parts[1];
                            String loginResult = EmshopNativeInterface.login(username, parts[2]);
                            
                            // 如果登录成功，保存会话信息
                            if (loginResult.contains("\"success\":true")) {
                                // 简单解析JSON获取用户信息（实际项目建议使用JSON库）
                                try {
                                    long userId = extractUserIdFromResponse(loginResult);
                                    String role = extractRoleFromResponse(loginResult);
                                    
                                    // 临时解决方案：如果用户ID为1，则设为管理员
                                    if (userId == 1) {
                                        role = "admin";
                                    }
                                    
                                    session = new UserSession(userId, username, role);
                                    userSessions.put(ctx.channel().id(), session);
                                    TraceIdUtil.setUserContext(userId, username);
                                    
                                    handlerLogger.info("User login successful - username={}, userId={}, role={}", 
                                        username, userId, role);
                                    BusinessLogger.logLogin(userId, username, 
                                        ctx.channel().remoteAddress().toString(), true);
                                } catch (Exception e) {
                                    handlerLogger.error("Failed to parse login response - username={}, error={}", 
                                        username, e.getMessage(), e);
                                }
                            } else {
                                handlerLogger.warn("User login failed - username={}", username);
                                BusinessLogger.logLogin(-1, username, 
                                    ctx.channel().remoteAddress().toString(), false);
                            }
                            return loginResult;
                        }
                        break;
                        
                    case "REGISTER":
                        if (parts.length >= 4) {
                            String regUsername = parts[1];
                            String regEmail = parts[3];
                            String regResult = EmshopNativeInterface.register(regUsername, parts[2], regEmail);
                            
                            if (regResult.contains("\"success\":true")) {
                                handlerLogger.info("User registration successful - username={}", regUsername);
                                BusinessLogger.logRegister(-1, regUsername, regEmail, 
                                    ctx.channel().remoteAddress().toString());
                            } else {
                                handlerLogger.warn("User registration failed - username={}", regUsername);
                            }
                            return regResult;
                        }
                        break;

                    // === Admin - User Management ===
                    case "GET_ALL_USERS":
                        if (session == null || !session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        {
                            int userPage = 1;
                            int userPageSize = 20;
                            String statusFilter = "all";
                            String keyword = "";

                            for (int i = 1; i < parts.length; i++) {
                                String arg = parts[i];
                                if (arg == null || arg.isEmpty()) continue;
                                String lower = arg.toLowerCase();
                                if (lower.startsWith("status=")) {
                                    statusFilter = arg.substring(arg.indexOf('=') + 1);
                                } else if (lower.startsWith("page=")) {
                                    userPage = Integer.parseInt(arg.substring(arg.indexOf('=') + 1));
                                } else if (lower.startsWith("pagesize=") || lower.startsWith("page_size=")) {
                                    userPageSize = Integer.parseInt(arg.substring(arg.indexOf('=') + 1));
                                } else if (lower.startsWith("keyword=") || lower.startsWith("search=")) {
                                    keyword = arg.substring(arg.indexOf('=') + 1);
                                } else if (keyword.isEmpty() && arg.matches("-?\\d+")) {
                                    if (userPage == 1) {
                                        userPage = Integer.parseInt(arg);
                                    } else {
                                        userPageSize = Integer.parseInt(arg);
                                    }
                                } else if (keyword.isEmpty()) {
                                    keyword = arg;
                                }
                            }

                            userPage = Math.max(userPage, 1);
                            userPageSize = Math.min(Math.max(userPageSize, 1), 100);

                            if (!keyword.isEmpty()) {
                                return EmshopNativeInterface.searchUsers(keyword, userPage, userPageSize);
                            }
                            return EmshopNativeInterface.getAllUsers(userPage, userPageSize, statusFilter);
                        }

                    case "SET_USER_ROLE":
                        if (session == null || !session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 3) {
                            long targetUserId = Long.parseLong(parts[1]);
                            String roleValue = parts[2];
                            return EmshopNativeInterface.setUserRole(targetUserId, roleValue);
                        }
                        break;

                    case "SET_USER_STATUS":
                        if (session == null || !session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 3) {
                            int targetUserId = Integer.parseInt(parts[1]);
                            String targetStatus = parts[2];
                            return EmshopNativeInterface.setUserStatus(targetUserId, targetStatus);
                        }
                        break;

                    case "ASSIGN_COUPON":
                        if (session == null || !session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 3) {
                            long targetUserId = Long.parseLong(parts[1]);
                            String couponToken = parts[2];
                            if (couponToken != null && !couponToken.isEmpty()) {
                                couponToken = couponToken.replace("\\\"", "\"").replace("\\\\", "\\");
                            }
                            return EmshopNativeInterface.assignCoupon(targetUserId, couponToken);
                        }
                        break;
                        
                    // === Product Management ===
                    case "GET_PRODUCTS":
                    case "VIEW_PRODUCTS":
                        String category = parts.length > 1 ? parts[1] : "all";
                        int page = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                        int pageSize = parts.length > 3 ? Integer.parseInt(parts[3]) : 10;
                        return EmshopNativeInterface.getProductList(category, page, pageSize);
                        
                    case "SEARCH_PRODUCTS":
                        if (parts.length >= 2) {
                            String keyword = parts[1];
                            int searchPage = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                            int searchPageSize = parts.length > 3 ? Integer.parseInt(parts[3]) : 10;
                            String sortBy = parts.length > 4 ? parts[4] : "id";
                            double minPrice = parts.length > 5 ? Double.parseDouble(parts[5]) : 0.0;
                            double maxPrice = parts.length > 6 ? Double.parseDouble(parts[6]) : 99999.0;
                            return EmshopNativeInterface.searchProducts(keyword, searchPage, searchPageSize, sortBy, minPrice, maxPrice);
                        }
                        break;
                        
                    // === Shopping Cart ===
                    case "ADD_TO_CART":
                        if (session == null) {
                            return "{\"success\":false,\"message\":\"请先登录\"}";
                        }
                        if (parts.length >= 3) {
                            // ADD_TO_CART productId quantity
                            long productId = Long.parseLong(parts[1]);
                            int quantity = Integer.parseInt(parts[2]);
                            return EmshopNativeInterface.addToCart(session.userId, productId, quantity);
                        } else if (parts.length >= 4) {
                            // 保持向后兼容 ADD_TO_CART userId productId quantity
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            int quantity = Integer.parseInt(parts[3]);
                            return EmshopNativeInterface.addToCart(userId, productId, quantity);
                        }
                        break;
                        
                    case "GET_CART":
                    case "VIEW_CART":
                        if (session == null) {
                            return "{\"success\":false,\"message\":\"请先登录\"}";
                        }
                        if (parts.length >= 2) {
                            // 向后兼容：GET_CART userId
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getCart(userId);
                        } else {
                            // 新格式：GET_CART (使用session中的userId)
                            return EmshopNativeInterface.getCart(session.userId);
                        }
                        
                    case "UPDATE_CART":
                        if (parts.length >= 4) {
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            int quantity = Integer.parseInt(parts[3]);
                            return EmshopNativeInterface.updateCartItemQuantity(userId, productId, quantity);
                        }
                        break;
                        
                    case "REMOVE_FROM_CART":
                        if (parts.length >= 2) {
                            // Session-based: REMOVE_FROM_CART productId
                            if (session != null && session.getUserId() != -1) {
                                long productId = Long.parseLong(parts[1]);
                                return EmshopNativeInterface.removeFromCart(session.getUserId(), productId);
                            }
                        } else if (parts.length >= 3) {
                            // Backward compatible: REMOVE_FROM_CART userId productId
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            return EmshopNativeInterface.removeFromCart(userId, productId);
                        }
                        break;
                        
                    case "CLEAR_CART":
                        if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.clearCart(userId);
                        }
                        break;

                    case "SELECT_CART_ITEM":
                        // Session-based: SELECT_CART_ITEM productId selected(true/false)
                        if (session != null && session.getUserId() != -1 && parts.length >= 3) {
                            long productId = Long.parseLong(parts[1]);
                            boolean selected = Boolean.parseBoolean(parts[2]);
                            return EmshopNativeInterface.updateCartSelected(session.getUserId(), productId, selected);
                        }
                        break;

                    case "SELECT_CART_ALL":
                        // Session-based: SELECT_CART_ALL selected(true/false)
                        if (session != null && session.getUserId() != -1 && parts.length >= 2) {
                            boolean selected = Boolean.parseBoolean(parts[1]);
                            // 约定 productId = -1 表示对该用户所有条目应用
                            return EmshopNativeInterface.updateCartSelected(session.getUserId(), -1, selected);
                        }
                        break;
                        
                    // === User Addresses ===
                    case "ADD_ADDRESS":
                        if (session != null && session.getUserId() != -1 && parts.length >= 7) {
                            // Session-based: ADD_ADDRESS receiverName phone province city district detailAddress [postalCode] [isDefault]
                            String receiverName = parts[1];
                            String phone = parts[2];
                            String province = parts[3];
                            String city = parts[4];
                            String district = parts[5];
                            // 合并后面的部分作为详细地址（可能包含空格）
                            StringBuilder detailAddress = new StringBuilder();
                            for (int i = 6; i < parts.length; i++) {
                                if (i > 6) detailAddress.append(" ");
                                detailAddress.append(parts[i]);
                            }
                            String postalCode = parts.length > 7 ? parts[7] : "000000";
                            boolean isDefault = parts.length > 8 ? Boolean.parseBoolean(parts[8]) : false;
                            return EmshopNativeInterface.addUserAddress(session.getUserId(), receiverName, phone, province, city, district, detailAddress.toString(), postalCode, isDefault);
                        } else if (parts.length >= 8) {
                            // Backward compatible: ADD_ADDRESS userId receiverName phone province city district detailAddress [postalCode] [isDefault]
                            long userId = Long.parseLong(parts[1]);
                            String receiverName = parts[2];
                            String phone = parts[3];
                            String province = parts[4];
                            String city = parts[5];
                            String district = parts[6];
                            // 合并后面的部分作为详细地址（可能包含空格）
                            StringBuilder detailAddress = new StringBuilder();
                            for (int i = 7; i < parts.length; i++) {
                                if (i > 7) detailAddress.append(" ");
                                detailAddress.append(parts[i]);
                            }
                            String postalCode = parts.length > 8 ? parts[8] : "000000";
                            boolean isDefault = parts.length > 9 ? Boolean.parseBoolean(parts[9]) : false;
                            return EmshopNativeInterface.addUserAddress(userId, receiverName, phone, province, city, district, detailAddress.toString(), postalCode, isDefault);
                        }
                        break;
                        
                    case "GET_USER_ADDRESSES":
                    case "VIEW_ADDRESSES":
                        if (session != null && session.getUserId() != -1) {
                            // Session-based: VIEW_ADDRESSES
                            return EmshopNativeInterface.getUserAddresses(session.getUserId());
                        } else if (parts.length >= 2) {
                            // Backward compatible: VIEW_ADDRESSES userId
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getUserAddresses(userId);
                        }
                        break;
                        
                    case "UPDATE_ADDRESS":
                        if (parts.length >= 8) {
                            long addressId = Long.parseLong(parts[1]);
                            String receiverName = parts[2];
                            String phone = parts[3];
                            String province = parts[4];
                            String city = parts[5];
                            String district = parts[6];
                            StringBuilder detailAddress = new StringBuilder();
                            for (int i = 7; i < parts.length; i++) {
                                if (i > 7) detailAddress.append(" ");
                                detailAddress.append(parts[i]);
                            }
                            String postalCode = parts.length > 8 ? parts[8] : "000000";
                            boolean isDefault = parts.length > 9 ? Boolean.parseBoolean(parts[9]) : false;
                            return EmshopNativeInterface.updateUserAddress(addressId, receiverName, phone, province, city, district, detailAddress.toString(), postalCode, isDefault);
                        }
                        break;
                        
                    case "DELETE_ADDRESS":
                        if (parts.length >= 2) {
                            long addressId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.deleteUserAddress(addressId);
                        }
                        break;
                        
                    case "SET_DEFAULT_ADDRESS":
                        if (parts.length >= 3) {
                            long userId = Long.parseLong(parts[1]);
                            long addressId = Long.parseLong(parts[2]);
                            return EmshopNativeInterface.setDefaultAddress(userId, addressId);
                        }
                        break;
                        
                    // === Order Management ===
                    case "CREATE_ORDER":
                        if (parts.length >= 2) {
                            // Session-based: CREATE_ORDER addressId [couponCode] [remark]
                            if (session != null && session.getUserId() != -1) {
                                long addressId = Long.parseLong(parts[1]);
                                String couponCode = parts.length > 2 && !parts[2].equals("0") ? parts[2] : null;
                                String remark = parts.length > 3 ? parts[3] : "";
                                String createResult = EmshopNativeInterface.createOrderFromCart(
                                    session.getUserId(), addressId, couponCode, remark);
                                
                                if (createResult.contains("\"success\":true")) {
                                    try {
                                        String orderIdStr = HumanReadable.extract(createResult, "order_id");
                                        long orderId = orderIdStr != null ? Long.parseLong(orderIdStr) : -1;
                                        String totalStr = HumanReadable.extract(createResult, "total_amount");
                                        String finalStr = HumanReadable.extract(createResult, "final_amount");
                                        double totalAmount = totalStr != null ? Double.parseDouble(totalStr) : 0.0;
                                        double finalAmount = finalStr != null ? Double.parseDouble(finalStr) : 0.0;
                                        
                                        handlerLogger.info("Order created successfully - userId={}, orderId={}, addressId={}, couponCode={}", 
                                            session.getUserId(), orderId, addressId, couponCode);
                                        BusinessLogger.logOrderCreate(orderId, session.getUserId(), session.getUsername(), 
                                            totalAmount, finalAmount);
                                    } catch (Exception e) {
                                        handlerLogger.error("Failed to parse order create response - error={}", e.getMessage(), e);
                                    }
                                } else {
                                    handlerLogger.warn("Order creation failed - userId={}", session.getUserId());
                                }
                                return createResult;
                            }
                        } else if (parts.length >= 5) {
                            // Backward compatible: CREATE_ORDER userId addressId couponCode remark
                            long userId = Long.parseLong(parts[1]);
                            long addressId = Long.parseLong(parts[2]);
                            String couponCode = parts[3].equals("0") ? null : parts[3];
                            String remark = parts.length > 4 ? parts[4] : "";
                            String createResult = EmshopNativeInterface.createOrderFromCart(userId, addressId, couponCode, remark);
                            
                            if (createResult.contains("\"success\":true")) {
                                try {
                                    String orderIdStr = HumanReadable.extract(createResult, "order_id");
                                    long orderId = orderIdStr != null ? Long.parseLong(orderIdStr) : -1;
                                    String totalStr = HumanReadable.extract(createResult, "total_amount");
                                    String finalStr = HumanReadable.extract(createResult, "final_amount");
                                    double totalAmount = totalStr != null ? Double.parseDouble(totalStr) : 0.0;
                                    double finalAmount = finalStr != null ? Double.parseDouble(finalStr) : 0.0;
                                    
                                    handlerLogger.info("Order created successfully - userId={}, orderId={}, addressId={}, couponCode={}", 
                                        userId, orderId, addressId, couponCode);
                                    BusinessLogger.logOrderCreate(orderId, userId, "user_" + userId, 
                                        totalAmount, finalAmount);
                                } catch (Exception e) {
                                    handlerLogger.error("Failed to parse order create response - error={}", e.getMessage(), e);
                                }
                            } else {
                                handlerLogger.warn("Order creation failed - userId={}", userId);
                            }
                            return createResult;
                        }
                        break;

                    case "CREATE_ORDER_ITEM":
                        // Session-based: CREATE_ORDER_ITEM productId quantity addressId [couponCode] [remark]
                        if (session != null && session.getUserId() != -1 && parts.length >= 4) {
                            long productId = Long.parseLong(parts[1]);
                            int quantity = Integer.parseInt(parts[2]);
                            long addressId = Long.parseLong(parts[3]);
                            String couponCode = parts.length > 4 && !parts[4].equals("0") ? parts[4] : null;
                            String remark = parts.length > 5 ? parts[5] : "";
                            return EmshopNativeInterface.createOrderDirect(session.getUserId(), productId, quantity, addressId, couponCode, remark);
                        }
                        break;
                        
                    case "GET_USER_ORDERS":
                    case "VIEW_ORDERS":
                        if (session != null && session.getUserId() != -1) {
                            // Session-based: VIEW_ORDERS
                            return EmshopNativeInterface.getOrderList(session.getUserId());
                        } else if (parts.length >= 2) {
                            // Backward compatible: VIEW_ORDERS userId
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getOrderList(userId);
                        }
                        break;

                    case "GET_ALL_ORDERS":
                        // Admin only: GET_ALL_ORDERS [status] [page] [pageSize] [startDate] [endDate]
                        if (session == null || !session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        {
                            String status = parts.length > 1 ? parts[1] : "all";
                            int pageAll = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                            int pageSizeAll = parts.length > 3 ? Integer.parseInt(parts[3]) : 20;
                            String startDate = parts.length > 4 ? parts[4] : "";
                            String endDate = parts.length > 5 ? parts[5] : "";
                            return EmshopNativeInterface.getAllOrders(status, pageAll, pageSizeAll, startDate, endDate);
                        }
                        
                    case "GET_ORDER_DETAIL":
                    case "VIEW_ORDER":
                        if (parts.length >= 2) {
                            long orderId = Long.parseLong(parts[1]);
                            String raw = EmshopNativeInterface.getOrderDetail(orderId);
                            // 附加友好的纯文本描述，便于客户端直接展示
                            try {
                                if (raw != null && raw.contains("\"success\":true")) {
                                    String friendly = HumanReadable.formatOrderDetail(raw);
                                    if (friendly != null) {
                                        // 稳健地插入 plain_text：优先在 message 字段前插入，否则在最外层闭括号前插入
                                        String escaped = friendly.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n");
                                        String marker = "\"message\":";
                                        int idx = raw.lastIndexOf(marker);
                                        if (idx > 0) {
                                            // 在 message 字段前注入，保留原文
                                            String head = raw.substring(0, idx);
                                            String tail = raw.substring(idx);
                                            return head + "\"plain_text\":\"" + escaped + "\"," + tail;
                                        }
                                        int insertPos = raw.lastIndexOf('}');
                                        if (insertPos > 0) {
                                            String head = raw.substring(0, insertPos);
                                            String tail = raw.substring(insertPos);
                                            return head + ",\"plain_text\":\"" + escaped + "\"" + tail;
                                        }
                                    }
                                }
                            } catch (Exception ignore) { }
                            return raw;
                        }
                        break;
                        
                    case "UPDATE_ORDER_STATUS":
                        if (parts.length >= 3) {
                            long orderId = Long.parseLong(parts[1]);
                            String status = parts[2];
                            return EmshopNativeInterface.updateOrderStatus(orderId, status);
                        }
                        break;
                        
                    case "CANCEL_ORDER":
                        if (parts.length >= 2) {
                            long orderId;
                            long userId;
                            if (parts.length == 3) {
                                userId = Long.parseLong(parts[1]);
                                orderId = Long.parseLong(parts[2]);
                            } else {
                                // Session-based: CANCEL_ORDER orderId
                                if (session == null || !session.isLoggedIn()) {
                                    return "{\"success\":false,\"message\":\"请先登录\",\"error_code\":401}";
                                }
                                userId = session.getUserId();
                                orderId = Long.parseLong(parts[1]);
                            }
                            
                            String cancelResult = EmshopNativeInterface.cancelOrder(userId, orderId);
                            if (cancelResult.contains("\"success\":true")) {
                                handlerLogger.info("Order cancelled successfully - userId={}, orderId={}", userId, orderId);
                                BusinessLogger.logOrderCancel(orderId, userId, 
                                    session != null ? session.getUsername() : "user_" + userId, 
                                    "User cancelled order");
                            } else {
                                handlerLogger.warn("Order cancellation failed - userId={}, orderId={}", userId, orderId);
                            }
                            return cancelResult;
                        }
                        break;

                    case "DELETE_ORDER":
                        if (parts.length >= 2) {
                            // 仅允许删除已取消订单；权限校验在JNI内按状态限制
                            long orderId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.deleteOrder(orderId);
                        }
                        break;
                        
                    // === Payment System ===
                    case "PROCESS_PAYMENT":
                        if (parts.length >= 4) {
                            long orderId = Long.parseLong(parts[1]);
                            String paymentMethod = parts[2];
                            double amount = Double.parseDouble(parts[3]);
                            String paymentDetails = parts.length > 4 ? parts[4] : "{}";
                            
                            String paymentResult = EmshopNativeInterface.processPayment(orderId, paymentMethod, amount, paymentDetails);
                            if (paymentResult.contains("\"success\":true")) {
                                long userId = session != null ? session.getUserId() : -1;
                                handlerLogger.info("Payment processed successfully - userId={}, orderId={}, amount={}, method={}", 
                                    userId, orderId, amount, paymentMethod);
                                BusinessLogger.logPayment(orderId, userId, paymentMethod, amount, true);
                            } else {
                                long userId = session != null ? session.getUserId() : -1;
                                handlerLogger.warn("Payment processing failed - userId={}, orderId={}, amount={}, method={}", 
                                    userId, orderId, amount, paymentMethod);
                                BusinessLogger.logPayment(orderId, userId, paymentMethod, amount, false);
                            }
                            return paymentResult;
                        }
                        break;
                        
                    case "REFUND_PAYMENT":
                        if (parts.length >= 4) {
                            long orderId = Long.parseLong(parts[1]);
                            double amount = Double.parseDouble(parts[2]);
                            String reason = parts[3];
                            return EmshopNativeInterface.refundPayment(orderId, amount, reason);
                        }
                        break;
                        
                    case "CHECK_PAYMENT_STATUS":
                        if (parts.length >= 2) {
                            long orderId = Long.parseLong(parts[1]);
                            // 使用现有的订单查看功能来检查支付状态
                            return EmshopNativeInterface.getOrderDetail(orderId);
                        }
                        break;
                        
                    case "GET_PAYMENT_METHODS":
                        return getPaymentMethods();
                        
                    case "VALIDATE_PAYMENT":
                        if (parts.length >= 4) {
                            String paymentMethod = parts[1];
                            double amount = Double.parseDouble(parts[2]);
                            String accountInfo = parts.length > 3 ? parts[3] : "{}";
                            return validatePaymentMethod(paymentMethod, amount, accountInfo);
                        }
                        break;
                        
                    // === Coupon System ===
                    case "GET_AVAILABLE_COUPONS":
                        return EmshopNativeInterface.getAvailableCoupons();
                        
                    case "GET_USER_COUPONS":
                        if (session != null && session.getUserId() != -1 && parts.length == 1) {
                            return EmshopNativeInterface.getUserCoupons(session.getUserId());
                        } else if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getUserCoupons(userId);
                        }
                        break;
                        
                    case "USE_COUPON":
                        if (parts.length >= 4) {
                            long userId = Long.parseLong(parts[1]);
                            long orderId = Long.parseLong(parts[2]);
                            String couponCode = parts[3];
                            return EmshopNativeInterface.useCoupon(userId, orderId, couponCode);
                        }
                        break;
                        
                    case "CLAIM_COUPON":
                        if (parts.length >= 3) {
                            long userId = Long.parseLong(parts[1]);
                            String couponCode = parts[2];
                            return EmshopNativeInterface.claimCoupon(userId, couponCode);
                        }
                        break;
                        
                    case "CALCULATE_DISCOUNT":
                        if (parts.length >= 4) {
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            String promoCode = parts[3];
                            return EmshopNativeInterface.calculateDiscount(userId, productId, promoCode);
                        }
                        break;
                        
                    // === Order Tracking ===
                    case "TRACK_ORDER":
                        if (parts.length >= 2) {
                            long orderId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.trackOrder(orderId);
                        }
                        break;
                        
                    // === Advanced Promotion System ===
                    case "GET_ACTIVE_PROMOTIONS":
                        return getActivePromotions();
                        
                    case "CALCULATE_CART_DISCOUNT":
                        if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            String promoCode = parts.length > 2 ? parts[2] : "";
                            return calculateCartDiscount(userId, promoCode);
                        }
                        break;
                        
                    case "APPLY_BULK_DISCOUNT":
                        if (parts.length >= 3) {
                            long productId = Long.parseLong(parts[1]);
                            int quantity = Integer.parseInt(parts[2]);
                            return applyBulkDiscount(productId, quantity);
                        }
                        break;
                        
                    case "CHECK_MEMBERSHIP_DISCOUNT":
                        if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            return checkMembershipDiscount(userId);
                        }
                        break;
                        
                    case "GET_SEASONAL_PROMOTIONS":
                        return getSeasonalPromotions();
                        
                    // === System Health Check ===
                    case "SYSTEM_STATUS":
                        return getSystemStatus();
                        
                    case "GET_FEATURE_STATUS":
                        return getFeatureCompletionStatus();
                        
                    // === Product Reviews ===
                    case "ADD_REVIEW":
                        if (parts.length >= 6) {
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            long orderId = Long.parseLong(parts[3]);
                            int rating = Integer.parseInt(parts[4]);
                            // 合并后面的部分作为评论内容（可能包含空格）
                            StringBuilder comment = new StringBuilder();
                            for (int i = 5; i < parts.length; i++) {
                                if (i > 5) comment.append(" ");
                                comment.append(parts[i]);
                            }
                            boolean isAnonymous = false; // 默认不匿名
                            return EmshopNativeInterface.addProductReview(userId, productId, orderId, rating, comment.toString(), isAnonymous);
                        }
                        break;
                        
                    case "GET_PRODUCT_REVIEWS":
                        if (parts.length >= 2) {
                            long productId = Long.parseLong(parts[1]);
                            int reviewPage = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                            int reviewPageSize = parts.length > 3 ? Integer.parseInt(parts[3]) : 10;
                            String sortBy = parts.length > 4 ? parts[4] : "id";
                            return EmshopNativeInterface.getProductReviews(productId, reviewPage, reviewPageSize, sortBy);
                        }
                        break;
                        
                    case "GET_USER_REVIEWS":
                        if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            int userReviewPage = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                            int userReviewPageSize = parts.length > 3 ? Integer.parseInt(parts[3]) : 10;
                            return EmshopNativeInterface.getUserReviews(userId, userReviewPage, userReviewPageSize);
                        }
                        break;
                        
                    case "UPDATE_REVIEW":
                        // 该功能暂不支持，返回错误信息
                        return "{\"success\": false, \"message\": \"Review update not supported yet\"}";
                        
                        
                    case "DELETE_REVIEW":
                        if (parts.length >= 3) {
                            long reviewId = Long.parseLong(parts[1]);
                            long userId = Long.parseLong(parts[2]);
                            return EmshopNativeInterface.deleteProductReview(reviewId, userId);
                        }
                        break;
                        
                    // === Inventory Management ===
                    case "CHECK_STOCK":
                        if (parts.length >= 2) {
                            long productId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.checkStock(productId);
                        }
                        break;
                        
                    case "UPDATE_STOCK":
                        if (parts.length >= 4) {
                            // 检查管理员权限
                            if (!session.isAdmin()) {
                                return "{\"success\":false,\"message\":\"权限不足：只有管理员可以修改库存\",\"error_code\":403}";
                            }
                            long productId = Long.parseLong(parts[1]);
                            int quantity = Integer.parseInt(parts[2]);
                            String operation = parts[3]; // "add" or "subtract"
                            return EmshopNativeInterface.updateStock(productId, quantity, operation);
                        }
                        break;
                        
                    case "GET_LOW_STOCK_PRODUCTS":
                        if (parts.length >= 2) {
                            // 检查管理员权限
                            if (!session.isAdmin()) {
                                return "{\"success\":false,\"message\":\"权限不足：只有管理员可以查看低库存报告\",\"error_code\":403}";
                            }
                            int threshold = Integer.parseInt(parts[1]);
                            return EmshopNativeInterface.getLowStockProducts(threshold);
                        }
                        break;
                        
                    // === System Commands ===
                    case "PING":
                        return "{\"success\":true,\"message\":\"Server is running\",\"timestamp\":" + System.currentTimeMillis() + "}";
                        
                    case "INIT":
                        return EmshopNativeInterface.initializeService();
                        
                    case "STATUS":
                        return EmshopNativeInterface.getInitializationStatus();
                    
                    // === Business Logic Enhancement v1.1.0 ===
                    
                    // 退款管理
                    case "APPROVE_REFUND":
                        if (!session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 4) {
                            long refundId = Long.parseLong(parts[1]);
                            boolean approve = Boolean.parseBoolean(parts[2]);
                            String adminReply = parts.length > 4 ? parts[3] : "";
                            return EmshopNativeInterface.approveRefund(refundId, session.getUserId(), approve, adminReply);
                        }
                        break;
                    
                    case "GET_REFUND_REQUESTS":
                        if (!session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        {
                            String refundStatus = parts.length > 1 ? parts[1] : "all";
                            int refundPage = parts.length > 2 ? Integer.parseInt(parts[2]) : 1;
                            int refundPageSize = parts.length > 3 ? Integer.parseInt(parts[3]) : 20;
                            return EmshopNativeInterface.getRefundRequests(refundStatus, refundPage, refundPageSize);
                        }
                    
                    case "GET_USER_REFUND_REQUESTS":
                        {
                            long userId = parts.length > 1 ? Long.parseLong(parts[1]) : session.getUserId();
                            // 用户只能查看自己的退款申请，管理员可以查看任何用户的
                            if (!session.isAdmin() && userId != session.getUserId()) {
                                return "{\"success\":false,\"message\":\"Permission denied\",\"error_code\":403}";
                            }
                            return EmshopNativeInterface.getUserRefundRequests(userId);
                        }
                    
                    case "REQUEST_REFUND":
                        if (parts.length >= 3) {
                            long orderId = Long.parseLong(parts[1]);
                            String reason = parts[2];
                            return EmshopNativeInterface.requestRefund(orderId, session.getUserId(), reason);
                        }
                        break;
                    
                    // 通知管理
                    case "GET_NOTIFICATIONS":
                        {
                            boolean unreadOnly = parts.length > 1 && Boolean.parseBoolean(parts[1]);
                            return EmshopNativeInterface.getNotifications(session.getUserId(), unreadOnly);
                        }
                    
                    case "MARK_NOTIFICATION_READ":
                        if (parts.length >= 2) {
                            long notificationId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.markNotificationRead(notificationId, session.getUserId());
                        }
                        break;

                    case "DELETE_NOTIFICATION":
                        if (parts.length >= 2) {
                            long notificationId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.deleteNotification(notificationId, session.getUserId());
                        }
                        break;
                    
                    // 优惠券增强
                    case "GET_AVAILABLE_COUPONS_FOR_ORDER":
                        if (parts.length >= 2) {
                            double orderAmount = Double.parseDouble(parts[1]);
                            return EmshopNativeInterface.getAvailableCouponsForOrder(session.getUserId(), orderAmount);
                        }
                        break;
                    
                    case "CALCULATE_COUPON_DISCOUNT":
                        if (parts.length >= 3) {
                            String couponCode = parts[1];
                            double orderAmount = Double.parseDouble(parts[2]);
                            return EmshopNativeInterface.calculateCouponDiscount(couponCode, orderAmount);
                        }
                        break;
                    
                    case "CREATE_COUPON_ACTIVITY":
                        if (!session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 9) {
                            String name = parts[1];
                            String code = parts[2];
                            String type = parts[3];
                            double value = Double.parseDouble(parts[4]);
                            double minAmount = Double.parseDouble(parts[5]);
                            int quantity = Integer.parseInt(parts[6]);
                            String startDate = parts[7];
                            String endDate = parts[8];
                            long templateId = parts.length > 9 ? Long.parseLong(parts[9]) : 0;
                            return EmshopNativeInterface.createCouponActivity(name, code, type, value, minAmount, quantity, startDate, endDate, templateId);
                        }
                        break;
                    
                    case "GET_COUPON_TEMPLATES":
                        return EmshopNativeInterface.getCouponTemplates();
                    
                    case "CREATE_PROMOTION":
                        // Qt客户端使用此命令创建促销/优惠券活动,接收JSON格式数据
                        if (!session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 2) {
                            try {
                                String jsonPayload = extractCommandPayload(request);
                                String parsedArgument = parts.length > 1 ? parts[1] : null;
                                handlerLogger.info("CREATE_PROMOTION - 收到原始载荷: {}; 首个参数: {}", jsonPayload, parsedArgument);

                                JsonNode jsonObj = parsePromotionPayload(jsonPayload, parsedArgument);
                                if (handlerLogger.isDebugEnabled()) {
                                    handlerLogger.debug("CREATE_PROMOTION - 标准化载荷: {}", jsonObj.toString());
                                }
                                
                                String name = jsonObj.get("name").asText();
                                String code = jsonObj.get("code").asText();
                                String type = jsonObj.get("discount_type").asText();
                                double value = jsonObj.get("discount_value").asDouble();
                                double minAmount = jsonObj.has("min_amount") ? jsonObj.get("min_amount").asDouble() : 0.0;
                                int quantity = jsonObj.has("quantity") ? jsonObj.get("quantity").asInt() : 100; // 默认100张
                                String startDate = jsonObj.has("start_date") ? jsonObj.get("start_date").asText() : "";
                                String endDate = jsonObj.has("end_date") ? jsonObj.get("end_date").asText() : "";
                                long templateId = 0; // 不使用模板
                                
                                return EmshopNativeInterface.createCouponActivity(name, code, type, value, minAmount, quantity, startDate, endDate, templateId);
                            } catch (Exception e) {
                                handlerLogger.error("Failed to parse CREATE_PROMOTION JSON - error={}", e.getMessage(), e);
                                return "{\"success\":false,\"message\":\"Invalid JSON format: " + e.getMessage() + "\"}";
                            }
                        }
                        break;
                    
                    case "DISTRIBUTE_COUPONS":
                        if (!session.isAdmin()) {
                            return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
                        }
                        if (parts.length >= 3) {
                            String couponCode = parts[1];
                            String userIdsJson = parts[2]; // 应该是JSON数组字符串，如 "[1,2,3]"
                            return EmshopNativeInterface.distributeCouponsToUsers(couponCode, userIdsJson);
                        }
                        break;
                        
                    default:
                        return "{\"success\":false,\"message\":\"Unknown method: " + method + "\"}";
                }
                
                return "{\"success\":false,\"message\":\"Invalid parameters for method: " + method + "\"}";
                
            } catch (UnsatisfiedLinkError e) {
                handlerLogger.error("Missing native method - error={}", e.getMessage(), e);
                return "{\"success\":false,\"message\":\"Server native module not available for request\",\"error_code\":500}";
            } catch (NumberFormatException e) {
                handlerLogger.error("Invalid number format - error={}", e.getMessage(), e);
                return "{\"success\":false,\"message\":\"Invalid number format: " + e.getMessage() + "\"}";
            } catch (Exception e) {
                handlerLogger.error("Error processing request - error={}", e.getMessage(), e);
                return "{\"success\":false,\"message\":\"Error processing request: " + e.getMessage() + "\"}";
            } catch (Throwable t) {
                handlerLogger.error("Unexpected server error - error={}", t.getMessage(), t);
                return "{\"success\":false,\"message\":\"Server internal error\",\"error_code\":500}";
            }
        }
        
        private String extractCommandPayload(String request) {
            if (request == null) {
                return "";
            }
            String trimmed = request.trim();
            int firstSpace = trimmed.indexOf(' ');
            if (firstSpace < 0) {
                return "";
            }
            return trimmed.substring(firstSpace + 1).trim();
        }

        private JsonNode parsePromotionPayload(String rawPayload, String parsedArgument) throws IOException {
            LinkedHashSet<String> visited = new LinkedHashSet<>();
            java.util.ArrayDeque<String> queue = new java.util.ArrayDeque<>();

            enqueuePromotionCandidate(visited, queue, rawPayload);
            enqueuePromotionCandidate(visited, queue, parsedArgument);

            if (queue.isEmpty()) {
                throw new IOException("Empty CREATE_PROMOTION payload");
            }

            IOException lastError = null;

            while (!queue.isEmpty()) {
                String candidate = queue.poll();
                if (candidate == null || candidate.isEmpty()) {
                    continue;
                }

                try {
                    JsonNode node = JSON_MAPPER.readTree(candidate);
                    if (node.isObject()) {
                        if (handlerLogger.isDebugEnabled()) {
                            handlerLogger.debug("CREATE_PROMOTION - 使用候选解析成功: {}", truncateForLog(candidate));
                        }
                        return node;
                    }

                    if (node.isTextual()) {
                        enqueuePromotionCandidate(visited, queue, node.asText());
                    }
                } catch (IOException ex) {
                    lastError = ex;
                    if (handlerLogger.isDebugEnabled()) {
                        handlerLogger.debug("CREATE_PROMOTION - 解析候选失败: {} -> {}", truncateForLog(candidate), ex.getMessage());
                    }
                }
            }

            if (lastError != null) {
                throw lastError;
            }
            throw new IOException("CREATE_PROMOTION JSON normalization failed");
        }

        private void enqueuePromotionCandidate(LinkedHashSet<String> visited, java.util.ArrayDeque<String> queue, String value) {
            if (value == null) {
                return;
            }

            String trimmed = value.trim();
            if (trimmed.isEmpty() || !visited.add(trimmed)) {
                return;
            }

            queue.add(trimmed);

            if (isWrappedWith(trimmed, '"')) {
                enqueuePromotionCandidate(visited, queue, trimmed.substring(1, trimmed.length() - 1));
            }

            String simplified = trimmed
                    .replace("\\\"", "\"")
                    .replace("\\\\", "\\")
                    .replace("\\/", "/");
            if (!simplified.equals(trimmed)) {
                enqueuePromotionCandidate(visited, queue, simplified);
            }

            String unescaped = unescapeJavaString(trimmed);
            if (!unescaped.equals(trimmed)) {
                enqueuePromotionCandidate(visited, queue, unescaped);
            }
        }

        private static String truncateForLog(String seed) {
            if (seed == null) {
                return "";
            }
            final int limit = 80;
            if (seed.length() <= limit) {
                return seed;
            }
            return seed.substring(0, limit) + "…";
        }

        private static boolean isWrappedWith(String text, char wrapper) {
            return text.length() >= 2 && text.charAt(0) == wrapper && text.charAt(text.length() - 1) == wrapper;
        }

        private static String unescapeJavaString(String input) {
            StringBuilder result = new StringBuilder(input.length());
            boolean escaping = false;

            for (int i = 0; i < input.length(); i++) {
                char c = input.charAt(i);
                if (!escaping) {
                    if (c == '\\') {
                        escaping = true;
                    } else {
                        result.append(c);
                    }
                } else {
                    switch (c) {
                        case 'b':
                            result.append('\b');
                            break;
                        case 't':
                            result.append('\t');
                            break;
                        case 'n':
                            result.append('\n');
                            break;
                        case 'f':
                            result.append('\f');
                            break;
                        case 'r':
                            result.append('\r');
                            break;
                        case '\\':
                            result.append('\\');
                            break;
                        case '"':
                            result.append('"');
                            break;
                        case '\'':
                            result.append('\'');
                            break;
                        case 'u':
                            if (i + 4 < input.length()) {
                                String hex = input.substring(i + 1, i + 5);
                                try {
                                    result.append((char) Integer.parseInt(hex, 16));
                                    i += 4;
                                } catch (NumberFormatException e) {
                                    result.append('\\').append('u').append(hex);
                                    i += 4;
                                }
                            } else {
                                result.append('\\').append('u');
                            }
                            break;
                        default:
                            result.append('\\').append(c);
                            break;
                    }
                    escaping = false;
                }
            }

            if (escaping) {
                result.append('\\');
            }

            return result.toString();
        }

        /**
         * 检查是否为公共命令（不需要登录）
         */
        private boolean isPublicCommand(String method) {
            switch (method) {
                case "LOGIN":
                case "REGISTER":
                case "PING":
                case "INIT":
                case "STATUS":
                    return true;
                default:
                    return false;
            }
        }
        
        /**
         * 从登录响应中提取用户ID
         */
        private long extractUserIdFromResponse(String response) {
            // 简单的JSON解析，查找 "user_id":数字
            int userIdIndex = response.indexOf("\"user_id\":");
            if (userIdIndex != -1) {
                int start = userIdIndex + 10;
                int end = response.indexOf(',', start);
                if (end == -1) end = response.indexOf('}', start);
                if (end != -1) {
                    String userIdStr = response.substring(start, end).trim();
                    return Long.parseLong(userIdStr);
                }
            }
            return 0; // 默认值
        }
        
        /**
         * 从登录响应中提取用户角色
         */
        private String extractRoleFromResponse(String response) {
            // 简单的JSON解析，查找 "role":"值"
            int roleIndex = response.indexOf("\"role\":\"");
            if (roleIndex != -1) {
                int start = roleIndex + 8;
                int end = response.indexOf('"', start);
                if (end != -1) {
                    return UserSession.normalizeRole(response.substring(start, end));
                }
            }
            
            // 如果直接查找role失败，尝试从user_info中提取
            // 查找user_info对象中的role字段
            int userInfoIndex = response.indexOf("\"user_info\":");
            if (userInfoIndex != -1) {
                // 在user_info对象内查找role
                int userInfoStart = response.indexOf('{', userInfoIndex);
                if (userInfoStart != -1) {
                    int userInfoEnd = response.indexOf('}', userInfoStart);
                    if (userInfoEnd != -1) {
                        String userInfoSection = response.substring(userInfoStart, userInfoEnd);
                        int roleInUserInfo = userInfoSection.indexOf("\"role\":\"");
                        if (roleInUserInfo != -1) {
                            int start = roleInUserInfo + 8;
                            int end = userInfoSection.indexOf('"', start);
                            if (end != -1) {
                                return UserSession.normalizeRole(userInfoSection.substring(start, end));
                            }
                        }
                    }
                }
            }
            
            return "user"; // 默认为普通用户
        }
        
        /**
         * 处理JSON格式的客户端请求
         */
        private String processJsonRequest(ChannelHandlerContext ctx, String request) {
            try {
                // 简单的JSON解析 - 提取action字段
                String action = extractJsonField(request, "action");
                
                // 获取当前用户会话
                UserSession session = userSessions.get(ctx.channel().id());
                if (session == null) {
                    session = new UserSession();
                    userSessions.put(ctx.channel().id(), session);
                }
                
                switch (action) {
                    case "heartbeat":
                        // 处理心跳包
                        return "{\"success\":true,\"action\":\"heartbeat_ack\",\"timestamp\":" + 
                               System.currentTimeMillis() + "}";
                        
                    case "login":
                        String username = extractJsonField(request, "username");
                        String password = extractJsonField(request, "password");
                        if (username != null && password != null) {
                            String loginResult = EmshopNativeInterface.login(username, password);
                            // 如果登录成功，保存会话信息
                            if (loginResult.contains("\"success\":true")) {
                                try {
                                    long userId = extractUserIdFromResponse(loginResult);
                                    String role = extractRoleFromResponse(loginResult);
                                    
                                    if (userId == 1) {
                                        role = "admin";
                                    }
                                    
                                    session = new UserSession(userId, username, role);
                                    userSessions.put(ctx.channel().id(), session);
                                    TraceIdUtil.setUserContext(userId, username);
                                    
                                    handlerLogger.info("User login successful (JSON) - username={}, userId={}, role={}", 
                                        username, userId, role);
                                    BusinessLogger.logLogin(userId, username, 
                                        ctx.channel().remoteAddress().toString(), true);
                                } catch (Exception e) {
                                    handlerLogger.error("Failed to parse login response (JSON) - username={}, error={}", 
                                        username, e.getMessage(), e);
                                }
                            } else {
                                handlerLogger.warn("User login failed (JSON) - username={}", username);
                                BusinessLogger.logLogin(-1, username, 
                                    ctx.channel().remoteAddress().toString(), false);
                            }
                            return loginResult;
                        }
                        break;
                        
                    default:
                        return "{\"success\":false,\"message\":\"Unknown JSON action: " + action + "\"}";
                }
                
                return "{\"success\":false,\"message\":\"Invalid JSON request\"}";
                
            } catch (UnsatisfiedLinkError e) {
                handlerLogger.error("Missing native method (JSON) - error={}", e.getMessage(), e);
                return "{\"success\":false,\"message\":\"Server native module not available\",\"error_code\":500}";
            } catch (Exception e) {
                handlerLogger.error("JSON processing error - error={}", e.getMessage(), e);
                return "{\"success\":false,\"message\":\"JSON processing error: " + e.getMessage() + "\"}";
            } catch (Throwable t) {
                handlerLogger.error("Unexpected JSON server error - error={}", t.getMessage(), t);
                return "{\"success\":false,\"message\":\"Server internal error\",\"error_code\":500}";
            }
        }
        
        /**
         * 从JSON字符串中提取指定字段的值
         */
        private String extractJsonField(String json, String fieldName) {
            try {
                // 简单的JSON解析：查找 "fieldName":"value" 或 "fieldName":value
                String pattern1 = "\"" + fieldName + "\":\"";
                String pattern2 = "\"" + fieldName + "\":";
                
                int start1 = json.indexOf(pattern1);
                if (start1 >= 0) {
                    start1 += pattern1.length();
                    int end = json.indexOf("\"", start1);
                    if (end > start1) {
                        return json.substring(start1, end);
                    }
                }
                
                int start2 = json.indexOf(pattern2);
                if (start2 >= 0) {
                    start2 += pattern2.length();
                    int end = json.indexOf(",", start2);
                    if (end == -1) {
                        end = json.indexOf("}", start2);
                    }
                    if (end > start2) {
                        return json.substring(start2, end).trim();
                    }
                }
                
                return null;
            } catch (Exception e) {
                return null;
            }
        }
    }

    /**
     * 解析命令行，支持引号包围的参数
     */
    private static String[] parseCommand(String command) {
        java.util.List<String> parts = new java.util.ArrayList<>();
        boolean inQuotes = false;
        StringBuilder current = new StringBuilder();
        boolean escaped = false;
        
        for (int i = 0; i < command.length(); i++) {
            char c = command.charAt(i);
            
            if (escaped) {
                // 处理转义字符
                if (c == '\\' || c == '"') {
                    current.append(c);
                } else {
                    // 如果不是有效的转义序列，保留反斜杠
                    current.append('\\').append(c);
                }
                escaped = false;
            } else if (c == '\\') {
                // 遇到反斜杠，标记为转义状态
                escaped = true;
            } else if (c == '"') {
                // 引号切换引用状态（不添加引号本身）
                inQuotes = !inQuotes;
            } else if (c == ' ' && !inQuotes) {
                // 空格且不在引号内，分割参数
                if (current.length() > 0) {
                    parts.add(current.toString());
                    current.setLength(0);
                }
            } else {
                // 普通字符
                current.append(c);
            }
        }
        
        // 添加最后一个参数
        if (current.length() > 0) {
            parts.add(current.toString());
        }
        
        return parts.toArray(new String[0]);
    }
    
    /**
     * 获取支付方式列表
     * TODO: 后续可以从数据库的 payment_methods 表读取配置
     */
    private static String getPaymentMethods() {
        try {
            ObjectMapper mapper = new ObjectMapper();
            ObjectNode result = mapper.createObjectNode();
            result.put("success", true);
            result.put("message", "获取支付方式成功");
            
            ObjectNode data = mapper.createObjectNode();
            com.fasterxml.jackson.databind.node.ArrayNode methods = mapper.createArrayNode();
            
            // 支付方式配置（后续可以从数据库读取）
            String[][] paymentMethods = {
                {"alipay", "支付宝", "true", "0.006"},
                {"wechat", "微信支付", "true", "0.006"},
                {"unionpay", "银联支付", "true", "0.005"},
                {"credit_card", "信用卡", "true", "0.008"},
                {"debit_card", "借记卡", "true", "0.005"}
            };
            
            for (String[] method : paymentMethods) {
                ObjectNode methodNode = mapper.createObjectNode();
                methodNode.put("code", method[0]);
                methodNode.put("name", method[1]);
                methodNode.put("enabled", Boolean.parseBoolean(method[2]));
                methodNode.put("fee_rate", Double.parseDouble(method[3]));
                methods.add(methodNode);
            }
            
            data.set("methods", methods);
            result.set("data", data);
            
            return mapper.writeValueAsString(result);
            
        } catch (Exception e) {
            logger.error("获取支付方式失败", e);
            return "{\"success\":false,\"message\":\"获取支付方式失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 验证支付方式
     */
    private static String validatePaymentMethod(String paymentMethod, double amount, String accountInfo) {
        try {
            logger.info("验证支付方式: method={}, amount={}", paymentMethod, amount);
            
            // 金额验证
            if (amount <= 0) {
                return "{\"success\":false,\"message\":\"支付金额必须大于0\",\"error_code\":\"INVALID_AMOUNT\"}";
            }
            
            if (amount > 50000) {
                return "{\"success\":false,\"message\":\"单笔支付金额不能超过50000元\",\"error_code\":\"AMOUNT_LIMIT_EXCEEDED\"}";
            }
            
            // 从数据库查询支付方式配置
            String paymentMethodsResult = getPaymentMethods();
            ObjectMapper mapper = new ObjectMapper();
            JsonNode root = mapper.readTree(paymentMethodsResult);
            
            if (!root.has("success") || !root.get("success").asBoolean()) {
                return "{\"success\":false,\"message\":\"获取支付方式配置失败\"}";
            }
            
            JsonNode methods = root.path("data").path("methods");
            boolean isValidMethod = false;
            double feeRate = 0.006; // 默认费率
            
            for (JsonNode method : methods) {
                String code = method.path("code").asText();
                if (code.equals(paymentMethod) && method.path("enabled").asBoolean(false)) {
                    isValidMethod = true;
                    if (method.has("fee_rate")) {
                        feeRate = method.path("fee_rate").asDouble(0.006);
                    }
                    break;
                }
            }
            
            if (!isValidMethod) {
                return "{\"success\":false,\"message\":\"不支持的支付方式或支付方式已禁用\",\"error_code\":\"INVALID_PAYMENT_METHOD\"}";
            }
            
            return String.format("{\"success\":true,\"message\":\"支付方式验证通过\",\"data\":{\"method\":\"%s\",\"fee_rate\":%.4f}}", 
                    paymentMethod, feeRate);
                    
        } catch (Exception e) {
            logger.error("支付验证异常", e);
            return "{\"success\":false,\"message\":\"支付验证异常: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 获取活跃促销活动
     */
    private static String getActivePromotions() {
        try {
            // 调用 Native 接口获取真实的优惠券数据
            String result = EmshopNativeInterface.getAvailableCoupons();
            logger.info("GET_ACTIVE_PROMOTIONS - 获取优惠券列表: {}", result);
            
            // 解析并转换数据格式以兼容前端
            ObjectMapper mapper = new ObjectMapper();
            JsonNode root = mapper.readTree(result);
            
            if (root.has("success") && root.get("success").asBoolean()) {
                // 如果数据在 data.coupons 中，将其映射到 data.promotions
                if (root.has("data")) {
                    JsonNode data = root.get("data");
                    if (data.has("coupons")) {
                        ObjectNode newRoot = mapper.createObjectNode();
                        newRoot.put("success", true);
                        newRoot.put("message", root.has("message") ? root.get("message").asText() : "获取促销活动成功");
                        ObjectNode newData = mapper.createObjectNode();
                        newData.set("promotions", data.get("coupons"));
                        newRoot.set("data", newData);
                        return mapper.writeValueAsString(newRoot);
                    }
                }
            }
            
            return result;
        } catch (Exception e) {
            logger.error("获取促销活动列表失败", e);
            return "{\"success\":false,\"message\":\"获取促销活动失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 计算购物车折扣
     */
    private static String calculateCartDiscount(long userId, String promoCode) {
        try {
            // 从数据库获取真实购物车数据
            String cartResult = EmshopNativeInterface.getCartSummary(userId);
            ObjectMapper mapper = new ObjectMapper();
            JsonNode cartRoot = mapper.readTree(cartResult);
            
            if (!cartRoot.has("success") || !cartRoot.get("success").asBoolean()) {
                return "{\"success\":false,\"message\":\"获取购物车信息失败\"}";
            }
            
            double originalTotal = cartRoot.path("data").path("total_price").asDouble(0.0);
            if (originalTotal <= 0) {
                return "{\"success\":false,\"message\":\"购物车为空或总价无效\"}";
            }
            
            double discount = 0.0;
            String discountType = "none";
            String discountDescription = "";
            
            // 如果提供了优惠码，查询优惠券信息
            if (promoCode != null && !promoCode.isEmpty()) {
                String couponsResult = EmshopNativeInterface.getAvailableCoupons();
                JsonNode couponsRoot = mapper.readTree(couponsResult);
                
                if (couponsRoot.has("success") && couponsRoot.get("success").asBoolean()) {
                    JsonNode coupons = couponsRoot.path("data").path("coupons");
                    for (JsonNode coupon : coupons) {
                        String code = coupon.path("code").asText();
                        if (code.equalsIgnoreCase(promoCode)) {
                            double minAmount = coupon.path("min_amount").asDouble(0.0);
                            if (originalTotal >= minAmount) {
                                String type = coupon.path("type").asText();
                                double value = coupon.path("value").asDouble(0.0);
                                
                                if (type.contains("percentage") || type.contains("percent")) {
                                    discount = originalTotal * (value / 100.0);
                                    discountType = "percentage";
                                } else {
                                    discount = value;
                                    discountType = "fixed_amount";
                                }
                                discountDescription = coupon.path("name").asText();
                            }
                            break;
                        }
                    }
                }
            }
            
            double finalTotal = Math.max(0, originalTotal - discount);
            
            ObjectNode resultData = mapper.createObjectNode();
            resultData.put("original_total", originalTotal);
            resultData.put("discount", discount);
            resultData.put("discount_type", discountType);
            resultData.put("discount_description", discountDescription);
            resultData.put("final_total", finalTotal);
            resultData.put("savings", discount);
            
            ObjectNode result = mapper.createObjectNode();
            result.put("success", true);
            result.put("message", "购物车折扣计算成功");
            result.set("data", resultData);
            
            return mapper.writeValueAsString(result);
                    
        } catch (Exception e) {
            logger.error("计算折扣失败", e);
            return "{\"success\":false,\"message\":\"计算折扣失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 应用批量折扣
     */
    private static String applyBulkDiscount(long productId, int quantity) {
        try {
            // 从数据库获取真实商品信息
            String productResult = EmshopNativeInterface.getProductDetail(productId);
            ObjectMapper mapper = new ObjectMapper();
            JsonNode productRoot = mapper.readTree(productResult);
            
            if (!productRoot.has("success") || !productRoot.get("success").asBoolean()) {
                return "{\"success\":false,\"message\":\"获取商品信息失败\"}";
            }
            
            JsonNode productData = productRoot.path("data");
            double unitPrice = productData.path("price").asDouble(0.0);
            
            if (unitPrice <= 0) {
                return "{\"success\":false,\"message\":\"商品价格无效\"}";
            }
            
            double originalTotal = unitPrice * quantity;
            double discountRate = 0.0;
            String discountDescription = "";
            
            // 批量折扣规则 - 可以从数据库的促销规则表读取
            if (quantity >= 10) {
                discountRate = 0.15; // 15%折扣
                discountDescription = "购买10件以上享受85折";
            } else if (quantity >= 5) {
                discountRate = 0.1; // 10%折扣
                discountDescription = "购买5件以上享受9折";
            } else if (quantity >= 3) {
                discountRate = 0.05; // 5%折扣
                discountDescription = "购买3件以上享受95折";
            }
            
            double discount = originalTotal * discountRate;
            double finalTotal = originalTotal - discount;
            
            ObjectNode resultData = mapper.createObjectNode();
            resultData.put("product_id", productId);
            resultData.put("product_name", productData.path("name").asText());
            resultData.put("quantity", quantity);
            resultData.put("unit_price", unitPrice);
            resultData.put("original_total", originalTotal);
            resultData.put("discount_rate", discountRate);
            resultData.put("discount", discount);
            resultData.put("final_total", finalTotal);
            resultData.put("description", discountDescription);
            
            ObjectNode result = mapper.createObjectNode();
            result.put("success", true);
            result.put("message", "批量折扣应用成功");
            result.set("data", resultData);
            
            return mapper.writeValueAsString(result);
                    
        } catch (Exception e) {
            logger.error("应用批量折扣失败", e);
            return "{\"success\":false,\"message\":\"应用批量折扣失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 检查会员折扣
     */
    private static String checkMembershipDiscount(long userId) {
        try {
            // 从数据库获取真实用户信息
            String userResult = EmshopNativeInterface.getUserInfo(userId);
            ObjectMapper mapper = new ObjectMapper();
            JsonNode userRoot = mapper.readTree(userResult);
            
            if (!userRoot.has("success") || !userRoot.get("success").asBoolean()) {
                return "{\"success\":false,\"message\":\"获取用户信息失败\"}";
            }
            
            JsonNode userData = userRoot.path("data");
            String role = userData.path("role").asText("user");
            int pointsBalance = userData.path("points_balance").asInt(0);
            
            // 根据用户角色确定会员等级和折扣
            String memberLevel = "REGULAR";
            double discountRate = 0.0;
            String benefits = "";
            
            if (role.equalsIgnoreCase("admin")) {
                memberLevel = "VIP";
                discountRate = 0.15; // 15%折扣
                benefits = "管理员VIP特权,85折优惠,免费配送,专属客服,生日特权";
            } else if (pointsBalance >= 10000) {
                memberLevel = "VIP";
                discountRate = 0.1; // 10%折扣
                benefits = "VIP专享9折,免费配送,专属客服,生日特权";
            } else if (pointsBalance >= 5000) {
                memberLevel = "GOLD";
                discountRate = 0.05; // 5%折扣
                benefits = "黄金会员95折,免费配送,优先发货";
            } else {
                memberLevel = "REGULAR";
                discountRate = 0.0;
                benefits = "普通会员,满299免运费";
            }
            
            ObjectNode resultData = mapper.createObjectNode();
            resultData.put("user_id", userId);
            resultData.put("username", userData.path("username").asText());
            resultData.put("member_level", memberLevel);
            resultData.put("discount_rate", discountRate);
            resultData.put("benefits", benefits);
            resultData.put("points_balance", pointsBalance);
            
            ObjectNode result = mapper.createObjectNode();
            result.put("success", true);
            result.put("message", "会员折扣检查成功");
            result.set("data", resultData);
            
            return mapper.writeValueAsString(result);
                    
        } catch (Exception e) {
            logger.error("检查会员折扣失败", e);
            return "{\"success\":false,\"message\":\"检查会员折扣失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 获取季节性促销 - 从数据库读取
     */
    private static String getSeasonalPromotions() {
        try {
            // 使用现有的优惠券接口获取促销信息
            String couponsResult = EmshopNativeInterface.getAvailableCoupons();
            ObjectMapper mapper = new ObjectMapper();
            JsonNode root = mapper.readTree(couponsResult);
            
            if (root.has("success") && root.get("success").asBoolean()) {
                ObjectNode result = mapper.createObjectNode();
                result.put("success", true);
                result.put("message", "获取季节性促销成功");
                
                ObjectNode data = mapper.createObjectNode();
                // 将优惠券数据映射为季节性促销
                data.set("seasonal_promotions", root.path("data").path("coupons"));
                result.set("data", data);
                
                return mapper.writeValueAsString(result);
            }
            
            return couponsResult;
        } catch (Exception e) {
            logger.error("获取季节性促销失败", e);
            return "{\"success\":false,\"message\":\"获取季节性促销失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 获取系统状态 - 真实运行时信息
     */
    private static String getSystemStatus() {
        try {
            ObjectMapper mapper = new ObjectMapper();
            ObjectNode result = mapper.createObjectNode();
            result.put("success", true);
            result.put("message", "系统运行正常");
            
            ObjectNode data = mapper.createObjectNode();
            
            // 真实的运行时信息
            Runtime runtime = Runtime.getRuntime();
            long totalMemory = runtime.totalMemory();
            long freeMemory = runtime.freeMemory();
            long usedMemory = totalMemory - freeMemory;
            long maxMemory = runtime.maxMemory();
            
            data.put("server_status", "运行中");
            data.put("active_sessions", userSessions.size());
            data.put("memory_usage", String.format("%dMB/%dMB", usedMemory / (1024 * 1024), maxMemory / (1024 * 1024)));
            data.put("memory_used_mb", usedMemory / (1024 * 1024));
            data.put("memory_max_mb", maxMemory / (1024 * 1024));
            data.put("memory_free_mb", freeMemory / (1024 * 1024));
            data.put("available_processors", runtime.availableProcessors());
            
            // 尝试获取数据库状态
            try {
                String initStatus = EmshopNativeInterface.getInitializationStatus();
                JsonNode initRoot = mapper.readTree(initStatus);
                if (initRoot.has("success") && initRoot.get("success").asBoolean()) {
                    data.put("database_status", "已连接");
                    data.put("jni_status", "正常");
                } else {
                    data.put("database_status", "未知");
                    data.put("jni_status", "异常");
                }
            } catch (Exception e) {
                data.put("database_status", "连接失败");
                data.put("jni_status", "异常: " + e.getMessage());
            }
            
            data.put("last_updated", new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new java.util.Date()));
            
            result.set("data", data);
            return mapper.writeValueAsString(result);
            
        } catch (Exception e) {
            logger.error("获取系统状态失败", e);
            return "{\"success\":false,\"message\":\"获取系统状态失败: " + e.getMessage() + "\"}";
        }
    }
    
    /**
     * 获取功能完成状态 - 项目统计（保留用于演示）
     */
    private static String getFeatureCompletionStatus() {
        return "{\"success\":true,\"data\":{" +
               "\"overall_completion\":\"95%\"," +
               "\"note\":\"此为项目功能完成度统计，非实时数据\"," +
               "\"modules\":{" +
               "\"user_management\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"shopping_cart\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"order_management\":{\"completion\":\"95%\",\"status\":\"完成\"}," +
               "\"payment_system\":{\"completion\":\"95%\",\"status\":\"完成\"}," +
               "\"promotion_system\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"coupon_system\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"address_management\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"product_management\":{\"completion\":\"100%\",\"status\":\"完成\"}," +
               "\"ui_interface\":{\"completion\":\"90%\",\"status\":\"Qt客户端完成\"}" +
               "}},\"message\":\"功能完成度统计\"}";
    }

    /**
     * 服务器启动入口
     */
    public static void main(String[] args) {
        // 在服务器启动前强制加载Native Library
        try {
            logger.info("Initializing native library...");
            // 触发EmshopNativeInterface的static块执行
            Class.forName("emshop.EmshopNativeInterface");
            logger.info("Native library initialization completed");
        } catch (ClassNotFoundException e) {
            logger.error("Failed to load EmshopNativeInterface class - error={}", e.getMessage(), e);
            System.err.println("FATAL: Cannot load native library. Server will not start.");
            System.exit(1);
        } catch (UnsatisfiedLinkError e) {
            logger.error("Failed to load native library - error={}", e.getMessage(), e);
            System.err.println("FATAL: Native library loading failed. Please ensure emshop_native_oop.dll and libmysql.dll are in the correct path.");
            System.err.println("Expected path: " + System.getProperty("user.dir"));
            System.exit(1);
        }
        
        int port = 8081;
        if (args.length > 0) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                logger.error("Invalid port number, using default port 8081 - error={}", e.getMessage());
                port = 8081;
            }
        }

        EmshopNettyServer server = new EmshopNettyServer(port);
        
        // 添加关闭钩子
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            logger.info("Shutdown hook triggered, stopping server...");
            server.shutdown();
        }));
        
        try {
            logger.info("Starting Emshop Netty Server on port {}...", port);
            server.start();
        } catch (InterruptedException e) {
            logger.error("Server interrupted - error={}", e.getMessage(), e);
            Thread.currentThread().interrupt();
        }
    }
}
