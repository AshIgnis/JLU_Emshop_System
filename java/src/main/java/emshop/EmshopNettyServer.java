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
                // 解析请求格式：METHOD:PARAMS
                String[] parts = request.split(":", 2);
                if (parts.length < 1) {
                    return "{\"success\":false,\"message\":\"Invalid request format\"}";
                }
                
                String method = parts[0].toUpperCase();
                String params = parts.length > 1 ? parts[1] : "";
                
                // 根据方法名调用相应的JNI接口
                switch (method) {
                    case "LOGIN":
                        String[] loginParams = params.split(",");
                        if (loginParams.length >= 2) {
                            return EmshopNativeInterface.login(loginParams[0], loginParams[1]);
                        }
                        break;
                        
                    case "REGISTER":
                        String[] regParams = params.split(",");
                        if (regParams.length >= 3) {
                            return EmshopNativeInterface.register(regParams[0], regParams[1], regParams[2]);
                        }
                        break;
                        
                    case "GET_PRODUCTS":
                        String[] prodParams = params.split(",");
                        String category = prodParams.length > 0 ? prodParams[0] : "all";
                        int page = prodParams.length > 1 ? Integer.parseInt(prodParams[1]) : 1;
                        int pageSize = prodParams.length > 2 ? Integer.parseInt(prodParams[2]) : 10;
                        return EmshopNativeInterface.getProductList(category, page, pageSize);
                        
                    case "ADD_TO_CART":
                        String[] cartParams = params.split(",");
                        if (cartParams.length >= 3) {
                            long userId = Long.parseLong(cartParams[0]);
                            long productId = Long.parseLong(cartParams[1]);
                            int quantity = Integer.parseInt(cartParams[2]);
                            return EmshopNativeInterface.addToCart(userId, productId, quantity);
                        }
                        break;
                        
                    case "GET_CART":
                        if (!params.isEmpty()) {
                            long userId = Long.parseLong(params);
                            return EmshopNativeInterface.getCart(userId);
                        }
                        break;
                        
                    case "PING":
                        return "{\"success\":true,\"message\":\"Server is running\",\"timestamp\":" + System.currentTimeMillis() + "}";
                        
                    default:
                        return "{\"success\":false,\"message\":\"Unknown method: " + method + "\"}";
                }
                
                return "{\"success\":false,\"message\":\"Invalid parameters for method: " + method + "\"}";
                
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
