package emshop;

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
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

/**
 * 基于Netty的Emshop服务器端
 * 负责接收客户端请求并分发到业务处理模块
 */
public class EmshopNettyServer {
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
        
        public UserSession() {
            this.isLoggedIn = false;
        }
        
        public UserSession(long userId, String username, String role) {
            this.userId = userId;
            this.username = username;
            this.role = role;
            this.isLoggedIn = true;
        }
        
        // Getters and setters
        public long getUserId() { return userId; }
        public String getUsername() { return username; }
        public String getRole() { return role; }
        public boolean isLoggedIn() { return isLoggedIn; }
        public boolean isAdmin() { return "admin".equals(role); }
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
            System.out.println("Emshop Netty Server started on port: " + port);
            
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
        System.out.println("Emshop Netty Server shutdown completed.");
    }

    /**
     * 服务器业务处理器
     */
    private static class EmshopServerHandler extends SimpleChannelInboundHandler<String> {
        
        @Override
        public void channelActive(ChannelHandlerContext ctx) {
            System.out.println("Client connected: " + ctx.channel().remoteAddress());
            // 初始化用户会话
            userSessions.put(ctx.channel().id(), new UserSession());
            ctx.writeAndFlush("Welcome to Emshop Server! Please login to access features.\n");
        }

        @Override
        public void channelInactive(ChannelHandlerContext ctx) {
            System.out.println("Client disconnected: " + ctx.channel().remoteAddress());
            // 清理用户会话
            UserSession session = userSessions.remove(ctx.channel().id());
            if (session != null && session.isLoggedIn()) {
                System.out.println("User " + session.getUsername() + " logged out");
            }
        }

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, String request) {
            System.out.println("Received from " + ctx.channel().remoteAddress() + ": " + request);
            
            try {
                // 调用JNI接口处理请求
                String response = processRequest(ctx, request.trim());
                ctx.writeAndFlush(response + "\n");
            } catch (Exception e) {
                System.err.println("Error processing request: " + e.getMessage());
                ctx.writeAndFlush("ERROR: " + e.getMessage() + "\n");
            }
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
            System.err.println("Server exception: " + cause.getMessage());
            cause.printStackTrace();
            ctx.close();
        }
        
        /**
         * 处理客户端请求，调用C++实现的JNI接口
         */
        private String processRequest(ChannelHandlerContext ctx, String request) {
            try {
                // 解析请求格式：METHOD PARAM1 PARAM2 ... (用空格分隔)
                String[] parts = request.split("\\s+");
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
                            String loginResult = EmshopNativeInterface.login(parts[1], parts[2]);
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
                                    
                                    session = new UserSession(userId, parts[1], role);
                                    userSessions.put(ctx.channel().id(), session);
                                    System.out.println("User " + parts[1] + " logged in with role: " + role);
                                } catch (Exception e) {
                                    System.err.println("Failed to parse login response: " + e.getMessage());
                                }
                            }
                            return loginResult;
                        }
                        break;
                        
                    case "REGISTER":
                        if (parts.length >= 4) {
                            return EmshopNativeInterface.register(parts[1], parts[2], parts[3]);
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
                        
                    // === User Addresses ===
                    case "ADD_ADDRESS":
                        if (parts.length >= 8) {
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
                            for (int i = 7; i < parts.length; i++) {                            LOGIN testuser password123
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
                                return EmshopNativeInterface.createOrderFromCart(session.getUserId(), addressId, couponCode, remark);
                            }
                        } else if (parts.length >= 5) {
                            // Backward compatible: CREATE_ORDER userId addressId couponCode remark
                            long userId = Long.parseLong(parts[1]);
                            long addressId = Long.parseLong(parts[2]);
                            String couponCode = parts[3].equals("0") ? null : parts[3];
                            String remark = parts.length > 4 ? parts[4] : "";
                            return EmshopNativeInterface.createOrderFromCart(userId, addressId, couponCode, remark);
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
                        
                    case "GET_ORDER_DETAIL":
                        if (parts.length >= 2) {
                            long orderId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getOrderDetail(orderId);
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
                        if (parts.length >= 3) {
                            long userId = Long.parseLong(parts[1]);
                            long orderId = Long.parseLong(parts[2]);
                            return EmshopNativeInterface.cancelOrder(userId, orderId);
                        }
                        break;
                        
                    // === Coupon System ===
                    case "GET_AVAILABLE_COUPONS":
                        return EmshopNativeInterface.getAvailableCoupons();
                        
                    case "GET_USER_COUPONS":
                        if (parts.length >= 2) {
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
                        
                    default:
                        return "{\"success\":false,\"message\":\"Unknown method: " + method + "\"}";
                }
                
                return "{\"success\":false,\"message\":\"Invalid parameters for method: " + method + "\"}";
                
            } catch (NumberFormatException e) {
                return "{\"success\":false,\"message\":\"Invalid number format: " + e.getMessage() + "\"}";
            } catch (Exception e) {
                return "{\"success\":false,\"message\":\"Error processing request: " + e.getMessage() + "\"}";
            }
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
                    return response.substring(start, end);
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
                                return userInfoSection.substring(start, end);
                            }
                        }
                    }
                }
            }
            
            return "user"; // 默认为普通用户
        }
    }

    /**
     * 服务器启动入口
     */
    public static void main(String[] args) {
        int port = 8080;
        if (args.length > 0) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                System.err.println("Invalid port number, using default port 8080");
            }
        }

        EmshopNettyServer server = new EmshopNettyServer(port);
        
        // 添加关闭钩子
        Runtime.getRuntime().addShutdownHook(new Thread(server::shutdown));
        
        try {
            server.start();
        } catch (InterruptedException e) {
            System.err.println("Server interrupted: " + e.getMessage());
            Thread.currentThread().interrupt();
        }
    }
}
