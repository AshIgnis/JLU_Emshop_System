@echo off
chcp 65001 >nul
echo =====================================
echo   推送限购商品通知
echo =====================================
echo.

echo [1] 清理旧通知...
mysql -u root -pQuxc060122 emshop -e "DELETE FROM notifications WHERE type='promotion' AND title LIKE '%%Limited%%';" 2>nul
echo [√] 旧通知已清理
echo.

echo [2] 为所有活跃用户创建通知...
mysql -u root -pQuxc060122 emshop -e "INSERT INTO notifications (user_id, type, title, content, status) SELECT user_id, 'promotion', 'Limited Purchase Products', 'Hot limited products: iPhone 15 Pro Max (Limit: 2), Hermes Birkin (Daily: 1), MacBook Pro M3 (Monthly: 5). Shop now!', 'unread' FROM users WHERE status='active';" 2>nul
echo [√] 通用通知已创建
echo.

echo [3] 为VIP用户创建专属通知...
mysql -u root -pQuxc060122 emshop -e "INSERT INTO notifications (user_id, type, title, content, status) SELECT user_id, 'promotion', 'VIP Exclusive Limited Sale', 'Dear VIP: Hermes luxury products with daily limit 1. Priority purchase available. Shop now!', 'unread' FROM users WHERE status='active' AND role='vip';" 2>nul
echo [√] VIP通知已创建
echo.

echo [4] 统计通知创建情况...
mysql -u root -pQuxc060122 emshop -e "SELECT COUNT(*) AS total_notifications, COUNT(DISTINCT user_id) AS total_users FROM notifications WHERE type='promotion' AND DATE(created_at)=CURDATE();" 2>nul
echo.

echo [5] 显示部分通知内容...
mysql -u root -pQuxc060122 emshop -e "SELECT n.notification_id, u.username, u.role, n.title, n.status FROM notifications n JOIN users u ON n.user_id=u.user_id WHERE n.type='promotion' AND DATE(n.created_at)=CURDATE() LIMIT 10;" 2>nul
echo.

echo =====================================
echo   ✅ 通知推送完成！
echo =====================================
echo.
echo 用户登录后将在通知中心看到限购商品信息
echo.
pause
