-- ====================================================================
-- 商品限购功能升级脚本
-- 功能：为商品表添加个人限购字段
-- 日期：2025-10-15
-- ====================================================================

USE emshop;

-- 1. 为商品表添加限购相关字段
ALTER TABLE products 
ADD COLUMN purchase_limit INT DEFAULT 0 COMMENT '个人限购数量，0表示不限购' AFTER stock_quantity,
ADD COLUMN purchase_limit_period ENUM('total', 'daily', 'weekly', 'monthly') DEFAULT 'total' 
    COMMENT '限购周期：total-总限购，daily-每日限购，weekly-每周限购，monthly-每月限购' AFTER purchase_limit;

-- 2. 创建用户购买记录表（用于统计限购）
DROP TABLE IF EXISTS user_purchase_records;
CREATE TABLE user_purchase_records (
    record_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '记录ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    quantity INT NOT NULL COMMENT '购买数量',
    order_id BIGINT NOT NULL COMMENT '关联订单ID',
    purchase_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '购买时间',
    status ENUM('valid', 'refunded', 'cancelled') DEFAULT 'valid' COMMENT '记录状态',
    
    INDEX idx_user_product (user_id, product_id),
    INDEX idx_product_id (product_id),
    INDEX idx_purchase_time (purchase_time),
    INDEX idx_status (status),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户购买记录表（用于限购统计）';

-- 3. 为一些热门商品设置限购（示例数据）
UPDATE products SET purchase_limit = 2, purchase_limit_period = 'total' 
WHERE name LIKE '%iPhone%' OR name LIKE '%限量%';

UPDATE products SET purchase_limit = 1, purchase_limit_period = 'daily' 
WHERE name LIKE '%爱马仕%' OR name LIKE '%抢购%';

UPDATE products SET purchase_limit = 5, purchase_limit_period = 'monthly' 
WHERE category_id IN (SELECT category_id FROM categories WHERE name LIKE '%电子%');

-- 4. 创建查询用户购买数量的存储过程
DELIMITER //

DROP PROCEDURE IF EXISTS check_user_purchase_limit//
CREATE PROCEDURE check_user_purchase_limit(
    IN p_user_id BIGINT,
    IN p_product_id BIGINT,
    IN p_quantity INT,
    OUT can_purchase BOOLEAN,
    OUT purchased_count INT,
    OUT limit_count INT,
    OUT limit_period VARCHAR(20)
)
BEGIN
    DECLARE v_purchase_limit INT;
    DECLARE v_purchase_limit_period VARCHAR(20);
    DECLARE v_purchased_count INT DEFAULT 0;
    DECLARE v_start_time TIMESTAMP;
    
    -- 获取商品限购配置
    SELECT purchase_limit, purchase_limit_period 
    INTO v_purchase_limit, v_purchase_limit_period
    FROM products 
    WHERE product_id = p_product_id;
    
    -- 如果不限购，直接返回可以购买
    IF v_purchase_limit = 0 OR v_purchase_limit IS NULL THEN
        SET can_purchase = TRUE;
        SET purchased_count = 0;
        SET limit_count = 0;
        SET limit_period = 'unlimited';
    ELSE
    
    -- 根据限购周期计算起始时间
    CASE v_purchase_limit_period
        WHEN 'daily' THEN
            SET v_start_time = DATE_FORMAT(NOW(), '%Y-%m-%d 00:00:00');
        WHEN 'weekly' THEN
            SET v_start_time = DATE_SUB(NOW(), INTERVAL WEEKDAY(NOW()) DAY);
            SET v_start_time = DATE_FORMAT(v_start_time, '%Y-%m-%d 00:00:00');
        WHEN 'monthly' THEN
            SET v_start_time = DATE_FORMAT(NOW(), '%Y-%m-01 00:00:00');
        ELSE -- 'total'
            SET v_start_time = '1970-01-01 00:00:00';
    END CASE;
    
    -- 统计用户在指定周期内的购买数量
    SELECT COALESCE(SUM(quantity), 0)
    INTO v_purchased_count
    FROM user_purchase_records
    WHERE user_id = p_user_id 
        AND product_id = p_product_id
        AND purchase_time >= v_start_time
        AND status = 'valid';
    
    -- 判断是否可以购买
    IF v_purchased_count + p_quantity <= v_purchase_limit THEN
        SET can_purchase = TRUE;
    ELSE
        SET can_purchase = FALSE;
    END IF;
    
    -- 返回信息
    SET purchased_count = v_purchased_count;
    SET limit_count = v_purchase_limit;
    SET limit_period = v_purchase_limit_period;
    
    END IF;
END//

DELIMITER ;

-- 5. 创建索引优化查询性能
CREATE INDEX idx_user_product_time_status ON user_purchase_records(user_id, product_id, purchase_time, status);

-- 6. 添加触发器：订单取消或退款时更新购买记录状态
DELIMITER //

DROP TRIGGER IF EXISTS update_purchase_record_on_refund//
CREATE TRIGGER update_purchase_record_on_refund
AFTER UPDATE ON orders
FOR EACH ROW
BEGIN
    -- 如果订单状态变为已取消或已退款，更新购买记录
    IF NEW.status IN ('cancelled', 'refunded') AND OLD.status NOT IN ('cancelled', 'refunded') THEN
        UPDATE user_purchase_records 
        SET status = NEW.status
        WHERE order_id = NEW.order_id;
    END IF;
END//

DELIMITER ;

-- 完成提示
SELECT '✅ 商品限购功能数据库升级完成！' AS message;
SELECT '已添加字段：purchase_limit（限购数量）、purchase_limit_period（限购周期）' AS info;
SELECT '已创建表：user_purchase_records（用户购买记录表）' AS info2;
SELECT '已创建存储过程：check_user_purchase_limit（检查限购）' AS info3;

