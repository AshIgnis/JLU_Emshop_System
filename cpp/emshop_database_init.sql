-- ====================================================================
-- JLU Emshop System 数据库初始化脚本
-- 数据库服务器: 127.0.0.1:3306
-- 数据库名: emshop
-- 字符集: utf8mb4
-- 创建日期: 2025-09-04
-- ====================================================================

-- 创建数据库
CREATE DATABASE IF NOT EXISTS emshop 
    CHARACTER SET utf8mb4 
    COLLATE utf8mb4_unicode_ci;

-- 使用数据库
USE emshop;

-- ====================================================================
-- 核心表结构定义
-- ====================================================================

-- 1. 用户表
DROP TABLE IF EXISTS users;
CREATE TABLE users (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID',
    username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
    password VARCHAR(255) NOT NULL COMMENT '密码哈希值',
    phone VARCHAR(20) UNIQUE COMMENT '手机号',
    email VARCHAR(100) UNIQUE COMMENT '邮箱地址',
    role ENUM('user', 'admin', 'vip') DEFAULT 'user' COMMENT '用户角色',
    status ENUM('active', 'inactive', 'banned') DEFAULT 'active' COMMENT '账户状态',
    avatar_url VARCHAR(255) COMMENT '头像URL',
    real_name VARCHAR(50) COMMENT '真实姓名',
    birth_date DATE COMMENT '生日',
    gender ENUM('male', 'female', 'other') COMMENT '性别',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    last_login_time TIMESTAMP NULL COMMENT '最后登录时间',
    
    INDEX idx_username (username),
    INDEX idx_phone (phone),
    INDEX idx_email (email),
    INDEX idx_status (status),
    INDEX idx_role (role)
) ENGINE=InnoDB COMMENT='用户信息表';

-- 2. 商品分类表
DROP TABLE IF EXISTS categories;
CREATE TABLE categories (
    category_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '分类ID',
    name VARCHAR(100) NOT NULL COMMENT '分类名称',
    description TEXT COMMENT '分类描述',
    parent_id BIGINT DEFAULT 0 COMMENT '父分类ID，0表示顶级分类',
    level TINYINT DEFAULT 1 COMMENT '分类层级',
    sort_order INT DEFAULT 0 COMMENT '排序权重',
    icon_url VARCHAR(255) COMMENT '分类图标URL',
    status ENUM('active', 'inactive') DEFAULT 'active' COMMENT '分类状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    INDEX idx_parent_id (parent_id),
    INDEX idx_status (status),
    INDEX idx_sort_order (sort_order)
) ENGINE=InnoDB COMMENT='商品分类表';

-- 3. 商品表
DROP TABLE IF EXISTS products;
CREATE TABLE products (
    product_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '商品ID',
    name VARCHAR(200) NOT NULL COMMENT '商品名称',
    description TEXT COMMENT '商品描述',
    short_description VARCHAR(500) COMMENT '商品简介',
    category_id BIGINT NOT NULL COMMENT '分类ID',
    brand VARCHAR(100) COMMENT '品牌',
    price DECIMAL(12,2) NOT NULL COMMENT '当前售价',
    original_price DECIMAL(12,2) COMMENT '原价',
    cost_price DECIMAL(12,2) COMMENT '成本价',
    stock_quantity INT NOT NULL DEFAULT 0 COMMENT '库存数量',
    sold_quantity INT DEFAULT 0 COMMENT '已售数量',
    min_stock INT DEFAULT 0 COMMENT '最小库存预警',
    weight DECIMAL(8,2) COMMENT '商品重量(kg)',
    dimensions VARCHAR(100) COMMENT '商品尺寸',
    sku VARCHAR(100) UNIQUE COMMENT 'SKU编码',
    barcode VARCHAR(100) COMMENT '条形码',
    main_image VARCHAR(500) COMMENT '主图片URL',
    images JSON COMMENT '商品图片JSON数组',
    specifications JSON COMMENT '商品规格JSON',
    tags VARCHAR(500) COMMENT '商品标签，逗号分隔',
    status ENUM('active', 'inactive', 'deleted', 'out_of_stock') DEFAULT 'active' COMMENT '商品状态',
    is_featured BOOLEAN DEFAULT FALSE COMMENT '是否推荐商品',
    is_new BOOLEAN DEFAULT TRUE COMMENT '是否新品',
    rating DECIMAL(3,2) DEFAULT 0.00 COMMENT '平均评分',
    review_count INT DEFAULT 0 COMMENT '评论数量',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    INDEX idx_category_id (category_id),
    INDEX idx_price (price),
    INDEX idx_status (status),
    INDEX idx_name (name),
    INDEX idx_brand (brand),
    INDEX idx_sku (sku),
    INDEX idx_featured (is_featured),
    INDEX idx_rating (rating),
    FULLTEXT idx_search (name, description, tags),
    
    FOREIGN KEY (category_id) REFERENCES categories(category_id) ON UPDATE CASCADE
) ENGINE=InnoDB COMMENT='商品信息表';

-- 4. 购物车表
DROP TABLE IF EXISTS cart;
CREATE TABLE cart (
    cart_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '购物车ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    quantity INT NOT NULL DEFAULT 1 COMMENT '商品数量',
    selected BOOLEAN DEFAULT TRUE COMMENT '是否选中',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    UNIQUE KEY uk_user_product (user_id, product_id),
    INDEX idx_user_id (user_id),
    INDEX idx_product_id (product_id),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='购物车表';

-- 5. 订单表
DROP TABLE IF EXISTS orders;
CREATE TABLE orders (
    order_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '订单ID',
    order_no VARCHAR(32) UNIQUE NOT NULL COMMENT '订单号',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    total_amount DECIMAL(12,2) NOT NULL COMMENT '订单总金额',
    discount_amount DECIMAL(12,2) DEFAULT 0.00 COMMENT '折扣金额',
    shipping_fee DECIMAL(10,2) DEFAULT 0.00 COMMENT '运费',
    final_amount DECIMAL(12,2) NOT NULL COMMENT '实际支付金额',
    status ENUM('pending', 'confirmed', 'paid', 'shipped', 'delivered', 'completed', 'cancelled', 'refunded') DEFAULT 'pending' COMMENT '订单状态',
    payment_method VARCHAR(50) COMMENT '支付方式',
    payment_status ENUM('unpaid', 'paid', 'refunded', 'partial_refunded') DEFAULT 'unpaid' COMMENT '支付状态',
    shipping_method VARCHAR(50) COMMENT '配送方式',
    shipping_address JSON COMMENT '收货地址JSON',
    tracking_number VARCHAR(100) COMMENT '物流单号',
    coupon_code VARCHAR(50) COMMENT '使用的优惠券码',
    coupon_discount DECIMAL(10,2) DEFAULT 0.00 COMMENT '优惠券折扣金额',
    points_used INT DEFAULT 0 COMMENT '使用积分',
    points_discount DECIMAL(10,2) DEFAULT 0.00 COMMENT '积分折扣金额',
    remark TEXT COMMENT '订单备注',
    admin_remark TEXT COMMENT '管理员备注',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    confirmed_at TIMESTAMP NULL COMMENT '确认时间',
    paid_at TIMESTAMP NULL COMMENT '支付时间',
    shipped_at TIMESTAMP NULL COMMENT '发货时间',
    delivered_at TIMESTAMP NULL COMMENT '收货时间',
    
    INDEX idx_order_no (order_no),
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_payment_status (payment_status),
    INDEX idx_created_at (created_at),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON UPDATE CASCADE
) ENGINE=InnoDB COMMENT='订单表';

-- 6. 订单明细表
DROP TABLE IF EXISTS order_items;
CREATE TABLE order_items (
    item_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '订单项ID',
    order_id BIGINT NOT NULL COMMENT '订单ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    product_name VARCHAR(200) NOT NULL COMMENT '商品名称',
    product_image VARCHAR(500) COMMENT '商品图片',
    sku VARCHAR(100) COMMENT '商品SKU',
    price DECIMAL(10,2) NOT NULL COMMENT '商品单价',
    quantity INT NOT NULL COMMENT '购买数量',
    subtotal DECIMAL(12,2) NOT NULL COMMENT '小计金额',
    
    INDEX idx_order_id (order_id),
    INDEX idx_product_id (product_id),
    
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON UPDATE CASCADE
) ENGINE=InnoDB COMMENT='订单明细表';

-- 7. 优惠券表
DROP TABLE IF EXISTS coupons;
CREATE TABLE coupons (
    coupon_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '优惠券ID',
    name VARCHAR(100) NOT NULL COMMENT '优惠券名称',
    code VARCHAR(50) UNIQUE NOT NULL COMMENT '优惠券代码',
    type ENUM('fixed_amount', 'percentage', 'free_shipping') NOT NULL COMMENT '优惠类型',
    value DECIMAL(10,2) NOT NULL COMMENT '优惠值',
    min_amount DECIMAL(10,2) DEFAULT 0.00 COMMENT '最低消费金额',
    max_discount DECIMAL(10,2) COMMENT '最大折扣金额',
    total_quantity INT NOT NULL COMMENT '发行总量',
    used_quantity INT DEFAULT 0 COMMENT '已使用数量',
    per_user_limit INT DEFAULT 1 COMMENT '每人限用次数',
    start_time TIMESTAMP NOT NULL COMMENT '生效时间',
    end_time TIMESTAMP NOT NULL COMMENT '失效时间',
    status ENUM('active', 'inactive', 'expired') DEFAULT 'active' COMMENT '状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    UNIQUE INDEX uk_code (code),
    INDEX idx_status (status),
    INDEX idx_time_range (start_time, end_time)
) ENGINE=InnoDB COMMENT='优惠券表';

-- 8. 用户优惠券表
DROP TABLE IF EXISTS user_coupons;
CREATE TABLE user_coupons (
    id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT 'ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    coupon_id BIGINT NOT NULL COMMENT '优惠券ID',
    order_id BIGINT COMMENT '使用的订单ID',
    status ENUM('unused', 'used', 'expired') DEFAULT 'unused' COMMENT '使用状态',
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '领取时间',
    used_at TIMESTAMP NULL COMMENT '使用时间',
    
    INDEX idx_user_id (user_id),
    INDEX idx_coupon_id (coupon_id),
    INDEX idx_status (status),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (coupon_id) REFERENCES coupons(coupon_id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='用户优惠券表';

-- 9. 商品评论表
DROP TABLE IF EXISTS product_reviews;
CREATE TABLE product_reviews (
    review_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '评论ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    order_id BIGINT COMMENT '订单ID',
    rating TINYINT NOT NULL COMMENT '评分(1-5)',
    content TEXT COMMENT '评论内容',
    images JSON COMMENT '评论图片',
    is_anonymous BOOLEAN DEFAULT FALSE COMMENT '是否匿名',
    status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending' COMMENT '审核状态',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    INDEX idx_product_id (product_id),
    INDEX idx_user_id (user_id),
    INDEX idx_rating (rating),
    INDEX idx_status (status),
    
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='商品评论表';

-- 10. 用户地址表
DROP TABLE IF EXISTS user_addresses;
CREATE TABLE user_addresses (
    address_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '地址ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    receiver_name VARCHAR(50) NOT NULL COMMENT '收件人姓名',
    receiver_phone VARCHAR(20) NOT NULL COMMENT '收件人电话',
    province VARCHAR(50) NOT NULL COMMENT '省份',
    city VARCHAR(50) NOT NULL COMMENT '城市',
    district VARCHAR(50) NOT NULL COMMENT '区县',
    detail_address VARCHAR(200) NOT NULL COMMENT '详细地址',
    postal_code VARCHAR(10) COMMENT '邮政编码',
    is_default BOOLEAN DEFAULT FALSE COMMENT '是否默认地址',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    INDEX idx_user_id (user_id),
    INDEX idx_default (is_default),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户地址表';

-- ====================================================================
-- 初始化数据
-- ====================================================================

-- 1. 插入管理员用户
INSERT INTO users (username, password, phone, email, role, real_name) VALUES 
('admin', '$2a$10$9.rOiU6YJhRk8QLEJ7kSy.1uLnqVjYxvZzqVDNGQhRJbKWLYYMZ6e', '13800000000', 'admin@emshop.com', 'admin', '系统管理员'),
('testuser', '$2a$10$9.rOiU6YJhRk8QLEJ7kSy.1uLnqVjYxvZzqVDNGQhRJbKWLYYMZ6e', '13800000001', 'test@emshop.com', 'user', '测试用户');

-- 2. 插入商品分类
INSERT INTO categories (name, description, parent_id, level, sort_order) VALUES 
('电子数码', '电子产品和数码设备', 0, 1, 1),
('手机通讯', '智能手机、配件等', 1, 2, 1),
('电脑办公', '笔记本、台式机、办公用品', 1, 2, 2),
('家用电器', '大小家电产品', 0, 1, 2),
('服装鞋帽', '男女服装、鞋类、配饰', 0, 1, 3),
('男装', '男士服装', 5, 2, 1),
('女装', '女士服装', 5, 2, 2),
('运动户外', '运动装备、户外用品', 0, 1, 4),
('图书音像', '图书、音像制品', 0, 1, 5),
('食品生鲜', '食品、饮料、生鲜', 0, 1, 6);

-- 3. 插入测试商品
INSERT INTO products (name, description, category_id, brand, price, original_price, stock_quantity, sku, main_image, tags) VALUES 
('华为 Mate 50 Pro 256GB', '华为最新旗舰手机，搭载麒麟9000s芯片', 2, '华为', 6999.00, 7999.00, 50, 'HW-MATE50PRO-256', '/images/huawei-mate50pro.jpg', '手机,华为,旗舰'),
('小米13 Ultra 512GB', '小米影像旗舰，徕卡专业摄影', 2, '小米', 5999.00, 6999.00, 30, 'MI-13ULTRA-512', '/images/mi13ultra.jpg', '手机,小米,摄影'),
('MacBook Air M2 13英寸', 'Apple M2芯片，8核CPU，8核GPU', 3, '苹果', 9999.00, 10999.00, 20, 'MBA-M2-13-256', '/images/macbook-air-m2.jpg', '笔记本,苹果,办公'),
('Dell XPS 13 Plus', '英特尔12代酷睿i7，13.4英寸4K+触屏', 3, '戴尔', 12999.00, 14999.00, 15, 'DELL-XPS13PLUS-I7', '/images/dell-xps13plus.jpg', '笔记本,戴尔,商务'),
('Nike Air Jordan 1', '经典篮球鞋，复古潮流设计', 5, 'Nike', 1299.00, 1599.00, 100, 'NIKE-AJ1-RED-42', '/images/jordan1.jpg', '鞋子,Nike,篮球'),
('Adidas Ultra Boost 22', '缓震跑鞋，BOOST中底科技', 8, 'Adidas', 899.00, 1199.00, 80, 'ADS-UB22-BLK-42', '/images/ultraboost22.jpg', '鞋子,Adidas,跑步'),
('Java核心技术 卷I', 'Java经典教程，第12版', 9, '机械工业出版社', 99.00, 119.00, 200, 'BOOK-JAVA-CORE-V1', '/images/java-core.jpg', '图书,编程,Java'),
('iPhone 14 Pro Max 1TB', 'A16芯片，4800万像素主摄', 2, '苹果', 12999.00, 13999.00, 25, 'IP14PM-1TB-GOLD', '/images/iphone14promax.jpg', '手机,苹果,旗舰'),
('三星 Galaxy S23 Ultra', 'S Pen手写笔，200MP相机', 2, '三星', 8999.00, 9999.00, 35, 'SSG-S23U-512-BLK', '/images/galaxy-s23ultra.jpg', '手机,三星,摄影'),
('联想拯救者Y9000P', '游戏本，RTX 4060显卡', 3, '联想', 8999.00, 9999.00, 40, 'LNV-Y9000P-RTX4060', '/images/legion-y9000p.jpg', '笔记本,联想,游戏');

-- 4. 插入优惠券
INSERT INTO coupons (name, code, type, value, min_amount, total_quantity, start_time, end_time) VALUES 
('新用户专享券', 'WELCOME100', 'fixed_amount', 100.00, 500.00, 1000, '2024-01-01 00:00:00', '2024-12-31 23:59:59'),
('满千减百', 'SAVE100', 'fixed_amount', 100.00, 1000.00, 500, '2024-01-01 00:00:00', '2024-12-31 23:59:59'),
('全场9折', 'DISCOUNT10', 'percentage', 10.00, 200.00, 200, '2024-01-01 00:00:00', '2024-06-30 23:59:59'),
('包邮券', 'FREESHIP', 'free_shipping', 0.00, 99.00, 2000, '2024-01-01 00:00:00', '2024-12-31 23:59:59');

-- ====================================================================
-- 创建视图（便于查询）
-- ====================================================================

-- 商品详情视图
CREATE OR REPLACE VIEW v_product_details AS
SELECT 
    p.product_id,
    p.name,
    p.description,
    p.price,
    p.original_price,
    p.stock_quantity,
    p.sold_quantity,
    p.brand,
    p.sku,
    p.main_image,
    p.rating,
    p.review_count,
    p.status,
    c.name as category_name,
    c.category_id
FROM products p
LEFT JOIN categories c ON p.category_id = c.category_id;

-- 订单详情视图
CREATE OR REPLACE VIEW v_order_details AS
SELECT 
    o.order_id,
    o.order_no,
    o.user_id,
    u.username,
    o.total_amount,
    o.discount_amount,
    o.final_amount,
    o.status,
    o.payment_status,
    o.created_at,
    o.paid_at,
    COUNT(oi.item_id) as item_count
FROM orders o
LEFT JOIN users u ON o.user_id = u.user_id
LEFT JOIN order_items oi ON o.order_id = oi.order_id
GROUP BY o.order_id;

-- ====================================================================
-- 创建存储过程
-- ====================================================================

DELIMITER //

-- 更新商品库存的存储过程
CREATE PROCEDURE UpdateProductStock(
    IN p_product_id BIGINT,
    IN p_quantity INT,
    IN p_operation VARCHAR(10) -- 'increase' 或 'decrease'
)
BEGIN
    DECLARE current_stock INT;
    
    -- 获取当前库存
    SELECT stock_quantity INTO current_stock 
    FROM products 
    WHERE product_id = p_product_id;
    
    -- 更新库存
    IF p_operation = 'increase' THEN
        UPDATE products 
        SET stock_quantity = stock_quantity + p_quantity,
            updated_at = CURRENT_TIMESTAMP
        WHERE product_id = p_product_id;
    ELSEIF p_operation = 'decrease' THEN
        IF current_stock >= p_quantity THEN
            UPDATE products 
            SET stock_quantity = stock_quantity - p_quantity,
                sold_quantity = sold_quantity + p_quantity,
                updated_at = CURRENT_TIMESTAMP
            WHERE product_id = p_product_id;
        ELSE
            SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = '库存不足';
        END IF;
    END IF;
END //

-- 计算订单总额的存储过程
CREATE PROCEDURE CalculateOrderTotal(
    IN p_order_id BIGINT,
    OUT p_total_amount DECIMAL(12,2)
)
BEGIN
    SELECT SUM(subtotal) INTO p_total_amount
    FROM order_items
    WHERE order_id = p_order_id;
END //

DELIMITER ;

-- ====================================================================
-- 创建触发器
-- ====================================================================

-- 更新商品评分的触发器
DELIMITER //
CREATE TRIGGER update_product_rating_after_review
AFTER INSERT ON product_reviews
FOR EACH ROW
BEGIN
    UPDATE products 
    SET 
        rating = (
            SELECT AVG(rating) 
            FROM product_reviews 
            WHERE product_id = NEW.product_id AND status = 'approved'
        ),
        review_count = (
            SELECT COUNT(*) 
            FROM product_reviews 
            WHERE product_id = NEW.product_id AND status = 'approved'
        )
    WHERE product_id = NEW.product_id;
END //
DELIMITER ;

-- ====================================================================
-- 性能优化索引
-- ====================================================================

-- 复合索引优化
CREATE INDEX idx_products_category_status ON products(category_id, status);
CREATE INDEX idx_orders_user_status_date ON orders(user_id, status, created_at);
CREATE INDEX idx_order_items_order_product ON order_items(order_id, product_id);

-- ====================================================================
-- 数据统计查询
-- ====================================================================

-- 显示数据库初始化结果
SELECT 
    'Database Setup Complete' as status,
    (SELECT COUNT(*) FROM users) as total_users,
    (SELECT COUNT(*) FROM categories) as total_categories,
    (SELECT COUNT(*) FROM products) as total_products,
    (SELECT COUNT(*) FROM coupons) as total_coupons;

-- 显示所有表
SHOW TABLES;

-- 提示信息
SELECT '=== JLU Emshop System Database Initialized Successfully ===' as message;
SELECT 'Connection: 127.0.0.1:3306' as connection_info;
SELECT 'Database: emshop' as database_name;
SELECT 'Default Admin: admin / 123456' as admin_account;
SELECT 'Default Test User: testuser / 123456' as test_account;
