-- ====================================================================
-- JLU Emshop System - 库存变动审计表
-- 用于记录所有库存变化,支持追溯和审计
-- 创建日期: 2025-10-12
-- ====================================================================

USE emshop;

-- 库存变动审计表
CREATE TABLE IF NOT EXISTS stock_change_audit (
    audit_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '审计记录ID',
    product_id BIGINT NOT NULL COMMENT '商品ID',
    order_id BIGINT NULL COMMENT '关联订单ID(如果适用)',
    user_id BIGINT NULL COMMENT '操作用户ID',
    change_type VARCHAR(32) NOT NULL COMMENT '变动类型: order_deduct(下单扣减)/refund_restore(退款返还)/manual_adjust(手动调整)/restock(补货)/init(初始化)',
    quantity_before INT NOT NULL COMMENT '变动前库存',
    quantity_change INT NOT NULL COMMENT '变动数量(正数为增加,负数为减少)',
    quantity_after INT NOT NULL COMMENT '变动后库存',
    operator VARCHAR(64) DEFAULT 'system' COMMENT '操作者(system/admin用户名)',
    reason TEXT COMMENT '变动原因说明',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
    
    INDEX idx_product_id (product_id),
    INDEX idx_order_id (order_id),
    INDEX idx_user_id (user_id),
    INDEX idx_change_type (change_type),
    INDEX idx_created_at (created_at),
    
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE,
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE SET NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='库存变动审计记录表';

-- 订单状态变更审计表
CREATE TABLE IF NOT EXISTS order_status_audit (
    audit_id BIGINT PRIMARY KEY AUTO_INCREMENT COMMENT '审计记录ID',
    order_id BIGINT NOT NULL COMMENT '订单ID',
    user_id BIGINT NULL COMMENT '操作用户ID',
    old_status VARCHAR(32) COMMENT '原状态',
    new_status VARCHAR(32) NOT NULL COMMENT '新状态',
    operator VARCHAR(64) DEFAULT 'system' COMMENT '操作者(system/admin用户名/user)',
    reason TEXT COMMENT '状态变更原因',
    metadata JSON COMMENT '附加元数据(如退款金额、物流单号等)',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
    
    INDEX idx_order_id (order_id),
    INDEX idx_user_id (user_id),
    INDEX idx_created_at (created_at),
    
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
) ENGINE=InnoDB COMMENT='订单状态变更审计记录表';

-- 创建用于自动记录库存变动的存储过程
DELIMITER $$

CREATE PROCEDURE record_stock_change(
    IN p_product_id BIGINT,
    IN p_order_id BIGINT,
    IN p_user_id BIGINT,
    IN p_change_type VARCHAR(32),
    IN p_quantity_before INT,
    IN p_quantity_change INT,
    IN p_quantity_after INT,
    IN p_operator VARCHAR(64),
    IN p_reason TEXT
)
BEGIN
    INSERT INTO stock_change_audit (
        product_id, order_id, user_id, change_type,
        quantity_before, quantity_change, quantity_after,
        operator, reason
    ) VALUES (
        p_product_id, p_order_id, p_user_id, p_change_type,
        p_quantity_before, p_quantity_change, p_quantity_after,
        p_operator, p_reason
    );
END$$

DELIMITER ;

-- 创建用于自动记录订单状态变更的存储过程
DELIMITER $$

CREATE PROCEDURE record_order_status_change(
    IN p_order_id BIGINT,
    IN p_user_id BIGINT,
    IN p_old_status VARCHAR(32),
    IN p_new_status VARCHAR(32),
    IN p_operator VARCHAR(64),
    IN p_reason TEXT,
    IN p_metadata JSON
)
BEGIN
    INSERT INTO order_status_audit (
        order_id, user_id, old_status, new_status,
        operator, reason, metadata
    ) VALUES (
        p_order_id, p_user_id, p_old_status, p_new_status,
        p_operator, p_reason, p_metadata
    );
END$$

DELIMITER ;

-- 创建触发器：商品库存变动时自动记录
DELIMITER $$

CREATE TRIGGER trg_product_stock_change_audit
AFTER UPDATE ON products
FOR EACH ROW
BEGIN
    -- 只在库存发生变化时记录
    IF OLD.stock_quantity != NEW.stock_quantity THEN
        INSERT INTO stock_change_audit (
            product_id, 
            change_type, 
            quantity_before, 
            quantity_change, 
            quantity_after,
            operator,
            reason
        ) VALUES (
            NEW.product_id,
            'auto_detected',
            OLD.stock_quantity,
            NEW.stock_quantity - OLD.stock_quantity,
            NEW.stock_quantity,
            'trigger',
            CONCAT('库存从 ', OLD.stock_quantity, ' 变更为 ', NEW.stock_quantity)
        );
    END IF;
END$$

DELIMITER ;

-- 测试数据插入示例
-- INSERT INTO stock_change_audit (product_id, order_id, change_type, quantity_before, quantity_change, quantity_after, operator, reason)
-- VALUES (1, 1001, 'order_deduct', 100, -5, 95, 'system', '用户下单购买5件商品');

-- 查询某商品的库存变动历史
-- SELECT * FROM stock_change_audit WHERE product_id = 1 ORDER BY created_at DESC;

-- 查询某订单的库存变动
-- SELECT * FROM stock_change_audit WHERE order_id = 1001;

-- 查询某时间段内的所有库存变动
-- SELECT * FROM stock_change_audit WHERE created_at BETWEEN '2025-10-01' AND '2025-10-31';

-- 统计各类型库存变动的次数
-- SELECT change_type, COUNT(*) as count, SUM(ABS(quantity_change)) as total_quantity
-- FROM stock_change_audit 
-- GROUP BY change_type;

COMMIT;

-- 显示创建结果
SELECT 'stock_change_audit 表创建成功!' as result;
SELECT 'order_status_audit 表创建成功!' as result;
SELECT '存储过程创建成功!' as result;
SELECT '触发器创建成功!' as result;
