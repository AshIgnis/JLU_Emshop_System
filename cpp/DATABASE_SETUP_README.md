# JLU Emshop System - 数据库初始化指南

## 概述

本目录包含了JLU电商系统的数据库初始化脚本和相关工具，用于快速搭建和配置emshop数据库。

## 文件说明

### 主要文件

- `emshop_database_init.sql` - 主要的数据库初始化脚本
- `verify_database_connection.sql` - 数据库连接和数据验证脚本
- `init_emshop_database.bat` - Windows批处理自动化脚本
- `DATABASE_SETUP_README.md` - 本说明文件

### 数据库配置

- **服务器地址**: 127.0.0.1
- **端口**: 3306
- **数据库名**: emshop
- **字符集**: utf8mb4
- **排序规则**: utf8mb4_unicode_ci

## 快速开始

### 方法一：使用批处理脚本（推荐）

1. 确保MySQL服务正在运行
2. 双击运行 `init_emshop_database.bat`
3. 输入MySQL root用户密码
4. 等待初始化完成

### 方法二：手动执行SQL脚本

1. 打开MySQL命令行或客户端工具
2. 以root用户连接到MySQL服务器
3. 执行以下命令：

```sql
source /path/to/emshop_database_init.sql
```

或者在命令行中：

```bash
mysql -h127.0.0.1 -P3306 -uroot -p < emshop_database_init.sql
```

### 方法三：使用MySQL Workbench

1. 打开MySQL Workbench
2. 连接到本地MySQL服务器
3. 打开 `emshop_database_init.sql` 文件
4. 点击执行按钮

## 数据库结构

### 核心表结构

| 表名 | 说明 | 主要字段 |
|------|------|----------|
| users | 用户表 | user_id, username, password, role, status |
| categories | 商品分类表 | category_id, name, parent_id, level |
| products | 商品表 | product_id, name, price, stock_quantity, category_id |
| cart | 购物车表 | cart_id, user_id, product_id, quantity |
| orders | 订单表 | order_id, order_no, user_id, total_amount, status |
| order_items | 订单明细表 | item_id, order_id, product_id, quantity, subtotal |
| coupons | 优惠券表 | coupon_id, code, type, value |
| user_coupons | 用户优惠券表 | user_id, coupon_id, status |
| product_reviews | 商品评论表 | review_id, product_id, user_id, rating, content |
| user_addresses | 用户地址表 | address_id, user_id, receiver_name, detail_address |

### 默认测试数据

#### 用户账户
- **管理员**: username: `admin`, password: `123456`
- **测试用户**: username: `testuser`, password: `123456`

#### 商品分类
- 电子数码（手机通讯、电脑办公）
- 家用电器
- 服装鞋帽（男装、女装）
- 运动户外
- 图书音像
- 食品生鲜

#### 示例商品
- 华为 Mate 50 Pro
- 小米13 Ultra
- MacBook Air M2
- Nike Air Jordan 1
- 等10个测试商品

#### 优惠券
- WELCOME100: 新用户专享100元券
- SAVE100: 满1000减100
- DISCOUNT10: 全场9折
- FREESHIP: 包邮券

## 数据库特性

### 视图
- `v_product_details`: 商品详情视图
- `v_order_details`: 订单详情视图

### 存储过程
- `UpdateProductStock`: 更新商品库存
- `CalculateOrderTotal`: 计算订单总额

### 触发器
- `update_product_rating_after_review`: 自动更新商品评分

### 索引优化
- 为常用查询字段创建了单列和复合索引
- 全文索引用于商品搜索
- 外键约束确保数据一致性

## 验证安装

执行验证脚本检查数据库状态：

```bash
mysql -h127.0.0.1 -P3306 -uroot -p < verify_database_connection.sql
```

## 连接配置

### Java连接示例
```java
String url = "jdbc:mysql://127.0.0.1:3306/emshop?useSSL=false&serverTimezone=UTC&characterEncoding=UTF8";
String username = "root";
String password = "your_password";
```

### C++连接示例
```cpp
MYSQL *conn = mysql_init(NULL);
if (mysql_real_connect(conn, "127.0.0.1", "root", "password", "emshop", 3306, NULL, 0) == NULL) {
    fprintf(stderr, "%s\n", mysql_error(conn));
}
```

## 常见问题

### Q: 初始化失败，提示"Access denied"
A: 检查MySQL用户权限，确保root用户有创建数据库的权限

### Q: 中文数据显示乱码
A: 确保数据库字符集为utf8mb4，连接时指定正确的字符集

### Q: 外键约束错误
A: 检查数据完整性，确保父表记录存在

### Q: 存储过程创建失败
A: 检查MySQL版本是否支持存储过程，需要MySQL 5.0+

## 维护建议

1. **定期备份**: 建议每天备份数据库
2. **监控性能**: 关注慢查询和索引使用情况
3. **数据清理**: 定期清理过期的购物车和会话数据
4. **安全更新**: 及时更新用户密码加密方式
5. **容量规划**: 监控表大小，及时进行分区或归档

## 扩展说明

数据库设计支持以下扩展：
- 多级商品分类
- 商品规格/SKU管理
- 优惠券和促销活动
- 用户积分系统
- 商品评论和评分
- 多收货地址管理
- 订单状态流转
- 库存管理和预警

## 联系信息

如有问题，请联系开发团队或查看项目文档。

---
**JLU Emshop System Database Setup**  
*Version: 1.0*  
*Date: 2025-09-04*
