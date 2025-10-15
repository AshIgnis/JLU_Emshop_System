-- 修复所有商品名称中的问号数据
-- 将包含问号的测试商品替换为真实商品名称

USE emshop;

-- 查看当前有问号的商品
SELECT COUNT(*) as '问号商品数量' FROM products WHERE name LIKE '%?%' OR description LIKE '%?%';

-- 更新所有问号商品为真实商品名称
UPDATE products SET 
    name = 'OPPO Find X7 Ultra',
    description = '天玑9300处理器，2K超感屏，5000万像素主摄'
WHERE product_id = 59;

UPDATE products SET 
    name = '华为 Mate 60 Pro',
    description = '卫星通信4.0，麒麟芯片，昆仑玻璃'
WHERE product_id = 60;

UPDATE products SET 
    name = '三星 Galaxy Z Fold5',
    description = '折叠屏手机，IPX8防水，骁龙8 Gen2'
WHERE product_id = 61;

UPDATE products SET 
    name = '华为 MateBook X Pro',
    description = '轻薄本，2.8K触控屏，i7-13700H处理器'
WHERE product_id = 63;

UPDATE products SET 
    name = '戴尔 XPS 13',
    description = '超薄笔记本，13英寸，i7处理器，16GB内存'
WHERE product_id = 64;

UPDATE products SET 
    name = '联想 ThinkPad X1',
    description = '商务笔记本，A级键盘，i7处理器'
WHERE product_id = 65;

UPDATE products SET 
    name = 'MacBook Pro 16英寸',
    description = 'M3 Max芯片，36GB内存，专业性能'
WHERE product_id = 66;

UPDATE products SET 
    name = 'Sony WH-1000XM5',
    description = '降噪耳机，30小时续航，8麦克风通话'
WHERE product_id = 68;

UPDATE products SET 
    name = '罗技 MX Master 3S',
    description = '无线鼠标，8K DPI传感器，静音按键'
WHERE product_id = 69;

UPDATE products SET 
    name = '深入理解计算机系统 第3版',
    description = 'CSAPP经典教材，CMU计算机系统课程教材'
WHERE product_id = 72;

UPDATE products SET 
    name = '算法导论 第4版',
    description = '算法经典，MIT算法课程教材，红宝书'
WHERE product_id = 73;

UPDATE products SET 
    name = 'C++ Primer 中文版 第5版',
    description = 'C++入门经典，全面覆盖C++11/14标准'
WHERE product_id = 74;

UPDATE products SET 
    name = '设计模式',
    description = 'Gang of Four经典，23种设计模式详解'
WHERE product_id = 75;

UPDATE products SET 
    name = 'Effective C++ 第3版',
    description = 'Scott Meyers著，55条改善程序建议'
WHERE product_id = 76;

UPDATE products SET 
    name = 'Java核心技术 第4版',
    description = 'Java开发必备，Java编程语言精粹'
WHERE product_id = 77;

UPDATE products SET 
    name = '重构 第2版',
    description = 'Martin Fowler著，JavaScript代码示例'
WHERE product_id = 78;

UPDATE products SET 
    name = '代码大全 第2版',
    description = '软件开发百科全书，Steve McConnell著'
WHERE product_id = 79;

UPDATE products SET 
    name = '东北大米',
    description = '五常大米，珍珠米，90%一等品'
WHERE product_id = 81;

UPDATE products SET 
    name = '新疆阿克苏苹果',
    description = '冰糖心苹果，脆甜多汁，新鲜直供'
WHERE product_id = 82;

UPDATE products SET 
    name = '海天酱油',
    description = '特级生抽，古法酿造，500ml'
WHERE product_id = 83;

UPDATE products SET 
    name = 'HM男士T恤',
    description = '纯棉基础款，多色可选'
WHERE product_id = 84;

UPDATE products SET 
    name = 'The North Face冲锋衣',
    description = '户外防水透气，Gore-Tex面料'
WHERE product_id = 85;

UPDATE products SET 
    name = 'Canada Goose羽绒服',
    description = '加拿大鹅，625蓬松度，极地保暖'
WHERE product_id = 86;

UPDATE products SET 
    name = 'Lululemon瑜伽裤',
    description = '高腰提臀，Nulu面料，四向弹力'
WHERE product_id = 87;

UPDATE products SET 
    name = '耐克 Air Max 气垫鞋',
    description = '可视气垫，缓震舒适，复古造型'
WHERE product_id = 88;

UPDATE products SET 
    name = '阿迪达斯 Ultra Boost 跑鞋',
    description = 'Boost中底，爆米花缓震，编织鞋面'
WHERE product_id = 89;

UPDATE products SET 
    name = '新百伦 574 复古鞋',
    description = '经典复古，ENCAP中底，麂皮材质'
WHERE product_id = 90;

UPDATE products SET 
    name = 'Jordan 1 篮球鞋',
    description = 'AJ1元年配色，经典高帮，皮革鞋面'
WHERE product_id = 91;

UPDATE products SET 
    name = '海尔冰箱双开门',
    description = '对开门冰箱，7级变频，风冷无霜'
WHERE product_id = 93;

UPDATE products SET 
    name = '美的空调变频',
    description = '1.5匹变频，一级能效，静音运行'
WHERE product_id = 94;

UPDATE products SET 
    name = '格力空调柜机',
    description = '3匹立式空调，客厅专用，静音节能'
WHERE product_id = 95;

UPDATE products SET 
    name = '西门子洗衣机',
    description = '10kg滚筒，变频电机，除菌洗'
WHERE product_id = 96;

UPDATE products SET 
    name = '九阳豆浆机 500ml',
    description = '破壁免滤，预约定时，不锈钢机身'
WHERE product_id = 97;

UPDATE products SET 
    name = '美的电饭煲 10kg',
    description = 'IH电磁加热，24小时预约，10L大容量'
WHERE product_id = 98;

UPDATE products SET 
    name = '小米扫地机器人',
    description = '激光导航，5000Pa吸力，自动集尘'
WHERE product_id = 99;

UPDATE products SET 
    name = '飞利浦空气炸锅',
    description = '无油烹饪，智能菜单，4.5L容量'
WHERE product_id = 100;

UPDATE products SET 
    name = '苏泊尔电饭煲',
    description = 'IH加热技术，智能预约，4L容量'
WHERE product_id = 101;

UPDATE products SET 
    name = '九阳破壁机',
    description = '多功能破壁，静音设计，1.3L容量'
WHERE product_id = 102;

UPDATE products SET 
    name = '戴森 V15 吸尘器',
    description = '无线吸尘器，LCD显示屏，激光探测'
WHERE product_id = 103;

UPDATE products SET 
    name = '松下剃须刀',
    description = '全身水洗，浮动刀头，4D贴面'
WHERE product_id = 104;

UPDATE products SET 
    name = 'Nintendo Switch OLED版',
    description = '7英寸OLED屏幕，64GB存储，增强音效'
WHERE product_id = 105;

UPDATE products SET 
    name = 'iPad Pro 12.9英寸 M2',
    description = 'Apple M2芯片，Liquid Retina XDR显示屏，支持Apple Pencil'
WHERE product_id = 106;

UPDATE products SET 
    name = 'Microsoft Surface Pro 9',
    description = '二合一平板电脑，第12代Intel酷睿i7，支持触控笔'
WHERE product_id = 107;

-- 确认修复结果
SELECT COUNT(*) as '修复后问号商品数量' FROM products WHERE name LIKE '%?%' OR description LIKE '%?%';

-- 显示所有商品名称（前50个）
SELECT product_id, name, price, stock_quantity FROM products ORDER BY product_id LIMIT 50;
