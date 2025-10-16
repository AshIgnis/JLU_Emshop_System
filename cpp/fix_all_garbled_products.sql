-- 修正所有乱码商品名称
-- 使用纯英文避免字符编码问题
SET NAMES utf8mb4;
USE emshop;

-- 修正爱马仕系列
UPDATE products SET name = 'Hermes Birkin Limited Edition' WHERE product_id IN (108, 120);
UPDATE products SET name = 'Hermes Kelly Classic' WHERE product_id IN (109, 121);
UPDATE products SET name = 'Hermes Silk Scarf Gift Box' WHERE product_id IN (110, 122);

-- 修正iPhone系列（确保唯一性）
UPDATE products SET name = 'iPhone 15 Pro Max 256GB' WHERE product_id IN (113, 125);

-- 修正球鞋
UPDATE products SET name = 'Nike Air Jordan 1 Limited' WHERE product_id IN (114, 126);

-- 修正苹果产品（使用inch替代引号）
UPDATE products SET name = 'MacBook Pro 14 inch M3 Max' WHERE product_id IN (115, 127);
UPDATE products SET name = 'iPad Pro 12.9 inch M2' WHERE product_id IN (116, 128);
UPDATE products SET name = 'AirPods Pro 2nd Generation' WHERE product_id IN (117, 129);

-- 修正家电和游戏机
UPDATE products SET name = 'Dyson V15 Detect Vacuum' WHERE product_id IN (118, 130);
UPDATE products SET name = 'Nintendo Switch OLED Model' WHERE product_id IN (119, 131);

-- 验证修正结果
SELECT product_id, name, purchase_limit, purchase_limit_period, price, stock_quantity
FROM products 
WHERE product_id BETWEEN 108 AND 131
ORDER BY product_id;
