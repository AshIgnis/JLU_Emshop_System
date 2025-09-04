-- ====================================================================
-- JLU Emshop System 数据库连接验证脚本
-- 用于验证数据库连接和初始化状态
-- 连接信息: 127.0.0.1:3306/emshop
-- ====================================================================

-- 检查数据库是否存在
SELECT SCHEMA_NAME as database_name 
FROM INFORMATION_SCHEMA.SCHEMATA 
WHERE SCHEMA_NAME = 'emshop';

-- 使用emshop数据库
USE emshop;

-- 检查所有表是否创建成功
SELECT 
    TABLE_NAME as table_name,
    TABLE_ROWS as row_count,
    CREATE_TIME as created_time
FROM INFORMATION_SCHEMA.TABLES 
WHERE TABLE_SCHEMA = 'emshop' 
ORDER BY TABLE_NAME;

-- 验证核心数据
SELECT 'Users' as table_name, COUNT(*) as count FROM users
UNION ALL
SELECT 'Categories' as table_name, COUNT(*) as count FROM categories
UNION ALL
SELECT 'Products' as table_name, COUNT(*) as count FROM products
UNION ALL
SELECT 'Coupons' as table_name, COUNT(*) as count FROM coupons;

-- 测试用户登录数据
SELECT 
    user_id,
    username,
    role,
    status,
    created_at
FROM users
WHERE username IN ('admin', 'testuser');

-- 测试商品数据
SELECT 
    product_id,
    name,
    brand,
    price,
    stock_quantity,
    status
FROM products 
LIMIT 5;

-- 验证外键关系
SELECT 
    p.name as product_name,
    c.name as category_name
FROM products p
JOIN categories c ON p.category_id = c.category_id
LIMIT 5;

-- 检查视图是否创建成功
SELECT TABLE_NAME as view_name
FROM INFORMATION_SCHEMA.VIEWS
WHERE TABLE_SCHEMA = 'emshop';

-- 检查存储过程是否创建成功
SELECT ROUTINE_NAME as procedure_name
FROM INFORMATION_SCHEMA.ROUTINES
WHERE ROUTINE_SCHEMA = 'emshop' AND ROUTINE_TYPE = 'PROCEDURE';

-- 检查触发器是否创建成功
SELECT TRIGGER_NAME as trigger_name
FROM INFORMATION_SCHEMA.TRIGGERS
WHERE TRIGGER_SCHEMA = 'emshop';

-- 数据库状态总览
SELECT 
    'Database Status' as info_type,
    'Ready' as status,
    NOW() as check_time
UNION ALL
SELECT 
    'Total Tables' as info_type,
    CAST(COUNT(*) as CHAR) as status,
    NOW() as check_time
FROM INFORMATION_SCHEMA.TABLES 
WHERE TABLE_SCHEMA = 'emshop'
UNION ALL
SELECT 
    'Connection' as info_type,
    '127.0.0.1:3306' as status,
    NOW() as check_time;
