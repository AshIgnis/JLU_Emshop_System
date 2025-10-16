-- ====================================================================
-- 为所有用户推送限购商品通知（使用现有通知表）
-- 日期：2025-10-15
-- ====================================================================

USE emshop;

-- 方式1：为所有活跃用户创建通用限购通知
INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read)
SELECT 
    user_id,
    'system_notice',
    'Limited Purchase Alert',
    'Hot limited products available: iPhone 15 Pro Max (Limit:2), Hermes Birkin (Daily:1), MacBook Pro M3 (Monthly:5), Switch OLED (Weekly:2). Limited quantity, shop now!',
    0,
    FALSE
FROM users 
WHERE status = 'active';

-- 方式2：为VIP用户创建专属通知
INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read)
SELECT 
    user_id,
    'coupon_available',
    'VIP Exclusive Offer',
    'Dear VIP: Hermes luxury collection with special daily limit. Priority access and exclusive service. Check now!',
    108,
    FALSE
FROM users 
WHERE status = 'active' AND role = 'vip';

-- 验证通知创建
SELECT 
    'Notifications sent successfully!' AS status,
    COUNT(*) AS total_sent,
    COUNT(DISTINCT user_id) AS users_notified
FROM user_notifications 
WHERE type IN ('system_notice', 'coupon_available')
AND DATE(created_at) = CURDATE();

-- 显示最近创建的通知
SELECT 
    n.notification_id,
    u.username,
    u.role,
    n.title,
    LEFT(n.content, 80) AS preview,
    n.is_read
FROM user_notifications n
JOIN users u ON n.user_id = u.user_id
WHERE n.type IN ('system_notice', 'coupon_available')
AND DATE(n.created_at) = CURDATE()
ORDER BY n.notification_id DESC
LIMIT 10;
