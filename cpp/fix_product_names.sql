-- 修复商品名称中的问号数据
-- 将包含问号的测试商品替换为真实商品名称

USE emshop;

-- 查看当前有问号的商品
SELECT product_id, name, description FROM products WHERE name LIKE '%?%';

-- 更新商品 ID 99-107 的名称为真实商品
UPDATE products SET 
    name = 'Lenovo ThinkPad X1 Carbon',
    description = '商务超薄笔记本，第10代i7处理器'
WHERE product_id = 99;

UPDATE products SET 
    name = 'Sony WH-1000XM5 降噪耳机',
    description = '旗舰级主动降噪，30小时续航'
WHERE product_id = 101;

UPDATE products SET 
    name = 'Nintendo Switch OLED版',
    description = '7英寸OLED屏幕，64GB存储'
WHERE product_id = 105;

UPDATE products SET 
    name = 'iPad Pro 12.9英寸 M2',
    description = 'Apple M2芯片，Liquid Retina XDR显示屏'
WHERE product_id = 106;

UPDATE products SET 
    name = 'Microsoft Surface Pro 9',
    description = '二合一平板电脑，第12代Intel酷睿i7'
WHERE product_id = 107;

-- 确认修改
SELECT product_id, name, description FROM products WHERE product_id IN (99, 101, 105, 106, 107);
