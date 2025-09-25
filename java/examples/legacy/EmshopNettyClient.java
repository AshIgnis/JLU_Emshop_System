package emshop;

import io.netty.bootstrap.Bootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioSocketChannel;
import io.netty.handler.codec.DelimiterBasedFrameDecoder;
import io.netty.handler.codec.Delimiters;
import io.netty.handler.codec.string.StringDecoder;
import io.netty.handler.codec.string.StringEncoder;
import io.netty.util.CharsetUtil;

import java.util.Scanner;

/**
 * 基于Netty的 Emshop 控制台客户端（示例/遗留）
 */
@Deprecated
public class EmshopNettyClient {
    private final String host;
    private final int port;
    private Channel channel;
    private EventLoopGroup group;

    public EmshopNettyClient(String host, int port) {
        this.host = host;
        this.port = port;
    }

    public void connect() throws InterruptedException {
        group = new NioEventLoopGroup();

        try {
            Bootstrap bootstrap = new Bootstrap();
            bootstrap.group(group)
                    .channel(NioSocketChannel.class)
                    .option(ChannelOption.TCP_NODELAY, true)
                    .handler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        protected void initChannel(SocketChannel ch) {
                            ChannelPipeline pipeline = ch.pipeline();
                            pipeline.addLast("framer", new DelimiterBasedFrameDecoder(8192, Delimiters.lineDelimiter()));
                            pipeline.addLast("decoder", new StringDecoder(CharsetUtil.UTF_8));
                            pipeline.addLast("encoder", new StringEncoder(CharsetUtil.UTF_8));
                            pipeline.addLast("handler", new EmshopClientHandler());
                        }
                    });

            ChannelFuture future = bootstrap.connect(host, port).sync();
            channel = future.channel();
            System.out.println("Connected to Emshop Server at " + host + ":" + port);

            startUserInputThread();

            channel.closeFuture().sync();
        } finally {
            disconnect();
        }
    }

    public void disconnect() {
        if (channel != null) {
            channel.close();
        }
        if (group != null) {
            group.shutdownGracefully();
        }
        System.out.println("Disconnected from server.");
    }

    public void sendMessage(String message) {
        if (channel != null && channel.isActive()) {
            channel.writeAndFlush(message + "\n");
        }
    }

    private void startUserInputThread() {
        Thread inputThread = new Thread(() -> {
            Scanner scanner = new Scanner(System.in);
            System.out.println("=== Emshop Client Console ===");
            System.out.println("Type 'help' to show available commands, 'quit' to exit");

            while (scanner.hasNextLine()) {
                String input = scanner.nextLine().trim();
                if ("quit".equalsIgnoreCase(input)) {
                    disconnect();
                    break;
                } else if ("help".equalsIgnoreCase(input)) {
                    showHelpMenu();
                    continue;
                }
                if (!input.isEmpty()) {
                    sendMessage(input);
                }
            }
            scanner.close();
        });
        inputThread.setDaemon(true);
        inputThread.start();
    }

    private void showHelpMenu() {
        System.out.println("\n=== Emshop Client Help Menu ===");
        System.out.println("Type 'quit' to exit, 'help' to show this menu again");
    }

    private static class EmshopClientHandler extends SimpleChannelInboundHandler<String> {
        @Override
        public void channelActive(ChannelHandlerContext ctx) {
            System.out.println("Connection established with server.");
        }

        @Override
        public void channelInactive(ChannelHandlerContext ctx) {
            System.out.println("Connection lost.");
        }

        @Override
        protected void channelRead0(ChannelHandlerContext ctx, String response) {
            System.out.println("Server: " + response);
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
            System.err.println("Client exception: " + cause.getMessage());
            cause.printStackTrace();
            ctx.close();
        }
    }

    public static void main(String[] args) {
        String host = "localhost";
        int port = 8080;
        if (args.length >= 1) host = args[0];
        if (args.length >= 2) {
            try { port = Integer.parseInt(args[1]); } catch (NumberFormatException ignored) {}
        }
        EmshopNettyClient client = new EmshopNettyClient(host, port);
        try { client.connect(); } catch (InterruptedException e) { Thread.currentThread().interrupt(); }
    }
}
