-- ============================================
-- 优惠券功能数据库索引优化
-- ============================================
-- 用途: 优化优惠券相关查询性能
-- 执行前请备份数据库!
-- ============================================

USE emshop;

-- 显示执行信息
SET @start_time = NOW();
SELECT CONCAT('开始时间: ', @start_time) AS info;

-- ============================================
-- 1. 用户优惠券组合索引 (最重要!)
-- ============================================
-- 用于: 订单创建时验证用户拥有且未使用的优惠券
-- SELECT id, status FROM user_coupons 
-- WHERE user_id = ? AND coupon_id = ? AND status = 'unused'

SELECT '创建索引: idx_user_coupons_user_coupon_status' AS action;

-- 检查索引是否已存在
SET @index_exists = (
    SELECT COUNT(*) 
    FROM information_schema.statistics 
    WHERE table_schema = 'emshop' 
      AND table_name = 'user_coupons' 
      AND index_name = 'idx_user_coupons_user_coupon_status'
);

-- 如果不存在则创建
SET @sql = IF(
    @index_exists > 0,
    'SELECT "索引已存在,跳过" AS result',
    'CREATE INDEX idx_user_coupons_user_coupon_status ON user_coupons(user_id, coupon_id, status)'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- 2. 订单ID索引
-- ============================================
-- 用于: 订单取消/退款时恢复优惠券
-- SELECT id, coupon_id FROM user_coupons WHERE order_id = ? AND status = 'used'

SELECT '创建索引: idx_user_coupons_order_id' AS action;

SET @index_exists = (
    SELECT COUNT(*) 
    FROM information_schema.statistics 
    WHERE table_schema = 'emshop' 
      AND table_name = 'user_coupons' 
      AND index_name = 'idx_user_coupons_order_id'
);

SET @sql = IF(
    @index_exists > 0,
    'SELECT "索引已存在,跳过" AS result',
    'CREATE INDEX idx_user_coupons_order_id ON user_coupons(order_id)'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- 3. 优惠券状态和ID组合索引
-- ============================================
-- 用于: 过期检查和统计查询
-- UPDATE user_coupons SET status = 'expired' WHERE status = 'unused' AND coupon_id IN (...)

SELECT '创建索引: idx_user_coupons_status_coupon' AS action;

SET @index_exists = (
    SELECT COUNT(*) 
    FROM information_schema.statistics 
    WHERE table_schema = 'emshop' 
      AND table_name = 'user_coupons' 
      AND index_name = 'idx_user_coupons_status_coupon'
);

SET @sql = IF(
    @index_exists > 0,
    'SELECT "索引已存在,跳过" AS result',
    'CREATE INDEX idx_user_coupons_status_coupon ON user_coupons(status, coupon_id)'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- 4. 使用时间索引
-- ============================================
-- 用于: 统计报表中的时间分布分析
-- SELECT DATE_FORMAT(used_at, '%Y-%m'), COUNT(*) FROM user_coupons WHERE used_at IS NOT NULL

SELECT '创建索引: idx_user_coupons_used_at' AS action;

SET @index_exists = (
    SELECT COUNT(*) 
    FROM information_schema.statistics 
    WHERE table_schema = 'emshop' 
      AND table_name = 'user_coupons' 
      AND index_name = 'idx_user_coupons_used_at'
);

SET @sql = IF(
    @index_exists > 0,
    'SELECT "索引已存在,跳过" AS result',
    'CREATE INDEX idx_user_coupons_used_at ON user_coupons(used_at)'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- 5. 优惠券代码唯一索引
-- ============================================
-- 注意: 这个索引应该在表创建时已经存在(UNIQUE约束)
-- 这里仅做验证

SELECT '验证唯一索引: coupons.code' AS action;

SET @unique_exists = (
    SELECT COUNT(*) 
    FROM information_schema.table_constraints 
    WHERE table_schema = 'emshop' 
      AND table_name = 'coupons' 
      AND constraint_name LIKE '%code%'
      AND constraint_type = 'UNIQUE'
);

SELECT IF(
    @unique_exists > 0,
    '✓ 优惠券代码唯一索引已存在',
    '⚠ 警告: 优惠券代码唯一索引不存在,建议手动添加'
) AS result;

-- ============================================
-- 6. 优惠券状态和过期时间索引
-- ============================================
-- 用于: 过期检查优化
-- SELECT * FROM coupons WHERE status = 'active' AND end_time < NOW()

SELECT '创建索引: idx_coupons_status_endtime' AS action;

SET @index_exists = (
    SELECT COUNT(*) 
    FROM information_schema.statistics 
    WHERE table_schema = 'emshop' 
      AND table_name = 'coupons' 
      AND index_name = 'idx_coupons_status_endtime'
);

SET @sql = IF(
    @index_exists > 0,
    'SELECT "索引已存在,跳过" AS result',
    'CREATE INDEX idx_coupons_status_endtime ON coupons(status, end_time)'
);

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- ============================================
-- 显示所有创建的索引
-- ============================================
SELECT '=' AS separator, '已创建的索引列表' AS info, '=' AS separator2;

SELECT 
    table_name AS 'Table',
    index_name AS 'Index Name',
    GROUP_CONCAT(column_name ORDER BY seq_in_index) AS 'Columns',
    index_type AS 'Type',
    CASE non_unique 
        WHEN 0 THEN 'UNIQUE' 
        ELSE 'INDEX' 
    END AS 'Constraint'
FROM information_schema.statistics
WHERE table_schema = 'emshop'
  AND table_name IN ('user_coupons', 'coupons')
  AND index_name != 'PRIMARY'
GROUP BY table_name, index_name, index_type, non_unique
ORDER BY table_name, index_name;

-- ============================================
-- 性能测试示例查询
-- ============================================
SELECT '=' AS separator, '性能测试示例' AS info, '=' AS separator2;

-- 测试查询1: 验证用户优惠券
EXPLAIN SELECT id, status 
FROM user_coupons 
WHERE user_id = 9 AND coupon_id = 10 AND status = 'unused';

-- 测试查询2: 订单关联优惠券
EXPLAIN SELECT id, coupon_id 
FROM user_coupons 
WHERE order_id = 234 AND status = 'used';

-- 测试查询3: 过期优惠券查询
EXPLAIN SELECT uc.id 
FROM user_coupons uc
JOIN coupons c ON uc.coupon_id = c.coupon_id
WHERE uc.status = 'unused' AND c.end_time < NOW();

-- ============================================
-- 索引大小统计
-- ============================================
SELECT '=' AS separator, '索引大小统计' AS info, '=' AS separator2;

SELECT 
    table_name AS 'Table',
    index_name AS 'Index',
    ROUND(stat_value * @@innodb_page_size / 1024 / 1024, 2) AS 'Size (MB)'
FROM mysql.innodb_index_stats
WHERE database_name = 'emshop'
  AND table_name IN ('user_coupons', 'coupons')
  AND stat_name = 'size'
ORDER BY stat_value DESC;

-- ============================================
-- 完成信息
-- ============================================
SET @end_time = NOW();
SELECT 
    CONCAT('开始时间: ', @start_time) AS start_time,
    CONCAT('结束时间: ', @end_time) AS end_time,
    CONCAT('耗时: ', TIMESTAMPDIFF(SECOND, @start_time, @end_time), ' 秒') AS duration;

SELECT '✓ 索引优化完成!' AS result;

-- ============================================
-- 使用建议
-- ============================================
-- 1. 定期使用 ANALYZE TABLE 更新索引统计信息:
--    ANALYZE TABLE user_coupons, coupons;
--
-- 2. 监控慢查询日志:
--    SET GLOBAL slow_query_log = 'ON';
--    SET GLOBAL long_query_time = 1;
--
-- 3. 使用 EXPLAIN 分析查询计划:
--    EXPLAIN SELECT ... FROM user_coupons WHERE ...;
--
-- 4. 定期检查索引使用情况:
--    SELECT * FROM sys.schema_unused_indexes WHERE object_schema = 'emshop';
-- ============================================
