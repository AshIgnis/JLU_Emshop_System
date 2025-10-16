# 限购功能测试指南

## 测试环境准备

### 1. 确保数据库中有限购商品

运行SQL查询:
```sql
SELECT product_id, name, price, purchase_limit, purchase_limit_period, stock_quantity
FROM products 
WHERE purchase_limit > 0
ORDER BY purchase_limit_period, price DESC;
```

如果没有限购商品,运行初始化脚本:
```bash
mysql -u root -p emshop < cpp/init_purchase_limit_products.sql
```

### 2. 启动Java服务器

```bash
cd java
mvn exec:java@server
```

### 3. 启动Qt客户端

```bash
cd qtclient/build
.\src\emshop_qtclient.exe
```

## 测试场景

### 场景1: 添加限购商品到购物车 - 首次添加

**步骤**:
1. 登录系统 (user_id = 1)
2. 浏览商品,找到"iPhone 15 Pro Max 1TB"(限购2件/总计)
3. 添加1件到购物车

**预期结果**: ✅ 成功添加
**错误提示**: 无

---

### 场景2: 添加限购商品到购物车 - 超出限购

**前提**: 购物车中已有"iPhone 15 Pro Max"2件

**步骤**:
1. 尝试再添加1件"iPhone 15 Pro Max"到购物车

**预期结果**: ❌ 添加失败
**错误提示**: 
```
「iPhone 15 Pro Max 1TB」限购 2 件/总计
您总计已购买 0 件，购物车中已有 2 件
本次添加 1 件将超出限购
```

---

### 场景3: 修改购物车数量 - 超出限购

**前提**: 购物车中有"爱马仕Birkin"1件(限购1件/今日)

**步骤**:
1. 在购物车中将"爱马仕Birkin"数量修改为2件

**预期结果**: ❌ 修改失败
**错误提示**:
```
「爱马仕Birkin经典款手提包」限购 1 件/今日
您今日已购买 0 件
更新为 2 件将超出限购
```

---

### 场景4: 购买后再次添加 - 超出限购

**步骤**:
1. 购买"双十一特价茅台酒"3件(限购3件/总计)
2. 订单支付完成
3. 尝试再添加1件"双十一特价茅台酒"到购物车

**预期结果**: ❌ 添加失败
**错误提示**:
```
「双十一特价茅台酒」限购 3 件/总计
您总计已购买 3 件
本次添加 1 件将超出限购
```

---

### 场景5: 跨周期限购 - 每日限购

**步骤**:
1. 今天购买"爱马仕Kelly凯莉包"1件(限购1件/今日)
2. 今天再次尝试添加到购物车 - **应该失败**
3. 等到第二天,再次添加 - **应该成功**

**预期结果**: 
- 今天第二次添加: ❌ 失败
- 第二天添加: ✅ 成功

---

### 场景6: 退款后再次购买

**步骤**:
1. 购买"限量款AJ1球鞋"3件(限购3件/总计)
2. 申请退款并审批通过
3. 再次添加"限量款AJ1球鞋"3件到购物车

**预期结果**: ✅ 成功添加
**原因**: 退款后购买记录状态变为'refunded',不计入限购统计

---

## 检查数据库记录

### 查看用户购买记录
```sql
SELECT 
    upr.record_id,
    upr.user_id,
    upr.order_id,
    p.name AS product_name,
    upr.quantity,
    upr.purchase_time,
    upr.status,
    o.status AS order_status
FROM user_purchase_records upr
JOIN products p ON upr.product_id = p.product_id
LEFT JOIN orders o ON upr.order_id = o.order_id
WHERE upr.user_id = 1
ORDER BY upr.purchase_time DESC;
```

### 手动测试存储过程
```sql
-- 测试用户1购买商品1，数量2件
CALL check_user_purchase_limit(1, 1, 2, @can, @purchased, @limit, @period);
SELECT 
    @can AS can_purchase,
    @purchased AS purchased_count,
    @limit AS limit_count,
    @period AS limit_period;
```

### 查看购物车状态
```sql
SELECT 
    c.cart_id,
    c.user_id,
    p.name AS product_name,
    c.quantity,
    p.purchase_limit,
    p.purchase_limit_period,
    (SELECT COALESCE(SUM(quantity), 0) 
     FROM user_purchase_records 
     WHERE user_id = c.user_id 
       AND product_id = c.product_id 
       AND status = 'valid') AS purchased_count
FROM cart c
JOIN products p ON c.product_id = p.product_id
WHERE c.user_id = 1;
```

## 问题排查

### 问题1: 仍然可以超出限购添加商品

**检查项**:
1. ✅ C++ JNI库是否已重新编译
   ```bash
   cd cpp
   .\build_oop_jni.bat
   ```

2. ✅ .dll文件是否已复制到正确位置
   ```bash
   # 检查文件日期
   ls -l java/target/classes/emshop_native_oop.dll
   ```

3. ✅ Java服务器是否已重启并加载新DLL
   ```bash
   # 重启服务器
   cd java
   mvn clean compile
   mvn exec:java@server
   ```

4. ✅ 数据库存储过程是否存在
   ```sql
   SHOW PROCEDURE STATUS WHERE Name = 'check_user_purchase_limit';
   ```

---

### 问题2: 购买记录未正确统计

**检查项**:
1. 订单创建时是否记录了购买记录
   ```sql
   SELECT * FROM user_purchase_records 
   WHERE order_id = <your_order_id>;
   ```

2. 触发器是否正常工作
   ```sql
   SHOW TRIGGERS LIKE 'orders';
   ```

3. 购买记录状态是否正确
   ```sql
   -- 应该只统计status='valid'的记录
   SELECT * FROM user_purchase_records 
   WHERE user_id = 1 AND status != 'valid';
   ```

---

## 退款UI按钮颜色修复

### 修复内容
- **文件**: `qtclient/src/ui/tabs/AdminTab.cpp`
- **修改**: 拒绝退款按钮样式优化

**修改前**:
```cpp
background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
    stop:0 #ff6b6b, stop:1 #ee5a6f);  // 颜色较浅
font-size: 9pt;  // 字号较小
```

**修改后**:
```cpp
background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
    stop:0 #e74c3c, stop:1 #c0392b);  // 更深的红色
font-size: 10pt;  // 增大字号
font-weight: 700;  // 加粗字体
min-height: 36px;  // 增加按钮高度
```

### 测试方法
1. 重新编译Qt客户端
   ```bash
   cd qtclient/build
   cmake --build . -- -j
   ```

2. 启动Qt客户端,进入管理员界面
3. 查看订单列表中状态为"refunding"的订单
4. 检查"拒绝退款"按钮颜色是否更加清晰醒目

---

## 预期UI效果

### 拒绝退款按钮
- **背景色**: 深红色渐变 (#e74c3c → #c0392b)
- **悬停效果**: 更深的红色 + 白色边框
- **字体**: 白色、加粗、10pt
- **尺寸**: 最小100px宽 × 36px高
- **对比度**: 高对比度,易于识别

---

## 测试完成清单

- [ ] 场景1: 首次添加限购商品 - 成功
- [ ] 场景2: 超出限购添加 - 失败并提示
- [ ] 场景3: 修改购物车超出限购 - 失败并提示
- [ ] 场景4: 购买后再次添加 - 失败并提示
- [ ] 场景5: 跨周期限购 - 正确限制
- [ ] 场景6: 退款后再次购买 - 成功
- [ ] 退款UI按钮颜色 - 清晰醒目

---

**测试日期**: 2025年10月16日
**测试人员**: _______________
**测试结果**: _______________

