-- ====================================================================
-- 为所有用户推送限购商品通知
-- 日期：2025-10-15
-- ====================================================================

USE emshop;

-- 清理旧的限购商品通知（可选）
DELETE FROM notifications 
WHERE type = 'promotion' 
AND (title LIKE '%Limited%' OR title LIKE '%Exclusive%' OR title LIKE '%Digital%');

-- 为所有活跃用户创建限购商品通知
INSERT INTO notifications (user_id, type, title, content, related_id, status, created_at)
SELECT 
    u.user_id,
    'promotion' AS type,
    'Hot Sale - Limited Purchase Products' AS title,
    CONCAT(
        'Dear Customer,\n\n',
        'New limited purchase products are now available:\n\n',
        '[Daily Limit]\n',
        '* Hermes Birkin Classic - 280,000 (Daily Limit: 1)\n',
        '* Hermes Kelly - 180,000 (Daily Limit: 1)\n',
        '* Hermes Scarf Limited - 8,800 (Daily Limit: 1)\n\n',
        '[Total Limit]\n',
        '* iPhone 15 Pro Max 1TB - 10,999 (Total Limit: 2)\n',
        '* iPhone 15 Pro 512GB - 8,999 (Total Limit: 2)\n',
        '* Maotai Special - 1,499 (Limit: 3)\n',
        '* AJ1 Limited - 1,999 (Limit: 3)\n\n',
        '[Monthly Limit]\n',
        '* MacBook Pro 14 M3 Max - 22,999 (Monthly: 5)\n',
        '* iPad Pro 12.9 M2 - 8,999 (Monthly: 5)\n',
        '* AirPods Pro 2 - 1,899 (Monthly: 5)\n\n',
        '[Weekly Limit]\n',
        '* Dyson V15 - 4,999 (Weekly: 2)\n',
        '* Switch OLED - 2,399 (Weekly: 2)\n\n',
        'Notice:\n',
        '- Limited quantity, first come first served\n',
        '- Purchase limits vary by product\n',
        '- Quota restored on refund\n\n',
        'Shop Now'
    ) AS content,
    0 AS related_id,
    'unread' AS status,
    NOW() AS created_at
FROM users u
WHERE u.status = 'active';

-- 统计通知创建情况
SELECT 
    'Purchase limit notifications sent successfully' AS message,
    COUNT(*) AS total_notifications,
    COUNT(DISTINCT user_id) AS total_users
FROM notifications 
WHERE type = 'promotion' 
AND title LIKE '%Limited%'
AND DATE(created_at) = CURDATE();

-- 按商品类型创建个性化通知（VIP用户）
INSERT INTO notifications (user_id, type, title, content, related_id, status, created_at)
SELECT 
    u.user_id,
    'promotion' AS type,
    'VIP Exclusive - Luxury Limited Products' AS title,
    CONCAT(
        'Dear VIP Member,\n\n',
        'Exclusive luxury limited products:\n\n',
        '* Hermes Birkin Classic\n',
        '  Price: 280,000\n',
        '  Daily Limit: 1\n',
        '  Stock: Only 5 left\n\n',
        '* Hermes Kelly\n',
        '  Price: 180,000\n',
        '  Daily Limit: 1\n',
        '  Stock: Only 3 left\n\n',
        '* Hermes Scarf Limited\n',
        '  Price: 8,800\n',
        '  Daily Limit: 1\n',
        '  Stock: 20 available\n\n',
        'VIP Benefits:\n',
        '- Priority purchase\n',
        '- Exclusive service\n',
        '- Free delivery\n\n',
        'Shop Now'
    ) AS content,
    108 AS related_id,
    'unread' AS status,
    NOW() AS created_at
FROM users u
WHERE u.role = 'vip' AND u.status = 'active';

-- 为普通用户创建数码产品限购通知
INSERT INTO notifications (user_id, type, title, content, related_id, status, created_at)
SELECT 
    u.user_id,
    'promotion' AS type,
    'Digital Products Limited Sale' AS title,
    CONCAT(
        'Hello!\n\n',
        'New digital products with limited purchase:\n\n',
        '* iPhone 15 Pro Max 1TB\n',
        '  Price: 10,999 (Was: 11,999)\n',
        '  Limit: 2 per person\n',
        '  A17 Pro chip, titanium frame\n\n',
        '* MacBook Pro 14 M3 Max\n',
        '  Price: 22,999 (Was: 24,999)\n',
        '  Monthly Limit: 5\n',
        '  36GB RAM, professional tool\n\n',
        '* Switch OLED\n',
        '  Price: 2,399 (Was: 2,699)\n',
        '  Weekly Limit: 2\n',
        '  OLED screen upgrade\n\n',
        'Limited time offer!\n',
        'Shop Now'
    ) AS content,
    111 AS related_id,
    'unread' AS status,
    NOW() AS created_at
FROM users u
WHERE u.role = 'user' AND u.status = 'active';

-- 显示所有限购商品通知
SELECT 
    n.notification_id,
    u.username,
    u.role,
    n.title,
    LEFT(n.content, 100) AS content_preview,
    n.status,
    n.created_at
FROM notifications n
JOIN users u ON n.user_id = u.user_id
WHERE n.type = 'promotion' 
AND (n.title LIKE '%Limited%' OR n.title LIKE '%Exclusive%')
AND DATE(n.created_at) = CURDATE()
ORDER BY u.role DESC, n.created_at DESC
LIMIT 20;

-- 统计通知发送情况
SELECT 
    'Notification Statistics' AS report_type,
    u.role AS user_role,
    COUNT(n.notification_id) AS notification_count,
    COUNT(DISTINCT n.user_id) AS user_count
FROM notifications n
JOIN users u ON n.user_id = u.user_id
WHERE n.type = 'promotion' 
AND (n.title LIKE '%Limited%' OR n.title LIKE '%Exclusive%')
AND DATE(n.created_at) = CURDATE()
GROUP BY u.role;

SELECT 'All purchase limit notifications created successfully!' AS final_message;
