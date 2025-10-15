-- ============================================
-- JLU Emshop System - 测试数据插入脚本
-- 创建日期: 2025-10-15
-- 说明: 添加丰富的测试数据，包括多种库存商品、用户、订单和通知
-- ============================================

USE emshop;

-- ============================================
-- 1. 插入测试用户（密码统一为 123456 的 MD5）
-- ============================================
-- 清理旧的测试数据（保留admin和testuser）
DELETE FROM user_notifications WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%');
DELETE FROM product_reviews WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%');
DELETE FROM order_items WHERE order_id IN (SELECT order_id FROM orders WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%'));
DELETE FROM orders WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%');
DELETE FROM cart WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%');
DELETE FROM user_addresses WHERE user_id IN (SELECT user_id FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%');
DELETE FROM users WHERE username LIKE 'testuser%' OR username LIKE 'buyer_%';

-- 清理测试商品（保留原有的10件初始商品，只删除新添加的）
DELETE FROM product_reviews WHERE product_id > 10;
DELETE FROM order_items WHERE product_id > 10;
DELETE FROM cart WHERE product_id > 10;
DELETE FROM products WHERE product_id > 10;

INSERT INTO users (username, password, phone, email, status, created_at) VALUES
('testuser1', 'e10adc3949ba59abbe56e057f20f883e', '13800138001', 'test1@example.com', 'active', NOW()),
('testuser2', 'e10adc3949ba59abbe56e057f20f883e', '13800138002', 'test2@example.com', 'active', NOW()),
('testuser3', 'e10adc3949ba59abbe56e057f20f883e', '13800138003', 'test3@example.com', 'active', NOW()),
('buyer_wang', 'e10adc3949ba59abbe56e057f20f883e', '13800138004', 'wang@example.com', 'active', NOW()),
('buyer_li', 'e10adc3949ba59abbe56e057f20f883e', '13800138005', 'li@example.com', 'active', NOW());

-- ============================================
-- 2. 插入多种商品（包含高库存和低库存）
-- ============================================

-- 电子数码 - 手机类（高库存）
INSERT INTO products (name, description, category_id, brand, price, original_price, stock_quantity, main_image, status, is_featured, is_new, created_at) VALUES
('iPhone 15 Pro Max', '苹果最新旗舰手机，A17芯片，钛合金边框，支持USB-C接口', 2, 'Apple', 9999.00, 10999.00, 150, '/images/iphone15.jpg', 'active', 1, 1, NOW()),
('小米14 Ultra', '徕卡光学镜头，骁龙8 Gen3旗舰芯片，2K曲面屏', 2, '小米', 5999.00, 6499.00, 200, '/images/xiaomi14.jpg', 'active', 1, 1, NOW()),
('华为Mate 60 Pro', '卫星通信，鸿蒙系统4.0，第二代昆仑玻璃', 2, '华为', 6999.00, 7499.00, 180, '/images/huawei_mate60.jpg', 'active', 1, 1, NOW());

-- 电子数码 - 手机类（低库存）
INSERT INTO products (name, description, category_id, brand, price, original_price, stock_quantity, main_image, status, is_featured, is_new, created_at) VALUES
('三星Galaxy Z Fold5', '折叠屏旗舰，IPX8防水，骁龙8 Gen2', 2, '三星', 13999.00, 14999.00, 5, '/images/galaxy_fold5.jpg', 'active', 1, 1, NOW()),
('OPPO Find X7 Ultra', '四主摄系统，哈苏影像，天玑9300', 2, 'OPPO', 5999.00, 6499.00, 8, '/images/oppo_findx7.jpg', 'active', 0, 1, NOW());

-- 电脑办公类（高库存）
INSERT INTO products (name, description, category_id, brand, price, original_price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('华为MateBook X Pro', '超轻薄商务本，2.8K触控屏，i7-13700H处理器', 3, '华为', 8999.00, 9999.00, 80, '/images/matebook.jpg', 'active', 1, NOW()),
('戴尔XPS 13', '超窄边框笔记本，13代酷睿i7，16GB内存', 3, 'Dell', 7999.00, 8999.00, 120, '/images/xps13.jpg', 'active', 0, NOW()),
('联想ThinkPad X1', '商务本经典，碳纤维A面，i7处理器', 3, 'Lenovo', 9999.00, 10999.00, 100, '/images/thinkpad_x1.jpg', 'active', 0, NOW());

-- 电脑办公类（低库存）
INSERT INTO products (name, description, category_id, brand, price, original_price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('MacBook Pro 16英寸', 'M3 Max芯片，36GB内存，专业创作首选', 3, 'Apple', 25999.00, 27999.00, 3, '/images/macbook_pro16.jpg', 'active', 1, NOW()),
('iPad Pro 12.9', 'M2芯片，Liquid视网膜XDR显示屏，2732x2048', 3, 'Apple', 8999.00, 9999.00, 6, '/images/ipad_pro.jpg', 'active', 1, NOW());

-- 数码配件（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('索尼WH-1000XM5', '主动降噪耳机，30小时续航，8麦克风降噪', 1, 'Sony', 2199.00, 300, '/images/sony_headphone.jpg', 'active', NOW()),
('罗技MX Master 3S', '无线鼠标，多设备切换，静音微动开关', 1, 'Logitech', 699.00, 500, '/images/mx_master.jpg', 'active', NOW());

-- 数码配件（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('AirPods Pro 2', '主动降噪，空间音频，MagSafe充电', 1, 'Apple', 1899.00, 8, '/images/airpods_pro.jpg', 'active', NOW()),
('Apple Watch Ultra 2', '钛金属表壳，运动探险智能手表，双频GPS', 1, 'Apple', 6299.00, 3, '/images/watch_ultra.jpg', 'active', NOW());

-- 图书音像类（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('深入理解计算机系统（第3版）', 'CSAPP经典教材，计算机专业必读，CMU权威著作', 9, '机械工业出版社', 139.00, 500, '/images/csapp.jpg', 'active', NOW()),
('算法导论（第4版）', '算法学习圣经，MIT经典教材，涵盖所有算法基础', 9, '机械工业出版社', 128.00, 400, '/images/algorithm.jpg', 'active', NOW()),
('C++ Primer中文版（第5版）', 'C++入门经典，全面讲解现代C++11/14特性', 9, '电子工业出版社', 118.00, 600, '/images/cpp_primer.jpg', 'active', NOW()),
('设计模式', 'Gang of Four经典著作，23种设计模式详解', 9, '机械工业出版社', 68.00, 350, '/images/design_pattern.jpg', 'active', NOW()),
('Effective C++（第3版）', 'Scott Meyers经典，55个改善程序与设计的做法', 9, '电子工业出版社', 78.00, 450, '/images/effective_cpp.jpg', 'active', NOW());

-- 图书音像类（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('Java编程思想（第4版）', 'Java程序员必读经典，深入理解Java核心', 9, '机械工业出版社', 108.00, 12, '/images/thinking_in_java.jpg', 'active', NOW()),
('重构（第2版）', 'Martin Fowler重构经典，JavaScript示例', 9, '人民邮电出版社', 98.00, 7, '/images/refactoring.jpg', 'active', NOW()),
('代码大全（第2版）', '软件构建实践指南，Steve McConnell巨著', 9, '电子工业出版社', 128.00, 9, '/images/code_complete.jpg', 'active', NOW()),
('Clean Code', 'Robert C. Martin代码整洁之道', 9, '人民邮电出版社', 89.00, 10, '/images/clean_code.jpg', 'active', NOW());

-- 服装鞋帽类（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('优衣库羽绒服', '轻薄保暖，多色可选，90%白鸭绒填充', 5, '优衣库', 499.00, 800, '/images/uniqlo_down.jpg', 'active', NOW()),
('阿迪达斯经典三条纹卫衣', '纯棉面料，休闲舒适，经典三条纹设计', 6, 'Adidas', 399.00, 600, '/images/adidas_hoodie.jpg', 'active', NOW()),
('李宁运动裤', '弹力面料，运动休闲两用，透气速干', 6, '李宁', 299.00, 700, '/images/lining_pants.jpg', 'active', NOW()),
('HM基础款T恤', '纯棉材质，多色可选，简约百搭', 5, 'H&M', 79.00, 1000, '/images/hm_tshirt.jpg', 'active', NOW());

-- 服装鞋帽类（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('The North Face冲锋衣', '防风防水，户外探险必备，Gore-Tex面料', 5, 'The North Face', 1899.00, 6, '/images/tnf_jacket.jpg', 'active', 1, NOW()),
('Canada Goose羽绒服', '极地保暖，奢侈品级，625蓬松度白鹅绒', 5, 'Canada Goose', 8999.00, 2, '/images/canada_goose.jpg', 'active', 1, NOW()),
('Lululemon瑜伽裤', '高弹力透气，运动时尚，Nulu面料', 7, 'Lululemon', 799.00, 10, '/images/lululemon.jpg', 'active', 1, NOW());

-- 运动户外类（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('耐克Air Max运动鞋', '气垫缓震，舒适透气，经典百搭款', 8, 'Nike', 699.00, 500, '/images/nike_airmax.jpg', 'active', NOW()),
('阿迪达斯Ultra Boost', '爆米花中底，能量回弹，跑步首选', 8, 'Adidas', 1299.00, 300, '/images/adidas_ultraboost.jpg', 'active', NOW()),
('新百伦574经典款', '复古休闲鞋，ENCAP中底，舒适耐磨', 8, 'New Balance', 599.00, 400, '/images/nb574.jpg', 'active', NOW());

-- 运动户外类（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('Jordan 1限量款', 'AJ1芝加哥配色，篮球文化经典', 8, 'Nike', 1599.00, 5, '/images/jordan1.jpg', 'active', 1, NOW()),
('Yeezy Boost 350', '椰子鞋V2版本，Primeknit鞋面', 8, 'Adidas', 1999.00, 3, '/images/yeezy350.jpg', 'active', 1, NOW());

-- 食品生鲜类（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('三只松鼠每日坚果', '混合坚果，营养健康，7种坚果组合', 10, '三只松鼠', 89.00, 1000, '/images/nuts.jpg', 'active', NOW()),
('良品铺子零食大礼包', '多种口味，满足挑剔味蕾，含肉干果干糖果', 10, '良品铺子', 199.00, 800, '/images/snack_pack.jpg', 'active', NOW()),
('五芳斋粽子礼盒', '传统美味，端午必备，嘉兴老字号', 10, '五芳斋', 128.00, 500, '/images/zongzi.jpg', 'active', NOW()),
('百草味蜜饯果干', '网红零食，酸甜可口，无添加防腐剂', 10, '百草味', 59.00, 900, '/images/dried_fruit.jpg', 'active', NOW());

-- 食品生鲜类（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('茅台飞天53度500ml', '国酒茅台，收藏送礼佳品，酱香型白酒', 10, '茅台', 2999.00, 4, '/images/maotai.jpg', 'active', 1, NOW()),
('五常稻花香大米10kg', '东北优质大米，香软可口，米中贵族', 10, '五常', 158.00, 15, '/images/rice.jpg', 'active', 0, NOW());

-- 家用电器类（高库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, created_at) VALUES
('小米扫地机器人', '智能路径规划，自动回充，5000Pa吸力', 4, '小米', 1499.00, 200, '/images/robot_vacuum.jpg', 'active', NOW()),
('戴森吹风机', '快速干发，护发黑科技，负离子技术', 4, 'Dyson', 2990.00, 100, '/images/dyson_dryer.jpg', 'active', NOW()),
('美的电饭煲', 'IH电磁加热，智能预约，4L容量', 4, '美的', 399.00, 300, '/images/midea_cooker.jpg', 'active', NOW()),
('九阳豆浆机', '破壁免滤，静音研磨，1.3L大容量', 4, '九阳', 299.00, 400, '/images/joyoung_maker.jpg', 'active', NOW());

-- 家用电器类（低库存）
INSERT INTO products (name, description, category_id, brand, price, stock_quantity, main_image, status, is_featured, created_at) VALUES
('戴森V15吸尘器', '激光探测灰尘，强劲吸力，LCD屏实时显示', 4, 'Dyson', 4990.00, 8, '/images/dyson_v15.jpg', 'active', 1, NOW()),
('松下按摩椅', '零重力设计，全身气囊按摩，4D机芯', 4, '松下', 19999.00, 5, '/images/massage_chair.jpg', 'active', 1, NOW());

-- ============================================
-- 3. 为测试用户添加收货地址
-- ============================================
INSERT INTO user_addresses (user_id, receiver_name, receiver_phone, province, city, district, detail_address, postal_code, is_default) 
SELECT user_id, username, phone, '吉林省', '长春市', '朝阳区', '人民大街5988号吉林大学前卫南区', '130012', TRUE
FROM users WHERE username IN ('testuser1', 'testuser2', 'testuser3', 'buyer_wang', 'buyer_li');

INSERT INTO user_addresses (user_id, receiver_name, receiver_phone, province, city, district, detail_address, postal_code, is_default) 
SELECT user_id, '张三', '13900139001', '北京市', '朝阳区', '望京', 'SOHO现代城3号楼1001室', '100102', FALSE
FROM users WHERE username = 'testuser1';

-- ============================================
-- 4. 为测试用户添加购物车（包含低库存商品）
-- ============================================
INSERT INTO cart (user_id, product_id, quantity, selected, created_at)
SELECT 
    u.user_id,
    p.product_id,
    CASE 
        WHEN p.stock_quantity > 100 THEN 2
        ELSE 1
    END as quantity,
    TRUE,
    NOW()
FROM users u
CROSS JOIN products p
WHERE u.username = 'testuser1' 
AND p.name IN ('iPhone 15 Pro Max', 'iPad Pro 12.9', 'C++ Primer中文版（第5版）', '重构（第2版）')
LIMIT 4;

INSERT INTO cart (user_id, product_id, quantity, selected, created_at)
SELECT 
    u.user_id,
    p.product_id,
    1,
    TRUE,
    NOW()
FROM users u
CROSS JOIN products p
WHERE u.username = 'testuser2' 
AND p.name IN ('小米14 Ultra', 'AirPods Pro 2', '优衣库羽绒服')
LIMIT 3;

-- ============================================
-- 5. 创建测试订单
-- ============================================
INSERT INTO orders (user_id, order_no, total_amount, payment_method, payment_status, order_status, address_id, remark, created_at)
SELECT 
    u.user_id,
    CONCAT('EM', DATE_FORMAT(NOW(), '%Y%m%d'), LPAD(FLOOR(RAND() * 100000), 5, '0')),
    11898.00,
    'alipay',
    'paid',
    'shipped',
    (SELECT address_id FROM user_addresses WHERE user_id = u.user_id AND is_default = TRUE LIMIT 1),
    '请尽快发货，谢谢',
    DATE_SUB(NOW(), INTERVAL 3 DAY)
FROM users u WHERE u.username = 'testuser1';

-- 为订单添加订单项
INSERT INTO order_items (order_id, product_id, product_name, price, quantity, subtotal)
SELECT 
    o.order_id,
    p.product_id,
    p.name,
    p.price,
    1,
    p.price
FROM orders o
CROSS JOIN products p
WHERE o.user_id = (SELECT user_id FROM users WHERE username = 'testuser1' LIMIT 1)
AND p.name IN ('iPhone 15 Pro Max', 'AirPods Pro 2')
AND o.order_status = 'shipped'
LIMIT 2;

-- ============================================
-- 6. 添加商品评论
-- ============================================
INSERT INTO product_reviews (user_id, product_id, order_id, rating, content, is_anonymous, review_status, created_at)
SELECT 
    u.user_id,
    p.product_id,
    NULL,
    5,
    '非常好用，物超所值！强烈推荐给大家！',
    FALSE,
    'approved',
    DATE_SUB(NOW(), INTERVAL 2 DAY)
FROM users u
CROSS JOIN products p
WHERE u.username IN ('testuser2', 'testuser3')
AND p.name IN ('小米14 Ultra', 'C++ Primer中文版（第5版）', '优衣库羽绒服')
LIMIT 6;

-- ============================================
-- 7. 添加用户通知
-- ============================================
INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read, created_at)
SELECT 
    u.user_id,
    'order_update',
    '订单发货通知',
    CONCAT('您的订单已发货，快递单号：SF', LPAD(FLOOR(RAND() * 1000000000000), 12, '0')),
    COALESCE((SELECT order_id FROM orders WHERE user_id = u.user_id LIMIT 1), 0),
    FALSE,
    DATE_SUB(NOW(), INTERVAL 1 DAY)
FROM users u
WHERE u.username IN ('testuser1', 'testuser2', 'testuser3');

INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read, created_at)
SELECT 
    u.user_id,
    'promotion',
    '限时优惠活动',
    '全场电子产品8折优惠，仅限今日！点击查看详情',
    0,
    FALSE,
    DATE_SUB(NOW(), INTERVAL 2 HOUR)
FROM users u
WHERE u.username IN ('testuser1', 'buyer_wang', 'buyer_li');

INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read, created_at)
SELECT 
    u.user_id,
    'system',
    '系统维护通知',
    '系统将于今晚23:00-01:00进行维护升级，请提前安排购物时间。',
    0,
    TRUE,
    DATE_SUB(NOW(), INTERVAL 5 DAY)
FROM users u;

-- ============================================
-- 8. 数据验证查询
-- ============================================
SELECT '=== 商品库存统计 ===' as info;
SELECT 
    c.name as '类别',
    COUNT(*) as '商品数量',
    SUM(CASE WHEN p.stock_quantity > 100 THEN 1 ELSE 0 END) as '高库存商品',
    SUM(CASE WHEN p.stock_quantity <= 15 THEN 1 ELSE 0 END) as '低库存商品',
    SUM(p.stock_quantity) as '总库存'
FROM products p
LEFT JOIN categories c ON p.category_id = c.category_id
WHERE p.status = 'active'
GROUP BY c.name;

SELECT '=== 用户统计 ===' as info;
SELECT COUNT(*) as '总用户数', COUNT(CASE WHEN status='active' THEN 1 END) as '活跃用户' FROM users;

SELECT '=== 通知统计 ===' as info;
SELECT 
    COUNT(*) as '总通知数',
    SUM(CASE WHEN is_read = FALSE THEN 1 ELSE 0 END) as '未读通知',
    SUM(CASE WHEN is_read = TRUE THEN 1 ELSE 0 END) as '已读通知'
FROM user_notifications;

SELECT '=== 购物车统计 ===' as info;
SELECT COUNT(*) as '购物车商品数', SUM(quantity) as '总商品件数' FROM cart;

SELECT '=== 订单统计 ===' as info;
SELECT COUNT(*) as '订单总数', SUM(total_amount) as '总金额' FROM orders;

-- 完成
SELECT '✅ 测试数据添加完成！' as result;
