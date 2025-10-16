-- 检查购买记录
USE emshop;

-- 1. 查看所有购买记录
SELECT '========== 所有购买记录 ==========' AS '';
SELECT 
    upr.record_id,
    upr.user_id,
    upr.product_id,
    p.name AS product_name,
    upr.quantity,
    upr.order_id,
    o.status AS order_status,
    o.payment_status,
    upr.purchase_time,
    upr.status AS record_status
FROM user_purchase_records upr
LEFT JOIN products p ON upr.product_id = p.product_id
LEFT JOIN orders o ON upr.order_id = o.order_id
ORDER BY upr.purchase_time DESC
LIMIT 20;

-- 2. 查看用户ID=9的购买记录
SELECT '========== 用户9的购买记录 ==========' AS '';
SELECT 
    upr.record_id,
    upr.product_id,
    p.name AS product_name,
    upr.quantity,
    upr.order_id,
    o.status AS order_status,
    upr.purchase_time,
    upr.status AS record_status
FROM user_purchase_records upr
LEFT JOIN products p ON upr.product_id = p.product_id
LEFT JOIN orders o ON upr.order_id = o.order_id
WHERE upr.user_id = 9
ORDER BY upr.purchase_time DESC;

-- 3. 查看订单49的详情
SELECT '========== 订单49详情 ==========' AS '';
SELECT * FROM orders WHERE order_id = 49;

SELECT '========== 订单49的订单明细 ==========' AS '';
SELECT * FROM order_items WHERE order_id = 49;

-- 4. 测试存储过程 - 用户9购买商品131 (Nintendo Switch OLED Model)
SELECT '========== 测试限购检查（用户9，商品131，数量1）==========' AS '';
CALL check_user_purchase_limit(9, 131, 1, @can, @purchased, @limit, @period);
SELECT 
    CASE WHEN @can = 1 THEN '✓ 可以购买' ELSE '✗ 超出限购' END AS result,
    @purchased AS already_purchased,
    @limit AS purchase_limit,
    @period AS limit_period;

-- 5. 手动查询用户9对商品131的购买记录（本周）
SELECT '========== 手动查询用户9本周购买商品131的记录 ==========' AS '';
SELECT 
    upr.record_id,
    upr.quantity,
    upr.purchase_time,
    upr.status,
    YEARWEEK(upr.purchase_time, 1) AS purchase_week,
    YEARWEEK(NOW(), 1) AS current_week
FROM user_purchase_records upr
WHERE upr.user_id = 9
  AND upr.product_id = 131
  AND upr.status = 'valid'
  AND YEARWEEK(upr.purchase_time, 1) = YEARWEEK(NOW(), 1);

-- 6. 查看商品131的限购设置
SELECT '========== 商品131的限购设置 ==========' AS '';
SELECT 
    product_id,
    name,
    price,
    stock_quantity,
    purchase_limit,
    purchase_limit_period
FROM products
WHERE product_id = 131;
