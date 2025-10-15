-- ============================================
-- 优惠券过期检查和标记脚本
-- ============================================
-- 用途: 定期执行此脚本标记过期优惠券
-- 建议: 配置定时任务(如Windows Task Scheduler或Cron)每天执行一次
-- ============================================

-- 开启事务
START TRANSACTION;

-- 标记过期的优惠券
-- 只更新未使用的优惠券,已使用的保持不变
UPDATE user_coupons uc
JOIN coupons c ON uc.coupon_id = c.coupon_id
SET uc.status = 'expired'
WHERE uc.status = 'unused' 
  AND c.end_time < NOW();

-- 获取本次更新的记录数
SELECT ROW_COUNT() AS expired_coupons_count;

-- 提交事务
COMMIT;

-- 查询已过期但未标记的优惠券(用于验证)
-- 这个查询应该返回0条记录
SELECT 
    uc.id AS user_coupon_id,
    uc.user_id,
    c.code AS coupon_code,
    c.coupon_id,
    uc.status,
    c.end_time,
    NOW() AS current_time
FROM user_coupons uc
JOIN coupons c ON uc.coupon_id = c.coupon_id
WHERE uc.status = 'unused' 
  AND c.end_time < NOW()
ORDER BY c.end_time DESC;

-- 统计各状态优惠券数量
SELECT 
    uc.status,
    COUNT(*) AS count,
    COUNT(DISTINCT uc.user_id) AS unique_users,
    COUNT(DISTINCT uc.coupon_id) AS unique_coupons
FROM user_coupons uc
GROUP BY uc.status
ORDER BY FIELD(uc.status, 'unused', 'used', 'expired');

-- ============================================
-- 可选: 清理非常旧的过期优惠券记录
-- ============================================
-- 注意: 谨慎使用,可能影响历史数据分析
-- 删除6个月前过期的优惠券记录

-- DELETE FROM user_coupons 
-- WHERE status = 'expired' 
--   AND id IN (
--       SELECT uc.id 
--       FROM user_coupons uc
--       JOIN coupons c ON uc.coupon_id = c.coupon_id
--       WHERE c.end_time < DATE_SUB(NOW(), INTERVAL 6 MONTH)
--   );
