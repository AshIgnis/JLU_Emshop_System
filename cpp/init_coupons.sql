-- 初始化优惠券数据
USE emshop;

-- 清除现有优惠券（如果需要）
-- DELETE FROM user_coupons;
-- DELETE FROM coupons;

-- 插入测试优惠券
INSERT INTO coupons (code, name, type, discount_value, min_order_amount, 
                    total_quantity, used_quantity, per_user_limit, status,
                    start_time, end_time, created_at, updated_at) 
VALUES 
('WELCOME10', '新用户欢迎券', 'fixed', 10.00, 50.00, 
 100, 0, 1, 'active', 
 '2025-01-01 00:00:00', '2025-12-31 23:59:59', NOW(), NOW()),
('DISCOUNT20', '20%折扣券', 'percentage', 0.20, 100.00, 
 50, 0, 2, 'active',
 '2025-01-01 00:00:00', '2025-12-31 23:59:59', NOW(), NOW()),
('SAVE50', '满减50元券', 'fixed', 50.00, 200.00, 
 30, 0, 1, 'active',
 '2025-01-01 00:00:00', '2025-12-31 23:59:59', NOW(), NOW()),
('FIRSTBUY', '首购优惠券', 'percentage', 0.15, 0.00, 
 200, 0, 1, 'active',
 '2025-01-01 00:00:00', '2025-12-31 23:59:59', NOW(), NOW());

-- 查看插入结果
SELECT * FROM coupons ORDER BY created_at DESC;
