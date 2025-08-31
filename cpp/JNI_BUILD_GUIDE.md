# JLU Emshop System - Windows JNI编译指南

## 1. 环境准备

### Windows 环境要求
- MinGW-w64 或 MSYS2
- MySQL C++ Connector
- JsonCpp 库
- JDK 21

### 依赖安装
```powershell
# 使用 vcpkg 安装依赖
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install jsoncpp:x64-windows
.\vcpkg install libmysql:x64-windows
```

## 2. 编译命令

### Windows (PowerShell)
```powershell
# 安装依赖
# 1. 下载并安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install jsoncpp:x64-windows
.\vcpkg install libmysql:x64-windows

# 2. 编译JNI库
g++ -shared -fPIC ^
    -I"C:\Program Files\Java\jdk-21\include" ^
    -I"C:\Program Files\Java\jdk-21\include\win32" ^
    -I"D:\MySQL\include" ^
    -I"D:\vcpkg\installed\x64-windows\include" ^
    -L"D:\MySQL\lib" ^
    -L"D:\vcpkg\installed\x64-windows\lib" ^
    -ljsoncpp -lmysqlclient ^
    -o emshop.dll ^
    emshop_native_impl.cpp
```

## 3. 数据库初始化SQL

```sql
-- 创建数据库
CREATE DATABASE IF NOT EXISTS emshop_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE emshop_db;

-- 创建数据库用户（如果需要）
-- CREATE USER 'emshop_user'@'127.0.0.1' IDENTIFIED BY 'emshop_password';
-- GRANT ALL PRIVILEGES ON emshop_db.* TO 'emshop_user'@'127.0.0.1';
-- FLUSH PRIVILEGES;

-- 确保MySQL服务运行在 127.0.0.1:3306
-- 测试连接: mysql -h 127.0.0.1 -P 3306 -u root -p

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(32) NOT NULL, -- MD5哈希
    phone VARCHAR(20),
    role VARCHAR(20) DEFAULT 'user', -- 'user', 'admin'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 商品表
CREATE TABLE IF NOT EXISTS products (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    stock_quantity INT DEFAULT 0,
    category VARCHAR(50),
    image_url VARCHAR(200),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 购物车表
CREATE TABLE IF NOT EXISTS cart (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    quantity INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_product (user_id, product_id)
);

-- 订单表
CREATE TABLE IF NOT EXISTS orders (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    total_amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending', -- 'pending', 'paid', 'shipped', 'completed', 'cancelled'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 订单详情表
CREATE TABLE IF NOT EXISTS order_items (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    quantity INT NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE
);

-- 促销活动表
CREATE TABLE IF NOT EXISTS promotions (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    discount_type VARCHAR(20), -- 'percentage', 'fixed_amount'
    discount_value DECIMAL(5,2) NOT NULL,
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 商品锁表（用于并发控制）
CREATE TABLE IF NOT EXISTS product_locks (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    product_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    locked_quantity INT NOT NULL,
    lock_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_product_user (product_id, user_id),
    INDEX idx_expires (expires_at)
);

-- 售后请求表
CREATE TABLE IF NOT EXISTS after_sale_requests (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    order_id BIGINT NOT NULL,
    type VARCHAR(20), -- 'refund', 'exchange', 'repair'
    reason TEXT,
    status VARCHAR(20) DEFAULT 'pending', -- 'pending', 'approved', 'rejected', 'completed'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE
);

-- 插入测试数据
INSERT INTO users (username, password, phone, role) VALUES 
('admin', MD5('admin123'), '13800138000', 'admin'),
('testuser', MD5('123456'), '13800138001', 'user');

INSERT INTO products (name, description, price, stock_quantity, category) VALUES 
('笔记本电脑', '高性能笔记本电脑', 5999.00, 50, '电子产品'),
('智能手机', '最新款智能手机', 2999.00, 100, '电子产品'),
('运动鞋', '舒适运动鞋', 299.00, 200, '服装鞋帽'),
('咖啡杯', '陶瓷咖啡杯', 39.90, 500, '生活用品');
```

## 4. Java使用示例

```java
// 更新EmshopServer.java中的dispatch方法以使用新的JNI接口

private static String dispatch(String command) {
    try {
        String[] parts = command.split(" ", 2);
        String action = parts[0];
        String params = parts.length > 1 ? parts[1] : "";

        switch (action.toLowerCase()) {
            case "login":
                String[] loginParams = params.split(" ");
                if (loginParams.length >= 2) {
                    return EmshopNativeInterface.login(loginParams[0], loginParams[1]);
                }
                return "{\"error\": \"Missing login parameters\"}";

            case "register":
                String[] regParams = params.split(" ");
                if (regParams.length >= 3) {
                    return EmshopNativeInterface.register(regParams[0], regParams[1], regParams[2]);
                }
                return "{\"error\": \"Missing register parameters\"}";

            case "products":
                return EmshopNativeInterface.getProductList("", 1, 10);

            case "product":
                try {
                    long productId = Long.parseLong(params);
                    return EmshopNativeInterface.getProductDetail(productId);
                } catch (NumberFormatException e) {
                    return "{\"error\": \"Invalid product ID\"}";
                }

            default:
                return MockDataProvider.getMockResponse(action, params);
        }
    } catch (Exception e) {
        return "{\"error\": \"" + e.getMessage() + "\"}";
    }
}
```

## 5. 部署和测试

1. 确保MySQL服务运行并执行初始化SQL
2. 编译生成JNI库文件
3. 将库文件放在Java程序的库路径中（Windows: emshop.dll, Linux: libemshop.so）
4. 运行Java服务器：`mvn exec:java@server -Dexec.args="8090"`
5. 运行Java客户端：`mvn exec:java@client -Dexec.args="localhost 8090"`

## 6. 故障排除

- 如果遇到"UnsatisfiedLinkError"，检查JNI库是否在正确路径
- 如果数据库连接失败，检查MySQL服务状态和连接参数
- 编译错误通常是因为缺少依赖库或头文件路径不正确

## 7. 性能优化建议

- 使用连接池管理数据库连接
- 实现查询结果缓存
- 对高频访问的接口添加内存缓存
- 使用预编译SQL语句防止SQL注入
- 实现异步处理长时间运行的操作
