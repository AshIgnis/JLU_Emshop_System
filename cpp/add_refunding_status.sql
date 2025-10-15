-- 数据库升级脚本: 添加 refunding 订单状态
-- 日期: 2025-10-14
-- 用途: 支持退款申请中的订单状态

USE emshop;

-- 修改 orders 表的 status 字段,添加 'refunding' 状态
ALTER TABLE orders 
MODIFY COLUMN status ENUM(
    'pending',      -- 待确认
    'confirmed',    -- 已确认
    'paid',         -- 已支付
    'shipped',      -- 已发货
    'delivered',    -- 已送达
    'completed',    -- 已完成
    'cancelled',    -- 已取消
    'refunding',    -- 退款中 (新增)
    'refunded'      -- 已退款
) DEFAULT 'pending' COMMENT '订单状态';

-- 验证修改
SELECT COLUMN_TYPE 
FROM INFORMATION_SCHEMA.COLUMNS 
WHERE TABLE_SCHEMA = 'emshop' 
  AND TABLE_NAME = 'orders' 
  AND COLUMN_NAME = 'status';

-- 输出修改结果
SELECT '订单状态字段已成功更新，新增 refunding 状态' AS result;
