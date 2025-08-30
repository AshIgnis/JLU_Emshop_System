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
                // 调用业务分发方法
                String response = EmshopServer.dispatch(request.trim());
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
