# JLU Emshop System - C++ Interface Reference

## 概述

本文档详细描述了需要在C++中实现的JNI接口，这些接口将被Java端的`EmshopServer`类调用。所有接口都返回JSON格式的字符串。

## 📋 基础数据结构

### JSON响应格式标准
```json
{
    "status": "success|error",
    "message": "操作描述信息",
    "data": {...},  // 具体数据
    "timestamp": "2024-08-30T10:30:00Z"
}
```

### 数据库表结构
```sql
-- 用户表
CREATE TABLE users (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    email VARCHAR(100),
    phone VARCHAR(20),
    role VARCHAR(20) DEFAULT 'customer',
    theme VARCHAR(20) DEFAULT 'default',
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 商品表
CREATE TABLE products (
    product_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(200) NOT NULL,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    stock INTEGER DEFAULT 0,
    reserved_stock INTEGER DEFAULT 0,
    category VARCHAR(50),
    image_url VARCHAR(500),
    is_limited BOOLEAN DEFAULT FALSE,
    limit_quantity INTEGER DEFAULT 0,
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 订单表
CREATE TABLE orders (
    order_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    total_amount DECIMAL(10,2) NOT NULL,
    discount_amount DECIMAL(10,2) DEFAULT 0,
    final_amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    payment_status VARCHAR(20) DEFAULT 'unpaid',
    payment_method VARCHAR(50),
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 购物车表
CREATE TABLE cart (
    cart_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    quantity INTEGER NOT NULL,
    add_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (product_id) REFERENCES products(product_id)
);

-- 促销表
CREATE TABLE promotions (
    promotion_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    type VARCHAR(20) NOT NULL, -- 'percentage', 'fixed', 'coupon'
    discount_value DECIMAL(10,2) NOT NULL,
    min_amount DECIMAL(10,2) DEFAULT 0,
    start_date DATETIME NOT NULL,
    end_date DATETIME NOT NULL,
    is_active BOOLEAN DEFAULT TRUE
);

-- 产品锁表 (并发控制)
CREATE TABLE product_locks (
    lock_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    product_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    quantity INTEGER NOT NULL,
    lock_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL,
    FOREIGN KEY (product_id) REFERENCES products(product_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 售后请求表
CREATE TABLE after_sale_requests (
    request_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    order_id BIGINT NOT NULL,
    type VARCHAR(20) NOT NULL, -- 'refund', 'exchange', 'repair'
    reason TEXT NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (order_id) REFERENCES orders(order_id)
);
```

## 🔧 核心接口实现指南

### 1. 用户管理接口

#### `nativeLogin(String username, String password)`
```cpp
JNIEXPORT jstring JNICALL Java_emshop_EmshopServer_nativeLogin
  (JNIEnv *env, jclass clazz, jstring username, jstring password) {
    
    // 1. 获取C++字符串
    const char* user = env->GetStringUTFChars(username, NULL);
    const char* pass = env->GetStringUTFChars(password, NULL);
    
    // 2. 数据库查询验证
    // SELECT user_id, username, role FROM users WHERE username = ? AND password = MD5(?)
    
    // 3. 构造JSON响应
    std::string result = R"({
        "status": "success",
        "userId": 1001,
        "username": ")" + std::string(user) + R"(",
        "role": "customer",
        "loginTime": "2024-08-30T10:30:00Z"
    })";
    
    // 4. 释放资源并返回
    env->ReleaseStringUTFChars(username, user);
    env->ReleaseStringUTFChars(password, pass);
    return env->NewStringUTF(result.c_str());
}
```

#### `nativeRegister(String username, String password, String phone)`
- 验证用户名唯一性
- 密码加密存储（MD5或SHA256）
- 手机号格式验证
- 创建默认用户配置

### 2. 商品管理接口

#### `nativeGetProductList(String category, int page, int pageSize)`
```cpp
// SQL: SELECT * FROM products WHERE category = ? LIMIT ? OFFSET ?
// 实现分页查询，返回商品列表JSON
```

#### `nativeGetProductDetail(long productId)`
```cpp
// SQL: SELECT * FROM products WHERE product_id = ?
// 返回详细的商品信息，包括库存、图片、描述等
```

### 3. 高级功能接口

#### `nativeAcquireProductLock(long productId, long userId, int quantity)`
```cpp
JNIEXPORT jstring JNICALL Java_emshop_EmshopServer_nativeAcquireProductLock
  (JNIEnv *env, jclass clazz, jlong productId, jlong userId, jint quantity) {
    
    // 1. 检查商品库存
    // 2. 检查是否为限量商品
    // 3. 创建产品锁记录
    // 4. 更新reserved_stock
    // 5. 设置锁过期时间（如10分钟）
    
    // 伪代码:
    /*
    BEGIN TRANSACTION;
    
    SELECT stock, reserved_stock, is_limited, limit_quantity 
    FROM products WHERE product_id = ?;
    
    if (available_stock >= quantity) {
        INSERT INTO product_locks (product_id, user_id, quantity, expires_at)
        VALUES (?, ?, ?, NOW() + INTERVAL 10 MINUTE);
        
        UPDATE products SET reserved_stock = reserved_stock + quantity
        WHERE product_id = ?;
        
        COMMIT;
        return success;
    } else {
        ROLLBACK;
        return error;
    }
    */
}
```

#### `nativeGetActivePromotions()`
```cpp
// SQL: SELECT * FROM promotions WHERE is_active = TRUE AND start_date <= NOW() AND end_date >= NOW()
// 返回当前有效的促销活动列表
```

#### `nativeCalculateDiscount(long userId, long productId, String promoCode)`
```cpp
// 1. 获取用户信息和商品价格
// 2. 查找适用的促销规则
// 3. 实现促销策略模式
// 4. 计算最终折扣金额
```

### 4. 并发控制实现

#### 产品锁机制
```cpp
class ProductLockManager {
private:
    std::mutex lock_mutex;
    std::unordered_map<long, std::vector<ProductLock>> active_locks;
    
public:
    bool acquireLock(long productId, long userId, int quantity);
    bool releaseLock(long productId, long userId);
    void cleanExpiredLocks();
    LockStatus getLockStatus(long productId);
};
```

#### 数据库连接池
```cpp
class DatabasePool {
private:
    std::queue<MYSQL*> available_connections;
    std::mutex pool_mutex;
    const int max_connections = 20;
    
public:
    MYSQL* getConnection();
    void releaseConnection(MYSQL* conn);
    bool executeQuery(const std::string& sql, const std::vector<std::string>& params);
};
```

### 5. 支付系统接口

#### `nativeProcessPayment(long orderId, String paymentMethod, double amount, String jsonPaymentDetails)`
```cpp
// 1. 验证订单状态
// 2. 调用第三方支付接口（支付宝/微信/银行卡）
// 3. 更新订单支付状态
// 4. 释放商品锁定
// 5. 减少实际库存
// 6. 记录支付流水
```

### 6. 数据分析接口

#### `nativeGetSalesStatistics(String startDate, String endDate)`
```cpp
// 复杂SQL查询，包含：
// - 总销售额统计
// - 订单数量统计
// - 热销商品排行
// - 用户购买行为分析
// - 每日/每月销售趋势
```

## 🚀 性能优化建议

### 1. 数据库优化
- 为常用查询字段创建索引
- 使用连接池避免频繁连接创建
- 实现读写分离
- 使用Redis缓存热点数据

### 2. 并发处理
```cpp
// 线程池实现
class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
    
public:
    ThreadPool(size_t threads);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args);
    ~ThreadPool();
};
```

### 3. 内存管理
- 使用智能指针管理资源
- 实现对象池减少内存分配
- 定期清理过期的锁和缓存

## 🔐 安全性考虑

### 1. SQL注入防护
```cpp
// 使用预编译语句
MYSQL_STMT* stmt = mysql_stmt_init(conn);
const char* query = "SELECT * FROM users WHERE username = ? AND password = ?";
mysql_stmt_prepare(stmt, query, strlen(query));
```

### 2. 数据加密
- 用户密码使用MD5+盐值加密
- 敏感数据传输加密
- 支付信息加密存储

### 3. 权限控制
- 实现基于角色的访问控制(RBAC)
- 接口调用频率限制
- 操作日志记录

## 📝 日志系统

### 实现建议
```cpp
class Logger {
public:
    enum LogLevel { DEBUG, INFO, WARN, ERROR };
    
    static void log(LogLevel level, const std::string& message);
    static void logTransaction(long userId, const std::string& action, const std::string& details);
    static void logError(const std::string& function, const std::string& error);
};
```

## 🔄 部署和测试

### 1. 编译配置
```cmake
# CMakeLists.txt
find_package(JNI REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

target_link_libraries(emshop_jni ${JNI_LIBRARIES} ${MYSQL_LIBRARIES})
```

### 2. JNI库加载
确保生成的动态库（Windows: emshop.dll, Linux: libemshop.so）能被Java找到。

### 3. 单元测试
为每个核心功能编写C++单元测试，确保接口正确性。

## 📚 实现优先级

### 阶段1：基础功能 (第1-2周)
1. 用户登录注册
2. 基础商品查询
3. 购物车基本操作
4. 简单订单处理

### 阶段2：核心功能 (第3-4周)
1. 促销策略系统
2. 支付处理
3. 库存管理
4. 订单状态跟踪

### 阶段3：高级功能 (第5-6周)
1. 并发控制和产品锁
2. 售后服务系统
3. 数据分析统计
4. 性能优化

### 阶段4：完善优化 (第7-8周)
1. 安全性加固
2. 错误处理完善
3. 日志系统
4. 压力测试和优化

## 📞 技术支持

如需要具体的实现示例或遇到技术问题，可以：
1. 参考Java端的MockDataProvider了解期望的返回格式
2. 查看readme.md中的详细需求说明
3. 运行Netty服务器测试接口调用流程

祝你实现顺利！🚀
