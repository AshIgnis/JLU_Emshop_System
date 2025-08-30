package emshop;
/**
 * 模拟数据提供者
 * 当C++本地库不可用时，提供模拟的业务数据
 * 用于开发和测试阶段
 */
public class MockDataProvider {
    
    /**
     * 模拟用户登录 - 新JNI接口方法
     */
    public static String login(String username, String password) {
        return mockLogin(username, password);
    }
    
    /**
     * 模拟用户注册 - 新JNI接口方法
     */
    public static String register(String username, String password, String phone) {
        return mockRegister(username, password, phone);
    }
    
    /**
     * 模拟用户登出 - 新JNI接口方法
     */
    public static String logout(long userId) {
        return "{\"success\":true,\"message\":\"用户登出成功\",\"userId\":" + userId + "}";
    }
    
    /**
     * 模拟获取用户信息 - 新JNI接口方法
     */
    public static String getUserInfo(long userId) {
        return "{\"success\":true,\"user\":{\"id\":" + userId + ",\"username\":\"testuser\",\"phone\":\"13800138000\",\"role\":\"user\"}}";
    }
    
    /**
     * 模拟更新用户信息 - 新JNI接口方法
     */
    public static String updateUserInfo(long userId, String jsonInfo) {
        return "{\"success\":true,\"message\":\"用户信息更新成功\",\"userId\":" + userId + "}";
    }
    
    /**
     * 模拟获取商品列表 - 新JNI接口方法
     */
    public static String getProductList(String category, int page, int pageSize) {
        return mockGetProductList(category, page, pageSize);
    }
    
    /**
     * 模拟获取商品详情 - 新JNI接口方法
     */
    public static String getProductDetail(long productId) {
        return mockGetProductDetail(productId);
    }
    
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

    // ==================== 扩展模拟数据方法 ====================
    
    // 🛍️ 商品分类管理
    public static String mockGetCategories() {
        return "{\"status\":\"success\",\"categories\":[" +
                "{\"id\":\"electronics\",\"name\":\"电子产品\",\"count\":156}," +
                "{\"id\":\"clothing\",\"name\":\"服装\",\"count\":423}," +
                "{\"id\":\"books\",\"name\":\"图书\",\"count\":789}," +
                "{\"id\":\"home\",\"name\":\"家居\",\"count\":234}," +
                "{\"id\":\"sports\",\"name\":\"运动\",\"count\":345}" +
                "]}";
    }
    
    public static String mockGetCategoryProducts(String category, int page, int pageSize, String sortBy) {
        return "{\"status\":\"success\",\"category\":\"" + category + "\"," +
                "\"page\":" + page + ",\"pageSize\":" + pageSize + ",\"sortBy\":\"" + sortBy + "\"," +
                "\"totalItems\":50,\"products\":[" +
                "{\"id\":1001,\"name\":\"" + category + " Product 1\",\"price\":299.99,\"stock\":20}," +
                "{\"id\":1002,\"name\":\"" + category + " Product 2\",\"price\":399.99,\"stock\":15}" +
                "]}";
    }
    
    // 🔍 商品搜索
    public static String mockSearchProducts(String keyword, int page, int pageSize, String sortBy, double minPrice, double maxPrice) {
        return "{\"status\":\"success\",\"keyword\":\"" + keyword + "\"," +
                "\"page\":" + page + ",\"pageSize\":" + pageSize + "," +
                "\"priceRange\":{\"min\":" + minPrice + ",\"max\":" + maxPrice + "}," +
                "\"totalItems\":25,\"products\":[" +
                "{\"id\":2001,\"name\":\"Search Result 1\",\"price\":199.99,\"stock\":30}," +
                "{\"id\":2002,\"name\":\"Search Result 2\",\"price\":249.99,\"stock\":25}" +
                "]}";
    }
    
    // 📦 库存管理
    public static String mockUpdateStock(long productId, int quantity, String operation) {
        return "{\"status\":\"success\",\"productId\":" + productId + 
                ",\"operation\":\"" + operation + "\",\"quantity\":" + quantity +
                ",\"newStock\":45,\"message\":\"Stock updated successfully\"}";
    }
    
    public static String mockCheckStock(long productId) {
        return "{\"status\":\"success\",\"productId\":" + productId + 
                ",\"currentStock\":28,\"reservedStock\":5,\"availableStock\":23}";
    }
    
    public static String mockGetLowStockProducts(int threshold) {
        return "{\"status\":\"success\",\"threshold\":" + threshold + ",\"products\":[" +
                "{\"id\":3001,\"name\":\"Low Stock Item 1\",\"currentStock\":3,\"minStock\":10}," +
                "{\"id\":3002,\"name\":\"Low Stock Item 2\",\"currentStock\":1,\"minStock\":5}" +
                "]}";
    }
    
    // 🎯 促销策略系统
    public static String mockGetActivePromotions() {
        return "{\"status\":\"success\",\"promotions\":[" +
                "{\"id\":1,\"name\":\"双11大促\",\"type\":\"percentage\",\"discount\":0.2,\"startDate\":\"2024-11-01\",\"endDate\":\"2024-11-11\"}," +
                "{\"id\":2,\"name\":\"新用户优惠券\",\"type\":\"coupon\",\"discount\":50.0,\"minAmount\":200.0}" +
                "]}";
    }
    
    public static String mockCreatePromotion(String jsonPromotion) {
        return "{\"status\":\"success\",\"promotionId\":123,\"message\":\"Promotion created successfully\"}";
    }
    
    public static String mockCalculateDiscount(long userId, long productId, String promoCode) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"productId\":" + productId + 
                ",\"promoCode\":\"" + promoCode + "\",\"originalPrice\":299.99," +
                "\"discountAmount\":59.99,\"finalPrice\":240.00}";
    }
    
    public static String mockApplyCoupon(long userId, String couponCode) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"couponCode\":\"" + couponCode + 
                "\",\"discountAmount\":30.0,\"message\":\"Coupon applied successfully\"}";
    }
    
    // 🛒 高级购物车功能
    public static String mockUpdateCartItemQuantity(long userId, long productId, int quantity) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"productId\":" + productId + 
                ",\"newQuantity\":" + quantity + ",\"totalItems\":3,\"totalAmount\":899.97}";
    }
    
    public static String mockClearCart(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"message\":\"Cart cleared successfully\"}";
    }
    
    public static String mockGetCartSummary(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"itemCount\":3,\"totalAmount\":899.97,\"discountAmount\":89.99," +
                "\"finalAmount\":809.98,\"estimatedTax\":64.80}";
    }
    
    // 📋 订单状态管理
    public static String mockUpdateOrderStatus(long orderId, String status) {
        return "{\"status\":\"success\",\"orderId\":" + orderId + 
                ",\"oldStatus\":\"processing\",\"newStatus\":\"" + status + 
                "\",\"updateTime\":\"2024-08-30T10:30:00Z\"}";
    }
    
    public static String mockGetOrdersByStatus(long userId, String status) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"orderStatus\":\"" + status + 
                "\",\"orders\":[" +
                "{\"orderId\":10001,\"amount\":299.99,\"createTime\":\"2024-08-25\",\"status\":\"" + status + "\"}," +
                "{\"orderId\":10002,\"amount\":199.99,\"createTime\":\"2024-08-28\",\"status\":\"" + status + "\"}" +
                "]}";
    }
    
    public static String mockTrackOrder(long orderId) {
        return "{\"status\":\"success\",\"orderId\":" + orderId + 
                ",\"currentStatus\":\"shipped\",\"trackingNumber\":\"SF123456789\"," +
                "\"timeline\":[" +
                "{\"status\":\"created\",\"time\":\"2024-08-25T10:00:00Z\",\"description\":\"订单创建\"}," +
                "{\"status\":\"paid\",\"time\":\"2024-08-25T10:15:00Z\",\"description\":\"支付成功\"}," +
                "{\"status\":\"shipped\",\"time\":\"2024-08-26T09:00:00Z\",\"description\":\"商品已发货\"}" +
                "]}";
    }
    
    // 🔧 售后服务系统
    public static String mockCreateAfterSaleRequest(long userId, long orderId, String type, String reason, String jsonDetails) {
        return "{\"status\":\"success\",\"requestId\":5001,\"userId\":" + userId + 
                ",\"orderId\":" + orderId + ",\"type\":\"" + type + "\",\"reason\":\"" + reason + 
                "\",\"createTime\":\"2024-08-30T10:30:00Z\",\"status\":\"pending\"}";
    }
    
    public static String mockGetAfterSaleRequests(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + ",\"requests\":[" +
                "{\"requestId\":5001,\"orderId\":10001,\"type\":\"refund\",\"status\":\"processing\",\"createTime\":\"2024-08-30\"}," +
                "{\"requestId\":5002,\"orderId\":10002,\"type\":\"exchange\",\"status\":\"completed\",\"createTime\":\"2024-08-28\"}" +
                "]}";
    }
    
    public static String mockProcessAfterSaleRequest(long requestId, String action, String note) {
        return "{\"status\":\"success\",\"requestId\":" + requestId + 
                ",\"action\":\"" + action + "\",\"note\":\"" + note + 
                "\",\"processTime\":\"2024-08-30T11:00:00Z\",\"newStatus\":\"approved\"}";
    }
    
    // 🎨 UI主题系统
    public static String mockGetAvailableThemes() {
        return "{\"status\":\"success\",\"themes\":[" +
                "{\"id\":\"default\",\"name\":\"默认主题\",\"preview\":\"default_preview.jpg\"}," +
                "{\"id\":\"dark\",\"name\":\"深色主题\",\"preview\":\"dark_preview.jpg\"}," +
                "{\"id\":\"blue\",\"name\":\"商务蓝\",\"preview\":\"blue_preview.jpg\"}," +
                "{\"id\":\"pink\",\"name\":\"粉色主题\",\"preview\":\"pink_preview.jpg\"}" +
                "]}";
    }
    
    public static String mockSetUserTheme(long userId, String themeName) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"oldTheme\":\"default\",\"newTheme\":\"" + themeName + 
                "\",\"message\":\"Theme updated successfully\"}";
    }
    
    public static String mockGetUserTheme(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"currentTheme\":\"dark\",\"themeSettings\":{\"fontSize\":14,\"language\":\"zh-CN\"}}";
    }
    
    // 🔒 并发控制系统
    public static String mockAcquireProductLock(long productId, long userId, int quantity) {
        return "{\"status\":\"success\",\"productId\":" + productId + ",\"userId\":" + userId + 
                ",\"quantity\":" + quantity + ",\"lockId\":\"LOCK_" + System.currentTimeMillis() + "\"" +
                ",\"expiresAt\":\"2024-08-30T11:00:00Z\",\"message\":\"Product locked successfully\"}";
    }
    
    public static String mockReleaseProductLock(long productId, long userId) {
        return "{\"status\":\"success\",\"productId\":" + productId + ",\"userId\":" + userId + 
                ",\"releaseTime\":\"2024-08-30T10:45:00Z\",\"message\":\"Product lock released\"}";
    }
    
    public static String mockGetProductLockStatus(long productId) {
        return "{\"status\":\"success\",\"productId\":" + productId + 
                ",\"isLocked\":true,\"lockedBy\":1001,\"lockCount\":3,\"expiresAt\":\"2024-08-30T11:00:00Z\"}";
    }
    
    public static String mockSetProductLimitQuantity(long productId, int limitQuantity) {
        return "{\"status\":\"success\",\"productId\":" + productId + 
                ",\"oldLimit\":10,\"newLimit\":" + limitQuantity + ",\"message\":\"Limit quantity updated\"}";
    }
    
    // 📊 数据分析和统计
    public static String mockGetSalesStatistics(String startDate, String endDate) {
        return "{\"status\":\"success\",\"period\":{\"start\":\"" + startDate + "\",\"end\":\"" + endDate + "\"}," +
                "\"totalSales\":125000.00,\"totalOrders\":342,\"averageOrderValue\":365.50," +
                "\"topProducts\":[{\"id\":1001,\"name\":\"热销商品1\",\"sales\":25000.00}]," +
                "\"dailyStats\":[{\"date\":\"2024-08-29\",\"sales\":5200.00,\"orders\":14}]}";
    }
    
    public static String mockGetUserBehaviorAnalysis(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"totalOrders\":15,\"totalSpent\":4500.00,\"avgOrderValue\":300.00," +
                "\"favoriteCategory\":\"electronics\",\"lastLoginTime\":\"2024-08-30T09:30:00Z\"," +
                "\"browsingHistory\":[{\"productId\":1001,\"viewTime\":\"2024-08-30T09:15:00Z\"}]}";
    }
    
    public static String mockGetPopularProducts(int topN) {
        return "{\"status\":\"success\",\"topN\":" + topN + ",\"products\":[" +
                "{\"rank\":1,\"productId\":1001,\"name\":\"热销产品1\",\"salesCount\":156,\"revenue\":46800.00}," +
                "{\"rank\":2,\"productId\":1002,\"name\":\"热销产品2\",\"salesCount\":134,\"revenue\":40200.00}" +
                "]}";
    }
    
    // 🔐 用户权限和角色管理
    public static String mockGetUserRoles(long userId) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"roles\":[\"customer\",\"vip\"],\"permissions\":[\"view_products\",\"place_orders\",\"access_vip_content\"]}";
    }
    
    public static String mockSetUserRole(long userId, String role) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"oldRoles\":[\"customer\"],\"newRoles\":[\"customer\",\"" + role + "\"],\"message\":\"Role added successfully\"}";
    }
    
    public static String mockCheckUserPermission(long userId, String permission) {
        return "{\"status\":\"success\",\"userId\":" + userId + 
                ",\"permission\":\"" + permission + "\",\"hasPermission\":true}";
    }
    
    // 💳 支付系统
    public static String mockProcessPayment(long orderId, String paymentMethod, double amount, String jsonPaymentDetails) {
        return "{\"status\":\"success\",\"orderId\":" + orderId + 
                ",\"paymentMethod\":\"" + paymentMethod + "\",\"amount\":" + amount + 
                ",\"transactionId\":\"TXN_" + System.currentTimeMillis() + "\"" +
                ",\"paymentStatus\":\"completed\",\"paymentTime\":\"2024-08-30T10:30:00Z\"}";
    }
    
    public static String mockGetPaymentStatus(long orderId) {
        return "{\"status\":\"success\",\"orderId\":" + orderId + 
                ",\"paymentStatus\":\"completed\",\"paymentMethod\":\"alipay\",\"amount\":299.99," +
                "\"transactionId\":\"TXN_123456789\",\"paymentTime\":\"2024-08-30T10:30:00Z\"}";
    }
    
    public static String mockRefundPayment(long orderId, double amount, String reason) {
        return "{\"status\":\"success\",\"orderId\":" + orderId + ",\"refundAmount\":" + amount + 
                ",\"reason\":\"" + reason + "\",\"refundId\":\"REF_" + System.currentTimeMillis() + "\"" +
                ",\"refundStatus\":\"processing\",\"estimatedTime\":\"3-5 business days\"}";
    }
    
    // 📝 系统日志和监控
    public static String mockGetSystemLogs(String logLevel, int page, int pageSize) {
        return "{\"status\":\"success\",\"logLevel\":\"" + logLevel + "\",\"page\":" + page + 
                ",\"pageSize\":" + pageSize + ",\"totalLogs\":1500,\"logs\":[" +
                "{\"timestamp\":\"2024-08-30T10:30:00Z\",\"level\":\"" + logLevel + "\",\"message\":\"User 1001 logged in\",\"source\":\"AuthService\"}," +
                "{\"timestamp\":\"2024-08-30T10:28:00Z\",\"level\":\"" + logLevel + "\",\"message\":\"Product 2001 updated\",\"source\":\"ProductService\"}" +
                "]}";
    }
    
    public static String mockGetSystemMetrics() {
        return "{\"status\":\"success\",\"timestamp\":\"2024-08-30T10:30:00Z\"," +
                "\"metrics\":{\"cpuUsage\":45.5,\"memoryUsage\":62.3,\"diskUsage\":78.9," +
                "\"activeConnections\":156,\"requestsPerSecond\":23.5,\"averageResponseTime\":125.6}}";
    }
    
    public static String mockGetActiveConnections() {
        return "{\"status\":\"success\",\"totalConnections\":156,\"connections\":[" +
                "{\"id\":\"conn_001\",\"userId\":1001,\"ip\":\"192.168.1.100\",\"connectedAt\":\"2024-08-30T09:30:00Z\"}," +
                "{\"id\":\"conn_002\",\"userId\":1002,\"ip\":\"192.168.1.101\",\"connectedAt\":\"2024-08-30T09:45:00Z\"}" +
                "]}";
    }
    
    // 🔄 数据库操作
    public static String mockExecuteDMLQuery(String sql, String jsonParameters) {
        return "{\"status\":\"success\",\"sql\":\"" + sql.substring(0, Math.min(sql.length(), 50)) + "...\"," +
                "\"affectedRows\":1,\"executionTime\":\"15ms\",\"message\":\"DML query executed successfully\"}";
    }
    
    public static String mockExecuteSelectQuery(String sql, String jsonParameters) {
        return "{\"status\":\"success\",\"sql\":\"" + sql.substring(0, Math.min(sql.length(), 50)) + "...\"," +
                "\"resultCount\":5,\"executionTime\":\"8ms\",\"columns\":[\"id\",\"name\",\"price\"]," +
                "\"data\":[[1001,\"Sample Product\",299.99],[1002,\"Another Product\",199.99]]}";
    }
    
    public static String mockGetDatabaseSchema() {
        return "{\"status\":\"success\",\"schema\":{" +
                "\"tables\":[" +
                "{\"name\":\"users\",\"columns\":[\"user_id\",\"username\",\"password\",\"email\",\"phone\",\"create_time\"]}," +
                "{\"name\":\"products\",\"columns\":[\"product_id\",\"name\",\"description\",\"price\",\"stock\",\"category\"]}," +
                "{\"name\":\"orders\",\"columns\":[\"order_id\",\"user_id\",\"total_amount\",\"status\",\"create_time\"]}" +
                "]}}";
    }
    
    public static String mockExecuteBatch(String jsonBatchQueries) {
        return "{\"status\":\"success\",\"batchSize\":5,\"successCount\":5,\"failureCount\":0," +
                "\"totalExecutionTime\":\"45ms\",\"message\":\"Batch queries executed successfully\"}";
    }
    
    // 🔄 缓存管理
    public static String mockClearCache(String cacheType) {
        return "{\"status\":\"success\",\"cacheType\":\"" + cacheType + 
                "\",\"clearedEntries\":156,\"message\":\"Cache cleared successfully\"}";
    }
    
    public static String mockGetCacheStats() {
        return "{\"status\":\"success\",\"caches\":{" +
                "\"product\":{\"size\":1200,\"hitRate\":0.85,\"missCount\":180}," +
                "\"user\":{\"size\":5000,\"hitRate\":0.92,\"missCount\":400}," +
                "\"session\":{\"size\":300,\"hitRate\":0.78,\"missCount\":85}" +
                "}}";
    }
}
