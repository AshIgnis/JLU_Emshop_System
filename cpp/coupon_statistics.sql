-- ============================================
-- 优惠券使用统计报表
-- ============================================
-- 用途: 为管理员提供详细的优惠券使用数据分析
-- ============================================

-- 1. 优惠券整体使用情况统计
-- ============================================
SELECT 
    '优惠券整体统计' AS report_type,
    COUNT(DISTINCT c.coupon_id) AS total_coupons,
    COUNT(DISTINCT uc.id) AS total_issued,
    SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) AS used_count,
    SUM(CASE WHEN uc.status = 'unused' THEN 1 ELSE 0 END) AS unused_count,
    SUM(CASE WHEN uc.status = 'expired' THEN 1 ELSE 0 END) AS expired_count,
    ROUND(
        SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) * 100.0 / 
        NULLIF(COUNT(uc.id), 0), 
        2
    ) AS usage_rate_percent
FROM coupons c
LEFT JOIN user_coupons uc ON c.coupon_id = uc.coupon_id;

-- 2. 各优惠券详细统计
-- ============================================
SELECT 
    c.coupon_id,
    c.code AS coupon_code,
    c.type,
    CASE 
        WHEN c.type = 'fixed' THEN CONCAT('¥', c.value)
        WHEN c.type = 'percentage' THEN CONCAT(c.value, '%')
        ELSE CAST(c.value AS CHAR)
    END AS discount_value,
    c.min_amount AS min_order_amount,
    c.status AS coupon_status,
    c.start_time,
    c.end_time,
    COUNT(uc.id) AS total_issued,
    SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) AS used_count,
    SUM(CASE WHEN uc.status = 'unused' THEN 1 ELSE 0 END) AS unused_count,
    SUM(CASE WHEN uc.status = 'expired' THEN 1 ELSE 0 END) AS expired_count,
    ROUND(
        SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) * 100.0 / 
        NULLIF(COUNT(uc.id), 0), 
        2
    ) AS usage_rate_percent,
    -- 计算总折扣金额(仅用于已使用的优惠券)
    (SELECT COALESCE(SUM(o.discount_amount), 0)
     FROM orders o
     JOIN user_coupons uc2 ON o.order_id = uc2.order_id
     WHERE uc2.coupon_id = c.coupon_id AND uc2.status = 'used'
    ) AS total_discount_amount
FROM coupons c
LEFT JOIN user_coupons uc ON c.coupon_id = uc.coupon_id
GROUP BY c.coupon_id, c.code, c.type, c.value, c.min_amount, c.status, c.start_time, c.end_time
ORDER BY used_count DESC, total_issued DESC;

-- 3. 用户优惠券使用排行榜(Top 20)
-- ============================================
SELECT 
    u.user_id,
    u.username,
    COUNT(uc.id) AS coupons_received,
    SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) AS coupons_used,
    SUM(CASE WHEN uc.status = 'unused' THEN 1 ELSE 0 END) AS coupons_unused,
    ROUND(
        SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) * 100.0 / 
        NULLIF(COUNT(uc.id), 0), 
        2
    ) AS usage_rate_percent,
    -- 计算用户通过优惠券节省的总金额
    (SELECT COALESCE(SUM(o.discount_amount), 0)
     FROM orders o
     JOIN user_coupons uc2 ON o.order_id = uc2.order_id
     WHERE uc2.user_id = u.user_id AND uc2.status = 'used'
    ) AS total_saved_amount
FROM users u
LEFT JOIN user_coupons uc ON u.user_id = uc.user_id
GROUP BY u.user_id, u.username
HAVING coupons_received > 0
ORDER BY coupons_used DESC, total_saved_amount DESC
LIMIT 20;

-- 4. 优惠券使用时间分布(按月统计)
-- ============================================
SELECT 
    DATE_FORMAT(uc.used_at, '%Y-%m') AS use_month,
    COUNT(*) AS usage_count,
    COUNT(DISTINCT uc.user_id) AS unique_users,
    COUNT(DISTINCT uc.coupon_id) AS unique_coupons,
    ROUND(AVG(o.discount_amount), 2) AS avg_discount,
    ROUND(SUM(o.discount_amount), 2) AS total_discount
FROM user_coupons uc
JOIN orders o ON uc.order_id = o.order_id
WHERE uc.status = 'used' 
  AND uc.used_at IS NOT NULL
  AND uc.used_at >= DATE_SUB(NOW(), INTERVAL 12 MONTH)
GROUP BY DATE_FORMAT(uc.used_at, '%Y-%m')
ORDER BY use_month DESC;

-- 5. 优惠券类型效果对比
-- ============================================
SELECT 
    c.type AS coupon_type,
    COUNT(DISTINCT c.coupon_id) AS coupon_count,
    COUNT(uc.id) AS total_issued,
    SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) AS used_count,
    ROUND(
        SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) * 100.0 / 
        NULLIF(COUNT(uc.id), 0), 
        2
    ) AS usage_rate_percent,
    ROUND(AVG(c.value), 2) AS avg_value,
    (SELECT COALESCE(SUM(o.discount_amount), 0)
     FROM orders o
     JOIN user_coupons uc2 ON o.order_id = uc2.order_id
     JOIN coupons c2 ON uc2.coupon_id = c2.coupon_id
     WHERE c2.type = c.type AND uc2.status = 'used'
    ) AS total_discount_amount
FROM coupons c
LEFT JOIN user_coupons uc ON c.coupon_id = uc.coupon_id
GROUP BY c.type
ORDER BY used_count DESC;

-- 6. 未使用优惠券详情(用户可能忘记使用)
-- ============================================
SELECT 
    uc.id AS user_coupon_id,
    u.user_id,
    u.username,
    c.code AS coupon_code,
    c.type,
    c.value,
    c.min_amount,
    uc.received_at,
    c.end_time,
    DATEDIFF(c.end_time, NOW()) AS days_until_expire,
    CASE 
        WHEN c.end_time < NOW() THEN '已过期'
        WHEN DATEDIFF(c.end_time, NOW()) <= 7 THEN '即将过期'
        ELSE '有效'
    END AS expire_status
FROM user_coupons uc
JOIN users u ON uc.user_id = u.user_id
JOIN coupons c ON uc.coupon_id = c.coupon_id
WHERE uc.status = 'unused'
  AND c.status = 'active'
ORDER BY c.end_time ASC
LIMIT 50;

-- 7. 优惠券对订单金额的影响分析
-- ============================================
SELECT 
    CASE 
        WHEN o.discount_amount > 0 THEN '使用优惠券'
        ELSE '未使用优惠券'
    END AS order_type,
    COUNT(*) AS order_count,
    ROUND(AVG(o.total_amount), 2) AS avg_total_amount,
    ROUND(AVG(o.discount_amount), 2) AS avg_discount_amount,
    ROUND(AVG(o.final_amount), 2) AS avg_final_amount,
    ROUND(SUM(o.total_amount), 2) AS sum_total_amount,
    ROUND(SUM(o.discount_amount), 2) AS sum_discount_amount,
    ROUND(SUM(o.final_amount), 2) AS sum_final_amount
FROM orders o
WHERE o.status NOT IN ('cancelled', 'refunded')
  AND o.created_at >= DATE_SUB(NOW(), INTERVAL 3 MONTH)
GROUP BY CASE WHEN o.discount_amount > 0 THEN '使用优惠券' ELSE '未使用优惠券' END;

-- 8. 优惠券分发渠道统计(假设有分发渠道字段)
-- ============================================
-- 注意: 此查询假设user_coupons表有source字段,如果没有则跳过
-- SELECT 
--     uc.source AS distribution_channel,
--     COUNT(*) AS issued_count,
--     SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) AS used_count,
--     ROUND(
--         SUM(CASE WHEN uc.status = 'used' THEN 1 ELSE 0 END) * 100.0 / 
--         COUNT(*), 
--         2
--     ) AS usage_rate_percent
-- FROM user_coupons uc
-- GROUP BY uc.source
-- ORDER BY issued_count DESC;

-- ============================================
-- 导出提示
-- ============================================
-- 如需导出为CSV文件,可使用以下命令:
-- mysql -u用户名 -p密码 数据库名 < coupon_statistics.sql > report.txt
-- 或在MySQL Workbench中右键"Export Recordset to an External File"
