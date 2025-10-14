-- ====================================================================
-- JLU Emshop System - 业务逻辑完善数据库升级脚本
-- 版本: 1.1.0
-- 创建日期: 2025-10-13
-- 用途: 完善退款、库存、优惠券、通知等业务逻辑
-- ====================================================================

USE emshop;

-- ====================================================================
-- 1. 修改订单状态,添加 refunding (退款中)
-- ====================================================================

ALTER TABLE orders MODIFY COLUMN status 
ENUM('pending', 'confirmed', 'paid', 'shipped', 'delivered', 
     'completed', 'cancelled', 'refunding', 'refunded') 
DEFAULT 'pending' 
COMMENT '订单状态: pending-待确认, confirmed-已确认, paid-已支付, shipped-已发货, delivered-已送达, completed-已完成, cancelled-已取消, refunding-退款中, refunded-已退款';

-- ====================================================================
-- 2. 创建用户通知表
-- ====================================================================

DROP TABLE IF EXISTS user_notifications;
CREATE TABLE user_notifications (
    notification_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '通知ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    type ENUM('order_update', 'refund_approved', 'refund_rejected', 
              'low_stock', 'coupon_available', 'system_notice') NOT NULL COMMENT '通知类型',
    title VARCHAR(200) NOT NULL COMMENT '通知标题',
    content TEXT NOT NULL COMMENT '通知内容',
    related_id BIGINT COMMENT '关联ID(订单ID/优惠券ID等)',
    is_read BOOLEAN DEFAULT FALSE COMMENT '是否已读',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    
    INDEX idx_user_id (user_id),
    INDEX idx_is_read (is_read),
    INDEX idx_type (type),
    INDEX idx_created_at (created_at),
    
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户通知表';

-- ====================================================================
-- 3. 创建退款申请表
-- ====================================================================

DROP TABLE IF EXISTS refund_requests;
CREATE TABLE refund_requests (
    refund_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '退款ID',
    order_id BIGINT NOT NULL COMMENT '订单ID',
    user_id BIGINT NOT NULL COMMENT '用户ID',
    reason TEXT NOT NULL COMMENT '退款原因',
    refund_amount DECIMAL(12,2) NOT NULL COMMENT '退款金额',
    status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending' COMMENT '状态: pending-待审核, approved-已批准, rejected-已拒绝',
    admin_reply TEXT COMMENT '管理员回复',
    images JSON COMMENT '退款凭证图片',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '申请时间',
    processed_at TIMESTAMP NULL COMMENT '处理时间',
    processed_by BIGINT COMMENT '处理人ID(管理员)',
    
    INDEX idx_order_id (order_id),
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at),
    
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='退款申请表';

-- ====================================================================
-- 4. 创建库存变动日志表
-- ====================================================================

DROP TABLE IF EXISTS stock_logs;
CREATE TABLE stock_logs (
    log_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '日志ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    change_quantity INT NOT NULL COMMENT '变动数量(正数为增加,负数为减少)',
    before_quantity INT NOT NULL COMMENT '变动前库存',
    after_quantity INT NOT NULL COMMENT '变动后库存',
    reason VARCHAR(100) NOT NULL COMMENT '变动原因',
    related_type ENUM('order', 'refund', 'manual', 'return', 'adjust') NOT NULL COMMENT '关联类型',
    related_id BIGINT COMMENT '关联ID(订单ID/退款ID等)',
    operator_id BIGINT COMMENT '操作人ID',
    operator_type ENUM('user', 'admin', 'system') DEFAULT 'system' COMMENT '操作人类型',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    
    INDEX idx_product_id (product_id),
    INDEX idx_created_at (created_at),
    INDEX idx_related (related_type, related_id),
    
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='库存变动日志表';

-- ====================================================================
-- 5. 创建优惠活动模板表
-- ====================================================================

DROP TABLE IF EXISTS coupon_templates;
CREATE TABLE coupon_templates (
    template_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '模板ID',
    name VARCHAR(100) NOT NULL COMMENT '模板名称',
    template_type ENUM('full_reduction', 'discount', 'free_shipping', 'gift') NOT NULL COMMENT '模板类型',
    description TEXT COMMENT '模板描述',
    icon_url VARCHAR(255) COMMENT '图标URL',
    is_active BOOLEAN DEFAULT TRUE COMMENT '是否启用',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    
    INDEX idx_template_type (template_type),
    INDEX idx_is_active (is_active)
) ENGINE=InnoDB COMMENT='优惠券模板表';

-- 插入默认模板
INSERT INTO coupon_templates (name, template_type, description) VALUES
('新用户专享', 'full_reduction', '新用户首次购物满额立减'),
('满减优惠', 'full_reduction', '满指定金额减现金'),
('折扣优惠', 'discount', '全场/指定商品打折'),
('包邮券', 'free_shipping', '免除运费'),
('赠品券', 'gift', '购买指定商品赠送礼品');

-- ====================================================================
-- 6. 修改优惠券表,添加模板关联
-- ====================================================================

-- 检查并添加 template_id 列
SET @col_exists = 0;
SELECT COUNT(*) INTO @col_exists 
FROM information_schema.columns 
WHERE table_schema = 'emshop' 
  AND table_name = 'coupons' 
  AND column_name = 'template_id';

SET @sql = IF(@col_exists = 0,
    'ALTER TABLE coupons ADD COLUMN template_id BIGINT COMMENT ''关联模板ID'' AFTER coupon_id',
    'SELECT ''Column template_id already exists'' AS message');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- 检查并添加 description 列
SET @col_exists = 0;
SELECT COUNT(*) INTO @col_exists 
FROM information_schema.columns 
WHERE table_schema = 'emshop' 
  AND table_name = 'coupons' 
  AND column_name = 'description';

SET @sql = IF(@col_exists = 0,
    'ALTER TABLE coupons ADD COLUMN description TEXT COMMENT ''优惠券详细说明'' AFTER name',
    'SELECT ''Column description already exists'' AS message');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- 检查并添加 usage_instructions 列
SET @col_exists = 0;
SELECT COUNT(*) INTO @col_exists 
FROM information_schema.columns 
WHERE table_schema = 'emshop' 
  AND table_name = 'coupons' 
  AND column_name = 'usage_instructions';

SET @sql = IF(@col_exists = 0,
    'ALTER TABLE coupons ADD COLUMN usage_instructions TEXT COMMENT ''使用说明'' AFTER description',
    'SELECT ''Column usage_instructions already exists'' AS message');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- 检查并添加索引
SET @index_exists = 0;
SELECT COUNT(*) INTO @index_exists 
FROM information_schema.statistics 
WHERE table_schema = 'emshop' 
  AND table_name = 'coupons' 
  AND index_name = 'idx_template_id';

SET @sql = IF(@index_exists = 0,
    'ALTER TABLE coupons ADD INDEX idx_template_id (template_id)',
    'SELECT ''Index idx_template_id already exists'' AS message');
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ====================================================================
-- 7. 创建低库存触发器
-- ====================================================================

DELIMITER //

-- 删除旧触发器(如果存在)
DROP TRIGGER IF EXISTS after_product_stock_update //

-- 创建新触发器
CREATE TRIGGER after_product_stock_update
AFTER UPDATE ON products
FOR EACH ROW
BEGIN
    -- 低库存预警(库存<=预警值且>0)
    IF NEW.stock_quantity <= NEW.min_stock AND NEW.stock_quantity > 0 AND OLD.stock_quantity > NEW.min_stock THEN
        INSERT INTO user_notifications (user_id, type, title, content, related_id)
        SELECT user_id, 'low_stock', 
               CONCAT('⚠️ 商品库存不足预警'),
               CONCAT('商品【', NEW.name, '】库存不足！\n',
                     '商品ID: ', NEW.product_id, '\n',
                     '当前库存: ', NEW.stock_quantity, '\n',
                     '预警阈值: ', NEW.min_stock, '\n',
                     '建议及时补货'),
               NEW.product_id
        FROM users WHERE role = 'admin';
    END IF;
    
    -- 缺货自动下架(库存=0)
    IF NEW.stock_quantity = 0 AND OLD.stock_quantity > 0 THEN
        UPDATE products SET status = 'out_of_stock' WHERE product_id = NEW.product_id;
        
        INSERT INTO user_notifications (user_id, type, title, content, related_id)
        SELECT user_id, 'low_stock', 
               CONCAT('🚫 商品已缺货'),
               CONCAT('商品【', NEW.name, '】库存为0，已自动下架\n',
                     '商品ID: ', NEW.product_id),
               NEW.product_id
        FROM users WHERE role = 'admin';
    END IF;
    
    -- 补货后自动上架
    IF NEW.stock_quantity > 0 AND OLD.stock_quantity = 0 AND NEW.status = 'out_of_stock' THEN
        UPDATE products SET status = 'active' WHERE product_id = NEW.product_id;
    END IF;
END //

DELIMITER ;

-- ====================================================================
-- 8. 创建存储过程: 记录库存变动
-- ====================================================================

DELIMITER //

DROP PROCEDURE IF EXISTS LogStockChange //

CREATE PROCEDURE LogStockChange(
    IN p_product_id BIGINT,
    IN p_change_qty INT,
    IN p_reason VARCHAR(100),
    IN p_related_type ENUM('order', 'refund', 'manual', 'return', 'adjust'),
    IN p_related_id BIGINT,
    IN p_operator_id BIGINT,
    IN p_operator_type ENUM('user', 'admin', 'system')
)
BEGIN
    DECLARE v_before_qty INT;
    DECLARE v_after_qty INT;
    
    -- 获取当前库存
    SELECT stock_quantity INTO v_before_qty 
    FROM products 
    WHERE product_id = p_product_id;
    
    -- 计算变动后库存
    SET v_after_qty = v_before_qty + p_change_qty;
    
    -- 插入日志
    INSERT INTO stock_logs (
        product_id, change_quantity, before_quantity, after_quantity,
        reason, related_type, related_id, operator_id, operator_type
    ) VALUES (
        p_product_id, p_change_qty, v_before_qty, v_after_qty,
        p_reason, p_related_type, p_related_id, p_operator_id, p_operator_type
    );
END //

DELIMITER ;

-- ====================================================================
-- 9. 创建存储过程: 批量发放优惠券
-- ====================================================================

DELIMITER //

DROP PROCEDURE IF EXISTS DistributeCoupons //

CREATE PROCEDURE DistributeCoupons(
    IN p_coupon_id BIGINT,
    IN p_user_ids TEXT  -- 逗号分隔的用户ID列表
)
BEGIN
    DECLARE done INT DEFAULT FALSE;
    DECLARE v_user_id BIGINT;
    DECLARE v_pos INT;
    DECLARE v_user_list TEXT;
    
    SET v_user_list = CONCAT(p_user_ids, ',');
    
    WHILE LENGTH(v_user_list) > 0 DO
        SET v_pos = LOCATE(',', v_user_list);
        IF v_pos > 0 THEN
            SET v_user_id = CAST(SUBSTRING(v_user_list, 1, v_pos - 1) AS UNSIGNED);
            SET v_user_list = SUBSTRING(v_user_list, v_pos + 1);
            
            -- 插入用户优惠券
            INSERT INTO user_coupons (user_id, coupon_id, status)
            VALUES (v_user_id, p_coupon_id, 'unused')
            ON DUPLICATE KEY UPDATE coupon_id = coupon_id; -- 防止重复
            
            -- 发送通知
            INSERT INTO user_notifications (user_id, type, title, content, related_id)
            SELECT v_user_id, 'coupon_available',
                   '🎉 新优惠券到账',
                   CONCAT('您获得了新的优惠券【', name, '】\n',
                         '优惠内容: ', 
                         CASE type
                             WHEN 'fixed_amount' THEN CONCAT('满', min_amount, '减', value)
                             WHEN 'percentage' THEN CONCAT(value, '折优惠')
                             WHEN 'free_shipping' THEN '免运费'
                             ELSE '特殊优惠'
                         END, '\n',
                         '有效期至: ', DATE_FORMAT(end_time, '%Y-%m-%d')),
                   p_coupon_id
            FROM coupons WHERE coupon_id = p_coupon_id;
        ELSE
            SET v_user_list = '';
        END IF;
    END WHILE;
END //

DELIMITER ;

-- ====================================================================
-- 10. 创建视图: 退款申请详情
-- ====================================================================

CREATE OR REPLACE VIEW v_refund_details AS
SELECT 
    r.refund_id,
    r.order_id,
    o.order_no,
    r.user_id,
    u.username,
    u.phone,
    r.reason,
    r.refund_amount,
    r.status as refund_status,
    r.admin_reply,
    r.created_at as apply_time,
    r.processed_at,
    a.username as admin_name,
    o.status as order_status,
    o.payment_status
FROM refund_requests r
LEFT JOIN orders o ON r.order_id = o.order_id
LEFT JOIN users u ON r.user_id = u.user_id
LEFT JOIN users a ON r.processed_by = a.user_id
ORDER BY r.created_at DESC;

-- ====================================================================
-- 11. 创建视图: 库存变动汇总
-- ====================================================================

CREATE OR REPLACE VIEW v_stock_summary AS
SELECT 
    p.product_id,
    p.name as product_name,
    p.stock_quantity as current_stock,
    p.min_stock,
    p.sold_quantity,
    COALESCE(SUM(CASE WHEN sl.change_quantity < 0 THEN ABS(sl.change_quantity) ELSE 0 END), 0) as total_out,
    COALESCE(SUM(CASE WHEN sl.change_quantity > 0 THEN sl.change_quantity ELSE 0 END), 0) as total_in,
    CASE 
        WHEN p.stock_quantity = 0 THEN 'out_of_stock'
        WHEN p.stock_quantity <= p.min_stock THEN 'low_stock'
        ELSE 'normal'
    END as stock_status
FROM products p
LEFT JOIN stock_logs sl ON p.product_id = sl.product_id
GROUP BY p.product_id;

-- ====================================================================
-- 12. 插入测试数据
-- ====================================================================

-- 插入管理员通知示例
INSERT INTO user_notifications (user_id, type, title, content, related_id)
SELECT user_id, 'system_notice', 
       '系统升级完成',
       '业务逻辑优化已完成:\n1. 新增退款审核流程\n2. 库存预警机制\n3. 优惠券使用说明\n4. 活动管理简化',
       NULL
FROM users WHERE role = 'admin';

-- ====================================================================
-- 13. 数据统计
-- ====================================================================

SELECT 
    '数据库升级完成' as status,
    (SELECT COUNT(*) FROM user_notifications) as notifications_count,
    (SELECT COUNT(*) FROM refund_requests) as refund_requests_count,
    (SELECT COUNT(*) FROM stock_logs) as stock_logs_count,
    (SELECT COUNT(*) FROM coupon_templates) as coupon_templates_count;

-- 显示新表
SHOW TABLES LIKE '%notifications%';
SHOW TABLES LIKE '%refund%';
SHOW TABLES LIKE '%stock%';
SHOW TABLES LIKE '%template%';

SELECT '=== JLU Emshop System 业务逻辑升级完成 ===' as message;
SELECT '新增4张表: user_notifications, refund_requests, stock_logs, coupon_templates' as tables_info;
SELECT '新增3个存储过程, 1个触发器, 2个视图' as procedures_info;
SELECT '升级完成时间' as info, NOW() as timestamp;
