-- JLU Emshop System 数据库初始化脚本
-- 连接信息: 127.0.0.1:3306
-- 数据库名: emshop_db

-- JLU Emshop System 数据库初始化脚本
-- 数据库: emshop
-- 服务器: 127.0.0.1:3306

-- 创建数据库
CREATE DATABASE IF NOT EXISTS emshop CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE emshop;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
    password VARCHAR(255) NOT NULL COMMENT '密码(加密后)',
    phone VARCHAR(20) COMMENT '手机号',
    email VARCHAR(100) COMMENT '邮箱',
    role VARCHAR(20) DEFAULT 'user' COMMENT '角色: user, admin, vip',
    status VARCHAR(20) DEFAULT 'active' COMMENT '状态: active, inactive, banned',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_username (username),
    INDEX idx_phone (phone)
) ENGINE=InnoDB COMMENT='用户表';

-- 商品分类表
CREATE TABLE IF NOT EXISTS categories (
    category_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL COMMENT '分类名称',
    description TEXT COMMENT '分类描述',
    parent_id BIGINT DEFAULT 0 COMMENT '父分类ID，0表示顶级分类',
    sort_order INT DEFAULT 0 COMMENT '排序',
    status VARCHAR(20) DEFAULT 'active' COMMENT '状态',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_parent (parent_id)
) ENGINE=InnoDB COMMENT='商品分类表';

-- 商品表
CREATE TABLE IF NOT EXISTS products (
    product_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(200) NOT NULL COMMENT '商品名称',
    description TEXT COMMENT '商品描述',
    category VARCHAR(50) NOT NULL COMMENT '商品分类',
    price DECIMAL(10,2) NOT NULL COMMENT '价格',
    stock INT NOT NULL DEFAULT 0 COMMENT '库存数量',
    status VARCHAR(20) DEFAULT 'active' COMMENT '状态: active, inactive, deleted',
    images TEXT COMMENT 'JSON格式的图片数组',
    specifications TEXT COMMENT 'JSON格式的规格参数',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_category (category),
    INDEX idx_price (price),
    INDEX idx_status (status),
    INDEX idx_name (name)
) ENGINE=InnoDB COMMENT='商品表';

-- 购物车表
CREATE TABLE IF NOT EXISTS cart (
    cart_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL COMMENT '用户ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    quantity INT NOT NULL DEFAULT 1 COMMENT '数量',
    add_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
    UNIQUE KEY uk_user_product (user_id, product_id),
    INDEX idx_user (user_id),
    INDEX idx_product (product_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='购物车表';

-- 订单表
CREATE TABLE IF NOT EXISTS orders (
    order_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL COMMENT '用户ID',
    total_amount DECIMAL(10,2) NOT NULL COMMENT '订单总金额',
    status VARCHAR(20) DEFAULT 'pending' COMMENT '订单状态: pending, paid, shipping, delivered, cancelled, refunded',
    payment_method VARCHAR(20) COMMENT '支付方式',
    shipping_address TEXT COMMENT '收货地址',
    remark TEXT COMMENT '订单备注',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_user (user_id),
    INDEX idx_status (status),
    INDEX idx_create_time (create_time),
    FOREIGN KEY (user_id) REFERENCES users(user_id)
) ENGINE=InnoDB COMMENT='订单表';

-- 订单明细表
CREATE TABLE IF NOT EXISTS order_items (
    item_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id BIGINT NOT NULL COMMENT '订单ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    product_name VARCHAR(200) NOT NULL COMMENT '商品名称',
    price DECIMAL(10,2) NOT NULL COMMENT '商品价格',
    quantity INT NOT NULL COMMENT '购买数量',
    subtotal DECIMAL(10,2) NOT NULL COMMENT '小计',
    INDEX idx_order (order_id),
    INDEX idx_product (product_id),
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id)
) ENGINE=InnoDB COMMENT='订单明细表';

-- 促销活动表
CREATE TABLE IF NOT EXISTS promotions (
    promotion_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL COMMENT '促销名称',
    type VARCHAR(20) NOT NULL COMMENT '促销类型: discount, coupon, flash_sale',
    discount_rate DECIMAL(5,2) COMMENT '折扣率',
    discount_amount DECIMAL(10,2) COMMENT '折扣金额',
    min_amount DECIMAL(10,2) COMMENT '最低消费金额',
    start_time TIMESTAMP COMMENT '开始时间',
    end_time TIMESTAMP COMMENT '结束时间',
    status VARCHAR(20) DEFAULT 'active' COMMENT '状态',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB COMMENT='促销活动表';

-- 用户会话表（可选，用于记录登录状态）
CREATE TABLE IF NOT EXISTS user_sessions (
    session_id VARCHAR(255) PRIMARY KEY,
    user_id BIGINT NOT NULL,
    token VARCHAR(255) NOT NULL,
    expire_time TIMESTAMP NOT NULL,
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user (user_id),
    INDEX idx_expire (expire_time),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户会话表';

-- 插入一些测试数据
INSERT IGNORE INTO users (username, password, phone, email, role) VALUES 
('admin', 'admin_encrypted', '13800138000', 'admin@emshop.com', 'admin'),
('testuser', 'test_encrypted', '13800138001', 'test@emshop.com', 'user'),
('vipuser', 'vip_encrypted', '13800138002', 'vip@emshop.com', 'vip');

INSERT IGNORE INTO categories (name, description) VALUES 
('电子产品', '手机、电脑、数码产品等'),
('服装', '男装、女装、童装等'),
('图书', '各类书籍、教材、文学作品等'),
('食品', '零食、饮料、生鲜等'),
('家居', '家具、装饰、日用品等');

INSERT IGNORE INTO products (name, description, category, price, stock) VALUES 
('iPhone 15', '苹果最新款智能手机', '电子产品', 7999.00, 50),
('MacBook Air', '苹果笔记本电脑', '电子产品', 8999.00, 30),
('Nike运动鞋', '舒适运动鞋', '服装', 599.00, 100),
('Java编程思想', '经典编程书籍', '图书', 89.00, 200),
('星巴克咖啡', '精选咖啡豆', '食品', 89.00, 500),
('宜家书桌', '简约风格书桌', '家居', 299.00, 20),
('小米手机', '性价比智能手机', '电子产品', 1999.00, 80),
('优衣库T恤', '纯棉舒适T恤', '服装', 99.00, 150),
('算法导论', '计算机算法经典教材', '图书', 128.00, 60),
('三只松鼠坚果', '精选坚果礼盒', '食品', 58.00, 300);

-- 创建数据库用户（根据需要调整）
-- CREATE USER 'emshop_user'@'127.0.0.1' IDENTIFIED BY 'emshop_password';
-- GRANT ALL PRIVILEGES ON emshop.* TO 'emshop_user'@'127.0.0.1';
-- FLUSH PRIVILEGES;

-- 显示创建结果
SHOW TABLES;
SELECT 'Database initialization completed!' as status;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(32) NOT NULL COMMENT 'MD5哈希值',
    phone VARCHAR(20),
    email VARCHAR(100),
    role VARCHAR(20) DEFAULT 'user' COMMENT 'user, admin',
    status VARCHAR(20) DEFAULT 'active' COMMENT 'active, inactive, banned',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 商品表
CREATE TABLE IF NOT EXISTS products (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    price DECIMAL(10,2) NOT NULL,
    original_price DECIMAL(10,2),
    stock_quantity INT DEFAULT 0,
    category VARCHAR(50),
    brand VARCHAR(50),
    image_url VARCHAR(500),
    status VARCHAR(20) DEFAULT 'active' COMMENT 'active, inactive, discontinued',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_category (category),
    INDEX idx_status (status),
    INDEX idx_price (price)
);

-- 商品分类表
CREATE TABLE IF NOT EXISTS categories (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) UNIQUE NOT NULL,
    parent_id BIGINT,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (parent_id) REFERENCES categories(id) ON DELETE SET NULL
);

-- 购物车表
CREATE TABLE IF NOT EXISTS cart (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    quantity INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE,
    UNIQUE KEY unique_user_product (user_id, product_id)
);

-- 订单表
CREATE TABLE IF NOT EXISTS orders (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    total_amount DECIMAL(10,2) NOT NULL,
    discount_amount DECIMAL(10,2) DEFAULT 0.00,
    final_amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending' COMMENT 'pending, paid, shipped, delivered, cancelled, refunded',
    payment_method VARCHAR(20),
    payment_status VARCHAR(20) DEFAULT 'unpaid' COMMENT 'unpaid, paid, refunded',
    shipping_address TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
);

-- 订单详情表
CREATE TABLE IF NOT EXISTS order_items (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id BIGINT NOT NULL,
    product_id BIGINT NOT NULL,
    product_name VARCHAR(100) NOT NULL,
    quantity INT NOT NULL,
    price DECIMAL(10,2) NOT NULL,
    total_price DECIMAL(10,2) NOT NULL,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE
);

-- 促销活动表
CREATE TABLE IF NOT EXISTS promotions (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    type VARCHAR(20) NOT NULL COMMENT 'percentage, fixed_amount, buy_one_get_one',
    discount_value DECIMAL(8,2) NOT NULL,
    min_amount DECIMAL(10,2),
    max_discount DECIMAL(10,2),
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_active_dates (is_active, start_date, end_date)
);

-- 优惠券表
CREATE TABLE IF NOT EXISTS coupons (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    code VARCHAR(20) UNIQUE NOT NULL,
    name VARCHAR(100) NOT NULL,
    type VARCHAR(20) NOT NULL COMMENT 'percentage, fixed_amount',
    value DECIMAL(8,2) NOT NULL,
    min_amount DECIMAL(10,2),
    max_uses INT DEFAULT 1,
    used_count INT DEFAULT 0,
    start_date TIMESTAMP,
    end_date TIMESTAMP,
    is_active BOOLEAN DEFAULT true,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_code (code),
    INDEX idx_active (is_active)
);

-- 用户优惠券使用记录表
CREATE TABLE IF NOT EXISTS user_coupon_usage (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    coupon_id BIGINT NOT NULL,
    order_id BIGINT,
    used_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (coupon_id) REFERENCES coupons(id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE SET NULL
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
    type VARCHAR(20) NOT NULL COMMENT 'refund, exchange, repair',
    reason VARCHAR(200),
    description TEXT,
    status VARCHAR(20) DEFAULT 'pending' COMMENT 'pending, approved, rejected, completed',
    admin_note TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id),
    INDEX idx_status (status)
);

-- 支付记录表
CREATE TABLE IF NOT EXISTS payments (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id BIGINT NOT NULL,
    payment_method VARCHAR(20) NOT NULL COMMENT 'alipay, wechat, credit_card',
    amount DECIMAL(10,2) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending' COMMENT 'pending, success, failed, cancelled',
    transaction_id VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE,
    INDEX idx_order_id (order_id),
    INDEX idx_transaction_id (transaction_id)
);

-- 系统日志表
CREATE TABLE IF NOT EXISTS system_logs (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    level VARCHAR(10) NOT NULL COMMENT 'INFO, WARN, ERROR, DEBUG',
    message TEXT NOT NULL,
    module VARCHAR(50),
    user_id BIGINT,
    ip_address VARCHAR(45),
    user_agent TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_level (level),
    INDEX idx_created_at (created_at),
    INDEX idx_user_id (user_id)
);

-- 插入测试数据
INSERT INTO categories (name, description) VALUES 
('电子产品', '各类电子产品'),
('服装鞋帽', '服装和鞋帽用品'),
('生活用品', '日常生活用品'),
('图书文具', '图书和文具用品'),
('运动健康', '运动和健康用品');

INSERT INTO users (username, password, phone, email, role) VALUES 
('admin', MD5('admin123'), '13800138000', 'admin@emshop.com', 'admin'),
('testuser', MD5('123456'), '13800138001', 'user@emshop.com', 'user'),
('customer1', MD5('password'), '13800138002', 'customer1@emshop.com', 'user'),
('customer2', MD5('password'), '13800138003', 'customer2@emshop.com', 'user');

INSERT INTO products (name, description, price, original_price, stock_quantity, category, brand) VALUES 
('华为Mate50 Pro', '华为旗舰手机', 5999.00, 6999.00, 50, '电子产品', '华为'),
('小米13 Ultra', '小米影像旗舰', 4999.00, 5499.00, 30, '电子产品', '小米'),
('苹果iPhone14', '苹果最新款手机', 6999.00, 7999.00, 25, '电子产品', '苹果'),
('戴尔XPS13', '轻薄本笔记本电脑', 8999.00, 9999.00, 15, '电子产品', '戴尔'),
('Nike Air Max', 'Nike经典运动鞋', 899.00, 1099.00, 100, '服装鞋帽', 'Nike'),
('Adidas Stan Smith', 'Adidas经典小白鞋', 599.00, 699.00, 80, '服装鞋帽', 'Adidas'),
('无印良品水杯', '简约设计保温杯', 199.00, 249.00, 200, '生活用品', '无印良品'),
('小米电饭煲', '智能电饭煲', 399.00, 499.00, 60, '生活用品', '小米'),
('Java程序设计', 'Java编程入门教程', 89.00, 99.00, 150, '图书文具', '清华大学出版社'),
('数据结构与算法', '计算机科学经典教材', 79.00, 89.00, 120, '图书文具', '机械工业出版社');

INSERT INTO promotions (name, description, type, discount_value, min_amount, start_date, end_date, is_active) VALUES 
('新用户专享', '新用户注册立减100元', 'fixed_amount', 100.00, 500.00, '2024-01-01', '2024-12-31', true),
('满减优惠', '满500减50', 'fixed_amount', 50.00, 500.00, '2024-01-01', '2024-12-31', true),
('电子产品9折', '电子产品类9折优惠', 'percentage', 10.00, 0.00, '2024-01-01', '2024-06-30', true);

INSERT INTO coupons (code, name, type, value, min_amount, max_uses, start_date, end_date, is_active) VALUES 
('WELCOME100', '新用户100元券', 'fixed_amount', 100.00, 500.00, 1000, '2024-01-01', '2024-12-31', true),
('SAVE50', '满300减50', 'fixed_amount', 50.00, 300.00, 500, '2024-01-01', '2024-12-31', true),
('DISCOUNT10', '全场9折', 'percentage', 10.00, 100.00, 200, '2024-01-01', '2024-06-30', true);

-- 创建索引以提高查询性能
CREATE INDEX idx_products_name ON products(name);
CREATE INDEX idx_orders_user_status ON orders(user_id, status);
CREATE INDEX idx_order_items_order ON order_items(order_id);

COMMIT;

-- 显示创建的表
SHOW TABLES;

-- 显示用户和商品统计
SELECT 
    (SELECT COUNT(*) FROM users) as total_users,
    (SELECT COUNT(*) FROM products) as total_products,
    (SELECT COUNT(*) FROM categories) as total_categories,
    (SELECT COUNT(*) FROM promotions WHERE is_active = true) as active_promotions;
