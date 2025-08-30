package emshop;
/**
 * 模拟数据提供者
 * 当C++本地库不可用时，提供模拟的业务数据
 * 用于开发和测试阶段
 */
public class MockDataProvider {
    
    /**
     * 模拟用户登录
     */
    public static String mockLogin(String username, String password) {
        if ("admin".equals(username) && "123456".equals(password)) {
            return "{\"status\":\"success\",\"userId\":1001,\"username\":\"admin\",\"role\":\"admin\"}";
        } else if ("user".equals(username) && "password".equals(password)) {
            return "{\"status\":\"success\",\"userId\":1002,\"username\":\"user\",\"role\":\"customer\"}";
        } else {
            return "{\"status\":\"error\",\"message\":\"Invalid username or password\"}";
        }
    }

    /**
     * 模拟用户注册
     */
    public static String mockRegister(String username, String password, String phone) {
        if (username.length() < 3) {
            return "{\"status\":\"error\",\"message\":\"Username too short\"}";
        }
        if (password.length() < 6) {
            return "{\"status\":\"error\",\"message\":\"Password too short\"}";
        }
        if (!phone.matches("\\d{11}")) {
            return "{\"status\":\"error\",\"message\":\"Invalid phone number\"}";
        }
        
        return "{\"status\":\"success\",\"userId\":2001,\"message\":\"User registered successfully\"}";
    }

    /**
     * 模拟获取商品列表
     */
    public static String mockGetProductList(String category, int page, int pageSize) {
        StringBuilder json = new StringBuilder();
        json.append("{\"status\":\"success\",\"category\":\"").append(category).append("\",");
        json.append("\"page\":").append(page).append(",\"pageSize\":").append(pageSize).append(",");
        json.append("\"totalItems\":50,\"products\":[");
        
        for (int i = 0; i < Math.min(pageSize, 5); i++) {
            if (i > 0) json.append(",");
            int productId = (page - 1) * pageSize + i + 1;
            json.append("{\"id\":").append(productId);
            json.append(",\"name\":\"").append(category).append(" Product ").append(productId).append("\"");
            json.append(",\"price\":").append(99.99 + i * 10);
            json.append(",\"stock\":").append(100 - i * 10);
            json.append(",\"description\":\"High quality ").append(category).append(" product\"}");
        }
        
        json.append("]}");
        return json.toString();
    }

    /**
     * 模拟获取商品详情
     */
    public static String mockGetProductDetail(long productId) {
        return "{\"status\":\"success\",\"product\":{" +
                "\"id\":" + productId + "," +
                "\"name\":\"Product " + productId + "\"," +
                "\"price\":299.99," +
                "\"stock\":50," +
                "\"category\":\"electronics\"," +
                "\"description\":\"Detailed description of product " + productId + "\"," +
                "\"images\":[\"img1.jpg\",\"img2.jpg\"]," +
                "\"rating\":4.5," +
                "\"reviews\":128" +
                "}}";
    }

    /**
     * 模拟加入购物车
     */
    public static String mockAddToCart(long userId, long productId, int quantity) {
        return "{\"status\":\"success\",\"message\":\"Added " + quantity + 
               " items of product " + productId + " to cart for user " + userId + "\"}";
    }

    /**
     * 模拟获取购物车
     */
    public static String mockGetCart(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"items\":[" +
                "{\"productId\":2001,\"productName\":\"Laptop\",\"price\":1299.99,\"quantity\":1}," +
                "{\"productId\":2002,\"productName\":\"Mouse\",\"price\":29.99,\"quantity\":2}" +
                "],\"totalAmount\":1359.97}";
    }

    /**
     * 模拟结算
     */
    public static String mockCheckout(long userId) {
        long orderId = System.currentTimeMillis() % 100000;
        return "{\"status\":\"success\",\"orderId\":" + orderId + 
               ",\"userId\":" + userId + ",\"totalAmount\":1359.97," +
               "\"message\":\"Order placed successfully\"}";
    }

    /**
     * 模拟获取订单列表
     */
    public static String mockGetOrderList(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"orders\":[" +
                "{\"orderId\":10001,\"date\":\"2024-01-15\",\"status\":\"delivered\",\"amount\":299.99}," +
                "{\"orderId\":10002,\"date\":\"2024-01-20\",\"status\":\"shipped\",\"amount\":1359.97}" +
                "]}";
    }

    /**
     * 模拟获取服务器状态
     */
    public static String mockGetServerStatus() {
        return "{\"status\":\"success\",\"serverStatus\":\"running\"," +
                "\"uptime\":\"2 days 5 hours\"," +
                "\"activeConnections\":3," +
                "\"memoryUsage\":\"45%\"," +
                "\"cpuUsage\":\"12%\"," +
                "\"version\":\"1.0.0-SNAPSHOT\"}";
    }

    /**
     * 模拟用户登出
     */
    public static String mockLogout(long userId) {
        return "{\"status\":\"success\",\"message\":\"User " + userId + " logged out successfully\"}";
    }

    /**
     * 模拟获取用户信息
     */
    public static String mockGetUserInfo(long userId) {
        if (userId == 1001) {
            return "{\"status\":\"success\",\"user\":{" +
                    "\"userId\":1001,\"username\":\"admin\",\"email\":\"admin@emshop.com\"," +
                    "\"phone\":\"13800138000\",\"role\":\"admin\",\"registerDate\":\"2024-01-01\"}}";
        } else {
            return "{\"status\":\"success\",\"user\":{" +
                    "\"userId\":" + userId + ",\"username\":\"user" + userId + "\"," +
                    "\"email\":\"user" + userId + "@emshop.com\",\"phone\":\"13800138001\"," +
                    "\"role\":\"customer\",\"registerDate\":\"2024-01-15\"}}";
        }
    }

    /**
     * 模拟删除商品
     */
    public static String mockDeleteProduct(long productId) {
        return "{\"status\":\"success\",\"message\":\"Product " + productId + " deleted successfully\"}";
    }

    /**
     * 模拟从购物车移除商品
     */
    public static String mockRemoveFromCart(long userId, long productId) {
        return "{\"status\":\"success\",\"message\":\"Product " + productId + 
               " removed from cart for user " + userId + "\"}";
    }

    /**
     * 模拟获取订单详情
     */
    public static String mockGetOrderDetail(long orderId) {
        return "{\"status\":\"success\",\"order\":{" +
                "\"orderId\":" + orderId + "," +
                "\"userId\":1002," +
                "\"orderDate\":\"2024-01-20\"," +
                "\"status\":\"shipped\"," +
                "\"shippingAddress\":\"123 Main St, City\"," +
                "\"items\":[" +
                "{\"productId\":2001,\"productName\":\"Laptop\",\"price\":1299.99,\"quantity\":1}," +
                "{\"productId\":2002,\"productName\":\"Mouse\",\"price\":29.99,\"quantity\":2}" +
                "]," +
                "\"totalAmount\":1359.97," +
                "\"trackingNumber\":\"TRK123456789\"" +
                "}}";
    }

    /**
     * 模拟取消订单
     */
    public static String mockCancelOrder(long userId, long orderId) {
        return "{\"status\":\"success\",\"message\":\"Order " + orderId + 
               " cancelled successfully for user " + userId + "\"}";
    }

    /**
     * 默认错误响应
     */
    public static String mockError(String operation) {
        return "{\"status\":\"error\",\"message\":\"Mock operation: " + operation + " not implemented\"}";
    }
}
