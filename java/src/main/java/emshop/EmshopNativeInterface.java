package emshop;

/**
 * JLU Emshop System - Native Interface
 * 包含所有需要在C++中实现的JNI接口声明
 * 
 * 编译生成头文件命令：
 * javac -h . EmshopNativeInterface.java
 * 或者使用：
 * javah -jni emshop.EmshopNativeInterface
 */
public class EmshopNativeInterface {

    // 加载本地库
    static {
        try {
            // 首先尝试加载新的面向对象JNI库
            System.loadLibrary("emshop_native_oop");
            System.out.println("Native library 'emshop_native_oop' loaded successfully.");
        } catch (UnsatisfiedLinkError e1) {
            try {
                // 备选方案：尝试加载原有库名
                System.loadLibrary("emshop");
                System.out.println("Native library 'emshop' loaded successfully.");
            } catch (UnsatisfiedLinkError e2) {
                System.err.println("Warning: Both native libraries not found:");
                System.err.println("  - emshop_native_oop: " + e1.getMessage());
                System.err.println("  - emshop: " + e2.getMessage());
                throw new RuntimeException("Failed to load any native library", e2);
            }
        }
    }

    // ==================== 系统初始化接口 ====================
    
    /**
     * 初始化系统服务
     * @return JSON格式的初始化结果
     */
    public static native String initializeService();
    
    /**
     * 获取初始化状态
     * @return JSON格式的初始化状态
     */
    public static native String getInitializationStatus();

    // ==================== 基础用户管理接口 ====================
    
    /**
     * 用户登录验证
     * @param username 用户名
     * @param password 密码
     * @return JSON格式的登录结果
     */
    public static native String login(String username, String password);
    
    /**
     * 用户注册
     * @param username 用户名
     * @param password 密码
     * @param phone 手机号
     * @return JSON格式的注册结果
     */
    public static native String register(String username, String password, String phone);
    
    /**
     * 用户登出
     * @param userId 用户ID
     * @return JSON格式的登出结果
     */
    public static native String logout(long userId);
    
    /**
     * 获取用户信息
     * @param userId 用户ID
     * @return JSON格式的用户信息
     */
    public static native String getUserInfo(long userId);
    
    /**
     * 更新用户信息
     * @param userId 用户ID
     * @param jsonInfo JSON格式的用户信息
     * @return JSON格式的更新结果
     */
    public static native String updateUserInfo(long userId, String jsonInfo);

    // ==================== 商品管理接口 ====================
    
    /**
     * 获取商品列表
     * @param category 商品分类
     * @param page 页码
     * @param pageSize 每页数量
     * @return JSON格式的商品列表
     */
    public static native String getProductList(String category, int page, int pageSize);
    
    /**
     * 获取商品详情
     * @param productId 商品ID
     * @return JSON格式的商品详情
     */
    public static native String getProductDetail(long productId);
    
    /**
     * 添加商品（管理员功能）
     * @param jsonProduct JSON格式的商品信息
     * @return JSON格式的添加结果
     */
    public static native String addProduct(String jsonProduct);
    
    /**
     * 更新商品信息
     * @param productId 商品ID
     * @param jsonProduct JSON格式的商品信息
     * @return JSON格式的更新结果
     */
    public static native String updateProduct(long productId, String jsonProduct);
    
    /**
     * 删除商品
     * @param productId 商品ID
     * @return JSON格式的删除结果
     */
    public static native String deleteProduct(long productId);
    
    /**
     * 获取商品分类列表
     * @return JSON格式的分类列表
     */
    public static native String getCategories();
    
    /**
     * 获取分类下的商品
     * @param category 分类名
     * @param page 页码
     * @param pageSize 每页数量
     * @param sortBy 排序方式
     * @return JSON格式的商品列表
     */
    public static native String getCategoryProducts(String category, int page, int pageSize, String sortBy);
    
    /**
     * 搜索商品
     * @param keyword 搜索关键词
     * @param page 页码
     * @param pageSize 每页数量
     * @param sortBy 排序方式
     * @param minPrice 最低价格
     * @param maxPrice 最高价格
     * @return JSON格式的搜索结果
     */
    public static native String searchProducts(String keyword, int page, int pageSize, String sortBy, double minPrice, double maxPrice);

    // ==================== 库存管理接口 ====================
    
    /**
     * 更新库存
     * @param productId 商品ID
     * @param quantity 数量
     * @param operation 操作类型（add/subtract/set）
     * @return JSON格式的更新结果
     */
    public static native String updateStock(long productId, int quantity, String operation);
    
    /**
     * 检查库存
     * @param productId 商品ID
     * @return JSON格式的库存信息
     */
    public static native String checkStock(long productId);
    
    /**
     * 获取低库存商品
     * @param threshold 库存阈值
     * @return JSON格式的低库存商品列表
     */
    public static native String getLowStockProducts(int threshold);

    // ==================== 购物车管理接口 ====================
    
    /**
     * 添加商品到购物车
     * @param userId 用户ID
     * @param productId 商品ID
     * @param quantity 数量
     * @return JSON格式的添加结果
     */
    public static native String addToCart(long userId, long productId, int quantity);
    
    /**
     * 获取购物车内容
     * @param userId 用户ID
     * @return JSON格式的购物车内容
     */
    public static native String getCart(long userId);
    
    /**
     * 从购物车移除商品
     * @param userId 用户ID
     * @param productId 商品ID
     * @return JSON格式的移除结果
     */
    public static native String removeFromCart(long userId, long productId);
    
    /**
     * 更新购物车商品数量
     * @param userId 用户ID
     * @param productId 商品ID
     * @param quantity 新数量
     * @return JSON格式的更新结果
     */
    public static native String updateCartItemQuantity(long userId, long productId, int quantity);
    
    /**
     * 清空购物车
     * @param userId 用户ID
     * @return JSON格式的清空结果
     */
    public static native String clearCart(long userId);
    
    /**
     * 获取购物车摘要
     * @param userId 用户ID
     * @return JSON格式的购物车摘要
     */
    public static native String getCartSummary(long userId);

    // ==================== 订单管理接口 ====================
    
    /**
     * 结算购物车
     * @param userId 用户ID
     * @return JSON格式的结算结果
     */
    public static native String checkout(long userId);
    
    /**
     * 获取订单列表
     * @param userId 用户ID
     * @return JSON格式的订单列表
     */
    public static native String getOrderList(long userId);
    
    /**
     * 获取订单详情
     * @param orderId 订单ID
     * @return JSON格式的订单详情
     */
    public static native String getOrderDetail(long orderId);
    
    /**
     * 取消订单
     * @param userId 用户ID
     * @param orderId 订单ID
     * @return JSON格式的取消结果
     */
    public static native String cancelOrder(long userId, long orderId);
    
    /**
     * 更新订单状态
     * @param orderId 订单ID
     * @param status 新状态
     * @return JSON格式的更新结果
     */
    public static native String updateOrderStatus(long orderId, String status);
    
    /**
     * 根据状态获取订单
     * @param userId 用户ID
     * @param status 订单状态
     * @return JSON格式的订单列表
     */
    public static native String getOrdersByStatus(long userId, String status);
    
    /**
     * 跟踪订单
     * @param orderId 订单ID
     * @return JSON格式的跟踪信息
     */
    public static native String trackOrder(long orderId);

    // ==================== 促销策略接口 ====================
    
    /**
     * 获取活跃的促销活动
     * @return JSON格式的促销活动列表
     */
    public static native String getActivePromotions();
    
    /**
     * 创建促销活动
     * @param jsonPromotion JSON格式的促销活动信息
     * @return JSON格式的创建结果
     */
    public static native String createPromotion(String jsonPromotion);
    
    /**
     * 计算折扣
     * @param userId 用户ID
     * @param productId 商品ID
     * @param promoCode 促销代码
     * @return JSON格式的折扣信息
     */
    public static native String calculateDiscount(long userId, long productId, String promoCode);
    
    /**
     * 应用优惠券
     * @param userId 用户ID
     * @param couponCode 优惠券代码
     * @return JSON格式的应用结果
     */
    public static native String applyCoupon(long userId, String couponCode);

    // ==================== 售后服务接口 ====================
    
    /**
     * 创建售后请求
     * @param userId 用户ID
     * @param orderId 订单ID
     * @param type 售后类型
     * @param reason 售后原因
     * @param jsonDetails JSON格式的详细信息
     * @return JSON格式的创建结果
     */
    public static native String createAfterSaleRequest(long userId, long orderId, String type, String reason, String jsonDetails);
    
    /**
     * 获取售后请求列表
     * @param userId 用户ID
     * @return JSON格式的售后请求列表
     */
    public static native String getAfterSaleRequests(long userId);
    
    /**
     * 处理售后请求
     * @param requestId 请求ID
     * @param action 处理动作
     * @param note 处理备注
     * @return JSON格式的处理结果
     */
    public static native String processAfterSaleRequest(long requestId, String action, String note);

    // ==================== UI主题系统接口 ====================
    
    /**
     * 获取可用主题
     * @return JSON格式的主题列表
     */
    public static native String getAvailableThemes();
    
    /**
     * 设置用户主题
     * @param userId 用户ID
     * @param themeName 主题名称
     * @return JSON格式的设置结果
     */
    public static native String setUserTheme(long userId, String themeName);
    
    /**
     * 获取用户主题
     * @param userId 用户ID
     * @return JSON格式的主题信息
     */
    public static native String getUserTheme(long userId);

    // ==================== 并发控制接口 ====================
    
    /**
     * 获取商品锁
     * @param productId 商品ID
     * @param userId 用户ID
     * @param quantity 数量
     * @return JSON格式的锁定结果
     */
    public static native String acquireProductLock(long productId, long userId, int quantity);
    
    /**
     * 释放商品锁
     * @param productId 商品ID
     * @param userId 用户ID
     * @return JSON格式的释放结果
     */
    public static native String releaseProductLock(long productId, long userId);
    
    /**
     * 获取商品锁状态
     * @param productId 商品ID
     * @return JSON格式的锁状态
     */
    public static native String getProductLockStatus(long productId);
    
    /**
     * 设置商品限量
     * @param productId 商品ID
     * @param limitQuantity 限量数量
     * @return JSON格式的设置结果
     */
    public static native String setProductLimitQuantity(long productId, int limitQuantity);

    // ==================== 数据分析接口 ====================
    
    /**
     * 获取销售统计
     * @param startDate 开始日期
     * @param endDate 结束日期
     * @return JSON格式的销售统计
     */
    public static native String getSalesStatistics(String startDate, String endDate);
    
    /**
     * 获取用户行为分析
     * @param userId 用户ID
     * @return JSON格式的行为分析
     */
    public static native String getUserBehaviorAnalysis(long userId);
    
    /**
     * 获取热销商品
     * @param topN 前N名
     * @return JSON格式的热销商品
     */
    public static native String getPopularProducts(int topN);

    // ==================== 用户权限接口 ====================
    
    /**
     * 获取用户角色
     * @param userId 用户ID
     * @return JSON格式的用户角色
     */
    public static native String getUserRoles(long userId);
    
    /**
     * 设置用户角色
     * @param userId 用户ID
     * @param role 角色名
     * @return JSON格式的设置结果
     */
    public static native String setUserRole(long userId, String role);
    
    /**
     * 检查用户权限
     * @param userId 用户ID
     * @param permission 权限名
     * @return JSON格式的权限检查结果
     */
    public static native String checkUserPermission(long userId, String permission);

    // ==================== 支付系统接口 ====================
    
    /**
     * 处理支付
     * @param orderId 订单ID
     * @param paymentMethod 支付方式
     * @param amount 支付金额
     * @param jsonPaymentDetails JSON格式的支付详情
     * @return JSON格式的支付结果
     */
    public static native String processPayment(long orderId, String paymentMethod, double amount, String jsonPaymentDetails);
    
    /**
     * 获取支付状态
     * @param orderId 订单ID
     * @return JSON格式的支付状态
     */
    public static native String getPaymentStatus(long orderId);
    
    /**
     * 退款处理
     * @param orderId 订单ID
     * @param amount 退款金额
     * @param reason 退款原因
     * @return JSON格式的退款结果
     */
    public static native String refundPayment(long orderId, double amount, String reason);

    // ==================== 系统监控接口 ====================
    
    /**
     * 获取系统状态
     * @return JSON格式的系统状态
     */
    public static native String getServerStatus();
    
    /**
     * 获取系统日志
     * @param logLevel 日志级别
     * @param page 页码
     * @param pageSize 每页数量
     * @return JSON格式的系统日志
     */
    public static native String getSystemLogs(String logLevel, int page, int pageSize);
    
    /**
     * 获取系统指标
     * @return JSON格式的系统指标
     */
    public static native String getSystemMetrics();
    
    /**
     * 获取活跃连接
     * @return JSON格式的活跃连接信息
     */
    public static native String getActiveConnections();

    // ==================== 数据库操作接口 ====================
    
    /**
     * 执行DML查询
     * @param sql SQL语句
     * @param jsonParameters JSON格式的参数
     * @return JSON格式的执行结果
     */
    public static native String executeDMLQuery(String sql, String jsonParameters);
    
    /**
     * 执行SELECT查询
     * @param sql SQL语句
     * @param jsonParameters JSON格式的参数
     * @return JSON格式的查询结果
     */
    public static native String executeSelectQuery(String sql, String jsonParameters);
    
    /**
     * 获取数据库模式
     * @return JSON格式的数据库模式
     */
    public static native String getDatabaseSchema();
    
    /**
     * 批量执行查询
     * @param jsonBatchQueries JSON格式的批量查询
     * @return JSON格式的批量执行结果
     */
    public static native String executeBatch(String jsonBatchQueries);

    // ==================== 缓存管理接口 ====================
    
    /**
     * 清理缓存
     * @param cacheType 缓存类型
     * @return JSON格式的清理结果
     */
    public static native String clearCache(String cacheType);
    
    /**
     * 获取缓存统计
     * @return JSON格式的缓存统计
     */
    public static native String getCacheStats();
}
