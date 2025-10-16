-- ====================================================================
-- 限购商品测试数据初始化脚本
-- 日期：2025-10-15
-- ====================================================================

USE emshop;

-- 插入限购测试商品
INSERT INTO products (name, description, category_id, price, original_price, stock_quantity, 
                     purchase_limit, purchase_limit_period, status, is_featured, main_image) 
VALUES
-- 爱马仕系列 - 每日限购1件
('爱马仕Birkin经典款手提包', '法国原装进口，纯手工缝制，鳄鱼皮材质，限量发售', 1, 280000.00, 320000.00, 5, 1, 'daily', 'active', true, '/img/hermes_birkin.jpg'),
('爱马仕Kelly凯莉包', '经典Kelly包，小牛皮材质，优雅奢华', 1, 180000.00, 200000.00, 3, 1, 'daily', 'active', true, '/img/hermes_kelly.jpg'),
('爱马仕丝巾限量款', '100%真丝，法国设计师限量款', 1, 8800.00, 9800.00, 20, 1, 'daily', 'active', true, '/img/hermes_scarf.jpg'),

-- iPhone系列 - 总限购2件
('iPhone 15 Pro Max 1TB', '全新A17 Pro芯片，钛金属边框，超强性能', 2, 10999.00, 11999.00, 100, 2, 'total', 'active', true, '/img/iphone15promax.jpg'),
('iPhone 15 Pro 512GB', '专业级影像系统，动作按钮，全新设计', 2, 8999.00, 9999.00, 150, 2, 'total', 'active', true, '/img/iphone15pro.jpg'),

-- 限时抢购商品 - 每人限购3件
('双十一特价茅台酒', '53度飞天茅台500ml，正品保证', 3, 1499.00, 2999.00, 50, 3, 'total', 'active', true, '/img/maotai.jpg'),
('限量款AJ1球鞋', 'Air Jordan 1 限量配色，全新正品', 4, 1999.00, 2999.00, 30, 3, 'total', 'active', true, '/img/aj1.jpg'),

-- 电子产品 - 每月限购5件
('MacBook Pro 14寸 M3 Max', '36GB内存，1TB固态硬盘，专业创作工具', 2, 22999.00, 24999.00, 50, 5, 'monthly', 'active', true, '/img/macbook_pro.jpg'),
('iPad Pro 12.9寸 M2', '性能强大，支持Apple Pencil 2', 2, 8999.00, 9999.00, 80, 5, 'monthly', 'active', true, '/img/ipad_pro.jpg'),
('AirPods Pro 2代', '主动降噪，空间音频，无线充电', 2, 1899.00, 1999.00, 200, 5, 'monthly', 'active', true, '/img/airpods_pro.jpg'),

-- 抢购商品 - 每周限购2件
('戴森V15吸尘器', '激光探测技术，深度清洁', 5, 4999.00, 5999.00, 40, 2, 'weekly', 'active', true, '/img/dyson_v15.jpg'),
('Switch OLED游戏机', '任天堂正版游戏机，OLED屏幕', 2, 2399.00, 2699.00, 60, 2, 'weekly', 'active', true, '/img/switch_oled.jpg');

-- 更新现有商品为限购商品（如果存在）
UPDATE products 
SET purchase_limit = 2, purchase_limit_period = 'total' 
WHERE name LIKE '%iPhone%' AND purchase_limit = 0;

UPDATE products 
SET purchase_limit = 1, purchase_limit_period = 'daily' 
WHERE name LIKE '%爱马仕%' AND purchase_limit = 0;

UPDATE products 
SET purchase_limit = 5, purchase_limit_period = 'monthly' 
WHERE category_id IN (SELECT category_id FROM categories WHERE name LIKE '%电子%') 
AND purchase_limit = 0;

-- 显示添加的限购商品
SELECT 
    product_id,
    name,
    price,
    stock_quantity AS stock,
    purchase_limit,
    purchase_limit_period,
    CASE purchase_limit_period
        WHEN 'daily' THEN 'Daily'
        WHEN 'weekly' THEN 'Weekly'
        WHEN 'monthly' THEN 'Monthly'
        WHEN 'total' THEN 'Total'
        ELSE purchase_limit_period
    END AS period_desc,
    status
FROM products 
WHERE purchase_limit > 0
ORDER BY 
    CASE purchase_limit_period
        WHEN 'daily' THEN 1
        WHEN 'weekly' THEN 2
        WHEN 'monthly' THEN 3
        WHEN 'total' THEN 4
    END,
    price DESC;

SELECT 'Purchase limit products initialized successfully!' AS message;
SELECT CONCAT('Total ', COUNT(*), ' products with purchase limit') AS summary
FROM products 
WHERE purchase_limit > 0;
