package emshop;
import java.io.*;
import java.net.*;

public class EmshopCSCore {
    // ================== 服务器端 ==================
    public static class Server {
        private int port;
        private ServerSocket serverSocket;
        private boolean running = false;

        public Server(int port) { this.port = port; }

        public void start() throws IOException {
            serverSocket = new ServerSocket(port);
            running = true;
            System.out.println("Server started on port " + port);
            while (running) {
                Socket clientSocket = serverSocket.accept();
                new Thread(new ClientHandler(clientSocket)).start();
            }
        }

        public void stop() throws IOException {
            running = false;
            if (serverSocket != null) serverSocket.close();
        }

        // 处理客户端连接
        private static class ClientHandler implements Runnable {
            private Socket socket;
            public ClientHandler(Socket socket) { this.socket = socket; }
            @Override
            public void run() {
                try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                     PrintWriter out = new PrintWriter(socket.getOutputStream(), true)) {
                    String request;
                    while ((request = in.readLine()) != null) {
                        // 协议解析与业务分发
                        String response = EmshopServer.dispatch(request);
                        out.println(response);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    try { socket.close(); } catch (IOException ignored) {}
                }
            }
        }
    }

    // ================== 客户端 ==================
    public static class Client {
        private String host;
        private int port;
        private Socket socket;

        public Client(String host, int port) {
            this.host = host;
            this.port = port;
        }

        public void start() throws IOException {
            socket = new Socket(host, port);
            try (BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                 PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
                 BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in))) {
                String userInput;
                System.out.println("Connected to server. Type your request:");
                while ((userInput = stdIn.readLine()) != null) {
                    out.println(userInput);
                    String response = in.readLine();
                    System.out.println("Server: " + response);
                }
            } finally {
                socket.close();
            }
        }
    }
}

/**
 * 业务接口声明，所有方法native，实际由C++实现
 * 并提供dispatch方法用于协议分发
 */
class EmshopServer {
    // 协议分发（根据请求内容调用对应native方法）
    public static String dispatch(String request) {
        // 这里只做协议分发示例，实际应解析request后调用对应native方法
        // 例如：if (request.startsWith("LOGIN")) return login(...);
        return "[JNI call placeholder]";
    }

    // 用户管理
    public static native String login(String username, String password); // 用户登录
    public static native String register(String username, String password, String phone); // 用户注册
    public static native String logout(long userId); // 用户登出
    public static native String getUserInfo(long userId); // 获取用户信息
    public static native String updateUserInfo(long userId, String jsonInfo); // 更新用户信息

    // 商品管理
    public static native String getProductList(String category, int page, int pageSize); // 获取商品列表
    public static native String getProductDetail(long productId); // 获取商品详情
    public static native String addProduct(String jsonProduct); // 添加商品（管理员）
    public static native String updateProduct(long productId, String jsonProduct); // 更新商品
    public static native String deleteProduct(long productId); // 删除商品

    // 购物车
    public static native String addToCart(long userId, long productId, int quantity); // 加入购物车
    public static native String getCart(long userId); // 获取购物车内容
    public static native String removeFromCart(long userId, long productId); // 移除购物车商品

    // 结算与订单
    public static native String checkout(long userId); // 结算购物车
    public static native String getOrderList(long userId); // 获取订单列表
    public static native String getOrderDetail(long orderId); // 获取订单详情
    public static native String cancelOrder(long userId, long orderId); // 取消订单

    // 售后服务
    public static native String afterSale(long userId, long orderId, String reason); // 售后服务申请

    // 促销策略
    public static native String applyPromotion(long userId, String promoCode); // 应用促销策略

    // UI风格
    public static native String getAvailableThemes(); // 获取可用UI主题
    public static native String setTheme(long userId, String themeName); // 设置用户UI主题

    // 并发与互斥
    public static native String lockProduct(long productId); // 商品加锁（限量/秒杀）
    public static native String unlockProduct(long productId); // 商品解锁

    // 数据库操作/映射
    public static native String executeSQL(String sql); // 执行SQL语句（仅管理员）

    // 日志与监控
    public static native String getSystemLog(int page, int pageSize); // 获取系统日志
    public static native String getServerStatus(); // 获取服务器状态
}
