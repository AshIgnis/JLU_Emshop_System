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
 * 基于Netty的Emshop客户端
 * 连接到服务器并发送请求
 */
public class EmshopNettyClient {
    private final String host;
    private final int port;
    private Channel channel;
    private EventLoopGroup group;

    public EmshopNettyClient(String host, int port) {
        this.host = host;
        this.port = port;
    }

    /**
     * 连接到服务器
     */
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
                            
                            // 添加解码器，以换行符为分隔符
                            pipeline.addLast("framer", new DelimiterBasedFrameDecoder(8192, Delimiters.lineDelimiter()));
                            // 字符串解码器和编码器
                            pipeline.addLast("decoder", new StringDecoder(CharsetUtil.UTF_8));
                            pipeline.addLast("encoder", new StringEncoder(CharsetUtil.UTF_8));
                            // 客户端处理器
                            pipeline.addLast("handler", new EmshopClientHandler());
                        }
                    });

            // 连接到服务器
            ChannelFuture future = bootstrap.connect(host, port).sync();
            channel = future.channel();
            System.out.println("Connected to Emshop Server at " + host + ":" + port);

            // 启动用户输入处理线程
            startUserInputThread();

            // 等待连接关闭
            channel.closeFuture().sync();

        } finally {
            disconnect();
        }
    }

    /**
     * 断开连接
     */
    public void disconnect() {
        if (channel != null) {
            channel.close();
        }
        if (group != null) {
            group.shutdownGracefully();
        }
        System.out.println("Disconnected from server.");
    }

    /**
     * 发送消息到服务器
     */
    public void sendMessage(String message) {
        if (channel != null && channel.isActive()) {
            channel.writeAndFlush(message + "\n");
        }
    }

    /**
     * 启动用户输入处理线程
     */
    private void startUserInputThread() {
        Thread inputThread = new Thread(() -> {
            Scanner scanner = new Scanner(System.in);
            System.out.println("=== Emshop Client Console ===");
            System.out.println("Type 'help' to show available commands, 'quit' to exit");
            System.out.println("=============================");

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

    /**
     * 显示帮助菜单
     */
    private void showHelpMenu() {
        System.out.println("\n=== Emshop Client Help Menu ===");
        System.out.println("=== User Authentication ===");
        System.out.println("LOGIN <username> <password>");
        System.out.println("REGISTER <username> <password> <phone>");
        System.out.println();
        System.out.println("=== Product Management ===");
        System.out.println("GET_PRODUCTS <category> <page> <pageSize>");
        System.out.println("SEARCH_PRODUCTS <keyword> <page> <pageSize>");
        System.out.println();
        System.out.println("=== Shopping Cart ===");
        System.out.println("ADD_TO_CART <userId> <productId> <quantity>");
        System.out.println("GET_CART <userId>");
        System.out.println("UPDATE_CART <userId> <productId> <quantity>");
        System.out.println("REMOVE_FROM_CART <userId> <productId>");
        System.out.println("CLEAR_CART <userId>");
        System.out.println();
        System.out.println("=== User Addresses ===");
        System.out.println("ADD_ADDRESS <userId> <receiverName> <phone> <province> <city> <district> <detailAddress>");
        System.out.println("GET_USER_ADDRESSES <userId>");
        System.out.println("UPDATE_ADDRESS <addressId> <receiverName> <phone> <province> <city> <district> <detailAddress>");
        System.out.println("DELETE_ADDRESS <addressId>");
        System.out.println("SET_DEFAULT_ADDRESS <userId> <addressId>");
        System.out.println();
        System.out.println("=== Order Management ===");
        System.out.println("CREATE_ORDER <userId> <addressId> <couponId> <paymentMethod>");
        System.out.println("GET_USER_ORDERS <userId> <page> <pageSize>");
        System.out.println("GET_ORDER_DETAIL <orderId>");
        System.out.println("PAY_ORDER <orderId> <paymentMethod>");
        System.out.println("SHIP_ORDER <orderId> <trackingNumber> <shippingMethod>");
        System.out.println("CONFIRM_DELIVERY <orderId>");
        System.out.println("REQUEST_REFUND <orderId> <reason>");
        System.out.println("GET_ORDERS_BY_STATUS <userId> <status>");
        System.out.println("TRACK_ORDER <orderId>");
        System.out.println("GET_ALL_ORDERS <status> <page> <pageSize> <startDate> <endDate>");
        System.out.println("UPDATE_ORDER_STATUS <orderId> <status>");
        System.out.println("CANCEL_ORDER <orderId>");
        System.out.println();
        System.out.println("=== Coupon System ===");
        System.out.println("GET_AVAILABLE_COUPONS <userId>");
        System.out.println("GET_USER_COUPONS <userId>");
        System.out.println("USE_COUPON <userId> <couponId>");
        System.out.println();
        System.out.println("=== Product Reviews ===");
        System.out.println("ADD_REVIEW <userId> <productId> <rating> <comment>");
        System.out.println("GET_PRODUCT_REVIEWS <productId> <page> <pageSize>");
        System.out.println("GET_USER_REVIEWS <userId> <page> <pageSize>");
        System.out.println("UPDATE_REVIEW <reviewId> <rating> <comment>");
        System.out.println("DELETE_REVIEW <reviewId>");
        System.out.println();
        System.out.println("Type 'quit' to exit, 'help' to show this menu again");
        System.out.println("===============================\n");
    }

    /**
     * 客户端消息处理器
     */
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

    /**
     * 客户端启动入口
     */
    public static void main(String[] args) {
        String host = "localhost";
        int port = 8080;

        // 解析命令行参数
        if (args.length >= 1) {
            host = args[0];
        }
        if (args.length >= 2) {
            try {
                port = Integer.parseInt(args[1]);
            } catch (NumberFormatException e) {
                System.err.println("Invalid port number, using default port 8080");
            }
        }

        EmshopNettyClient client = new EmshopNettyClient(host, port);

        try {
            client.connect();
        } catch (InterruptedException e) {
            System.err.println("Client interrupted: " + e.getMessage());
            Thread.currentThread().interrupt();
        }
    }
}
