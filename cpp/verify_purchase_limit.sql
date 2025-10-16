-- ====================================================================
-- 验证限购功能配置
-- 执行此脚本检查限购功能是否正确配置
-- ====================================================================

USE emshop;

-- 1. 检查限购商品
SELECT '========== 限购商品列表 ==========' AS '';
SELECT 
    product_id,
    name,
    price,
    stock_quantity AS stock,
    purchase_limit,
    purchase_limit_period,
    CASE purchase_limit_period
        WHEN 'daily' THEN '每日限购'
        WHEN 'weekly' THEN '每周限购'
        WHEN 'monthly' THEN '每月限购'
        WHEN 'total' THEN '总计限购'
        ELSE '未设置'
    END AS period_desc,
    status
FROM products 
WHERE purchase_limit > 0
ORDER BY purchase_limit_period, price DESC;

-- 2. 检查存储过程是否存在
SELECT '========== 存储过程检查 ==========' AS '';
SELECT 
    ROUTINE_NAME AS procedure_name,
    ROUTINE_TYPE AS type,
    CREATED AS created_time,
    LAST_ALTERED AS last_modified
FROM information_schema.ROUTINES
WHERE ROUTINE_SCHEMA = 'emshop' 
  AND ROUTINE_NAME = 'check_user_purchase_limit';

-- 3. 检查触发器是否存在
SELECT '========== 触发器检查 ==========' AS '';
SELECT 
    TRIGGER_NAME,
    EVENT_MANIPULATION AS event,
    EVENT_OBJECT_TABLE AS `table`,
    ACTION_TIMING AS timing
FROM information_schema.TRIGGERS
WHERE TRIGGER_SCHEMA = 'emshop'
  AND TRIGGER_NAME = 'update_purchase_record_on_refund';

-- 4. 检查购买记录表
SELECT '========== 购买记录统计 ==========' AS '';
SELECT 
    status,
    COUNT(*) AS count,
    SUM(quantity) AS total_quantity
FROM user_purchase_records
GROUP BY status;

-- 5. 检查索引
SELECT '========== 索引检查 ==========' AS '';
SHOW INDEX FROM user_purchase_records 
WHERE Key_name = 'idx_user_product_time_status';

-- 6. 测试存储过程
SELECT '========== 存储过程测试 ==========' AS '';
-- 假设测试用户ID=1，商品ID=1，数量=2
CALL check_user_purchase_limit(1, 1, 2, @can, @purchased, @limit, @period);
SELECT 
    CASE WHEN @can = 1 THEN '✓ 可以购买' ELSE '✗ 超出限购' END AS result,
    @purchased AS already_purchased,
    @limit AS purchase_limit,
    @period AS limit_period;

-- 7. 查看用户1的购买记录
SELECT '========== 用户1购买记录 ==========' AS '';
SELECT 
    upr.record_id,
    upr.order_id,
    p.name AS product_name,
    upr.quantity,
    upr.purchase_time,
    upr.status,
    CASE upr.status
        WHEN 'valid' THEN '✓ 有效'
        WHEN 'cancelled' THEN '已取消'
        WHEN 'refunded' THEN '已退款'
        ELSE upr.status
    END AS status_desc
FROM user_purchase_records upr
JOIN products p ON upr.product_id = p.product_id
WHERE upr.user_id = 1
ORDER BY upr.purchase_time DESC
LIMIT 10;

-- 8. 查看用户1的购物车
SELECT '========== 用户1购物车 ==========' AS '';
SELECT 
    c.cart_id,
    p.name AS product_name,
    c.quantity AS cart_quantity,
    p.purchase_limit,
    p.purchase_limit_period,
    (SELECT COALESCE(SUM(quantity), 0) 
     FROM user_purchase_records 
     WHERE user_id = c.user_id 
       AND product_id = c.product_id 
       AND status = 'valid') AS purchased_count
FROM cart c
JOIN products p ON c.product_id = p.product_id
WHERE c.user_id = 1;

-- 完成
SELECT '========== 验证完成 ==========' AS '';
SELECT '如果以上检查都正常，限购功能应该可以正常工作' AS message;
SELECT '如果仍有问题，请重启Java服务器并清除缓存' AS tip;
