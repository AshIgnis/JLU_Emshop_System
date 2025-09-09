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

/**
 * 基于Netty的Emshop服务器端
 * 负责接收客户端请求并分发到业务处理模块
 */
public class EmshopNettyServer {
    private final int port;
    private Channel channel;
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;

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
            ctx.writeAndFlush("Welcome to Emshop Server!\n");
        }

        @Override
        public void channelInactive(ChannelHandlerContext ctx) {
            System.out.println("Client disconnected: " + ctx.channel().remoteAddress());
        }

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, String request) {
            System.out.println("Received from " + ctx.channel().remoteAddress() + ": " + request);
            
            try {
                // 调用JNI接口处理请求
                String response = processRequest(request.trim());
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
        private String processRequest(String request) {
            try {
                // 解析请求格式：METHOD PARAM1 PARAM2 ... (用空格分隔)
                String[] parts = request.split("\\s+");
                if (parts.length < 1) {
                    return "{\"success\":false,\"message\":\"Invalid request format\"}";
                }
                
                String method = parts[0].toUpperCase();
                
                // 根据方法名调用相应的JNI接口
                switch (method) {
                    // === User Authentication ===
                    case "LOGIN":
                        if (parts.length >= 3) {
                            return EmshopNativeInterface.login(parts[1], parts[2]);
                        }
                        break;
                        
                    case "REGISTER":
                        if (parts.length >= 4) {
                            return EmshopNativeInterface.register(parts[1], parts[2], parts[3]);
                        }
                        break;
                        
                    // === Product Management ===
                    case "GET_PRODUCTS":
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
                        if (parts.length >= 4) {
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            int quantity = Integer.parseInt(parts[3]);
                            return EmshopNativeInterface.addToCart(userId, productId, quantity);
                        }
                        break;
                        
                    case "GET_CART":
                        if (parts.length >= 2) {
                            long userId = Long.parseLong(parts[1]);
                            return EmshopNativeInterface.getCart(userId);
                        }
                        break;
                        
                    case "UPDATE_CART":
                        if (parts.length >= 4) {
                            long userId = Long.parseLong(parts[1]);
                            long productId = Long.parseLong(parts[2]);
                            int quantity = Integer.parseInt(parts[3]);
                            return EmshopNativeInterface.updateCartItemQuantity(userId, productId, quantity);
                        }
                        break;
                        
                    case "REMOVE_FROM_CART":
                        if (parts.length >= 3) {
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
                        if (parts.length >= 2) {
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
                        if (parts.length >= 5) {
                            long userId = Long.parseLong(parts[1]);
                            long addressId = Long.parseLong(parts[2]);
                            String couponCode = parts[3].equals("0") ? null : parts[3];
                            String remark = parts.length > 4 ? parts[4] : "";
                            return EmshopNativeInterface.createOrderFromCart(userId, addressId, couponCode, remark);
                        }
                        break;
                        
                    case "GET_USER_ORDERS":
                        if (parts.length >= 2) {
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
                            long productId = Long.parseLong(parts[1]);
                            int quantity = Integer.parseInt(parts[2]);
                            String operation = parts[3]; // "add" or "subtract"
                            return EmshopNativeInterface.updateStock(productId, quantity, operation);
                        }
                        break;
                        
                    case "GET_LOW_STOCK_PRODUCTS":
                        if (parts.length >= 2) {
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
