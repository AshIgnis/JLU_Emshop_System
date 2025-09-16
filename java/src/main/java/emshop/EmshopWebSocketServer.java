package emshop;

import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.*;
import io.netty.channel.group.ChannelGroup;
import io.netty.channel.group.DefaultChannelGroup;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.http.HttpObjectAggregator;
import io.netty.handler.codec.http.HttpServerCodec;
import io.netty.handler.codec.http.websocketx.WebSocketServerProtocolHandler;
import io.netty.handler.codec.http.websocketx.extensions.compression.WebSocketServerCompressionHandler;
import io.netty.handler.ssl.SslContext;
import io.netty.handler.ssl.SslContextBuilder;
import io.netty.handler.ssl.util.SelfSignedCertificate;
import io.netty.handler.stream.ChunkedWriteHandler;
import io.netty.handler.timeout.IdleStateHandler;
import io.netty.util.AttributeKey;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.security.cert.CertificateException;
import javax.net.ssl.SSLException;

/**
 * 支持 WebSocket (WSS) 的 Emshop 服务器
 * 为 Qt 前端提供实时通信支持
 */
public class EmshopWebSocketServer {
    private final int port;
    private final boolean ssl;
    private Channel channel;
    private EventLoopGroup bossGroup;
    private EventLoopGroup workerGroup;
    private SslContext sslContext;
    
    // WebSocket 会话管理
    public static final AttributeKey<Long> USER_ID_ATTR = AttributeKey.valueOf("userId");
    public static final AttributeKey<String> USERNAME_ATTR = AttributeKey.valueOf("username");
    public static final AttributeKey<String> ROLE_ATTR = AttributeKey.valueOf("role");
    public static final AttributeKey<Boolean> AUTHENTICATED_ATTR = AttributeKey.valueOf("authenticated");
    
    // 活跃连接管理
    private static final ConcurrentHashMap<Long, ChannelGroup> userChannels = new ConcurrentHashMap<>();
    private static final ChannelGroup allChannels = new DefaultChannelGroup(
        io.netty.util.concurrent.GlobalEventExecutor.INSTANCE);
    
    public EmshopWebSocketServer(int port, boolean ssl) {
        this.port = port;
        this.ssl = ssl;
    }
    
    /**
     * 初始化 SSL 上下文
     */
    private void initSSL() throws CertificateException, SSLException {
        if (ssl) {
            // 在生产环境中，应该使用真实的证书
            // 这里使用自签名证书仅用于开发和测试
            SelfSignedCertificate ssc = new SelfSignedCertificate();
            sslContext = SslContextBuilder.forServer(ssc.certificate(), ssc.privateKey()).build();
            System.out.println("SSL/TLS enabled with self-signed certificate");
        }
    }
    
    /**
     * 启动服务器
     */
    public void start() throws InterruptedException {
        // 初始化 SSL
        try {
            initSSL();
        } catch (Exception e) {
            System.err.println("Failed to initialize SSL: " + e.getMessage());
            throw new RuntimeException("SSL initialization failed", e);
        }
        
        // 创建事件循环组
        bossGroup = new NioEventLoopGroup(1);
        workerGroup = new NioEventLoopGroup();
        
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
                            
                            // 添加 SSL 处理器（如果启用）
                            if (sslContext != null) {
                                pipeline.addLast(sslContext.newHandler(ch.alloc()));
                            }
                            
                            // HTTP 编解码器
                            pipeline.addLast(new HttpServerCodec());
                            // HTTP 对象聚合器，用于处理完整的 HTTP 请求
                            pipeline.addLast(new HttpObjectAggregator(65536));
                            // 支持写大数据流
                            pipeline.addLast(new ChunkedWriteHandler());
                            // WebSocket 压缩支持
                            pipeline.addLast(new WebSocketServerCompressionHandler());
                            // WebSocket 协议处理器
                            pipeline.addLast(new WebSocketServerProtocolHandler("/ws", null, true));
                            // 空闲状态检测（心跳检测）
                            pipeline.addLast(new IdleStateHandler(60, 0, 0, TimeUnit.SECONDS));
                            // WebSocket 消息处理器
                            pipeline.addLast(new WebSocketMessageHandler());
                        }
                    });
                    
            // 绑定端口并开始接收连接
            ChannelFuture future = bootstrap.bind(port).sync();
            channel = future.channel();
            
            String protocol = ssl ? "wss" : "ws";
            System.out.println("Emshop WebSocket Server started on " + protocol + "://localhost:" + port + "/ws");
            System.out.println("Ready for Qt QWebSocket connections...");
            
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
        // 关闭所有连接
        allChannels.close().awaitUninterruptibly();
        
        if (channel != null) {
            channel.close();
        }
        if (bossGroup != null) {
            bossGroup.shutdownGracefully();
        }
        if (workerGroup != null) {
            workerGroup.shutdownGracefully();
        }
        System.out.println("Emshop WebSocket Server shutdown completed.");
    }
    
    /**
     * 获取用户的所有连接通道
     */
    public static ChannelGroup getUserChannels(Long userId) {
        return userChannels.get(userId);
    }
    
    /**
     * 为用户添加通道
     */
    public static void addUserChannel(Long userId, Channel channel) {
        userChannels.computeIfAbsent(userId, k -> new DefaultChannelGroup(
            io.netty.util.concurrent.GlobalEventExecutor.INSTANCE)).add(channel);
        allChannels.add(channel);
    }
    
    /**
     * 移除用户通道
     */
    public static void removeUserChannel(Long userId, Channel channel) {
        ChannelGroup group = userChannels.get(userId);
        if (group != null) {
            group.remove(channel);
            if (group.isEmpty()) {
                userChannels.remove(userId);
            }
        }
        allChannels.remove(channel);
    }
    
    /**
     * 向指定用户推送消息
     */
    public static void sendToUser(Long userId, String message) {
        ChannelGroup channels = userChannels.get(userId);
        if (channels != null) {
            channels.writeAndFlush(new io.netty.handler.codec.http.websocketx.TextWebSocketFrame(message));
        }
    }
    
    /**
     * 向所有连接的用户广播消息
     */
    public static void broadcast(String message) {
        allChannels.writeAndFlush(new io.netty.handler.codec.http.websocketx.TextWebSocketFrame(message));
    }
    
    /**
     * 获取在线用户数量
     */
    public static int getOnlineUserCount() {
        return userChannels.size();
    }
    
    /**
     * 获取总连接数量
     */
    public static int getTotalConnectionCount() {
        return allChannels.size();
    }
    
    /**
     * 服务器启动入口
     */
    public static void main(String[] args) {
        int port = 8081; // 使用不同的端口以避免与现有服务器冲突
        boolean ssl = true; // 默认启用 SSL
        
        if (args.length > 0) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException e) {
                System.err.println("Invalid port number, using default port 8081");
            }
        }
        
        if (args.length > 1) {
            ssl = Boolean.parseBoolean(args[1]);
        }
        
        EmshopWebSocketServer server = new EmshopWebSocketServer(port, ssl);
        
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