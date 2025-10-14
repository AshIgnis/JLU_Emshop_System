# 业务逻辑完善方案

> 创建日期: 2025-10-13  
> 目的: 完善四个核心业务逻辑

---

## 📋 需要完善的四个逻辑

### 1. 退款流程优化 ✨

**现状问题**:
- ❌ 用户申请退款后直接变为`refunded`状态
- ❌ 缺少`refunding`(退款中)状态
- ❌ 没有管理员审核环节
- ❌ 没有用户通知机制

**改进方案**:

```text
用户申请退款流程:
1. 用户申请退款 → 状态变为 refunding
2. 创建退款通知记录
3. 管理员审核退款申请
4. 审核通过 → 状态变为 refunded + 库存返还 + 通知用户
5. 审核拒绝 → 恢复订单状态 + 通知用户
```

**数据库改动**:

```sql
-- 1. 修改订单状态枚举,添加 refunding
ALTER TABLE orders MODIFY COLUMN status 
ENUM('pending', 'confirmed', 'paid', 'shipped', 'delivered', 
     'completed', 'cancelled', 'refunding', 'refunded') 
DEFAULT 'pending';

-- 2. 创建通知表
CREATE TABLE user_notifications (
    notification_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id BIGINT NOT NULL,
    type ENUM('order_update', 'refund_approved', 'refund_rejected', 
              'low_stock', 'coupon_available') NOT NULL,
    title VARCHAR(200) NOT NULL,
    content TEXT NOT NULL,
    related_id BIGINT COMMENT '关联ID(订单/优惠券等)',
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_user_id (user_id),
    INDEX idx_is_read (is_read),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='用户通知表';

-- 3. 添加退款申请表
CREATE TABLE refund_requests (
    refund_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    reason TEXT NOT NULL,
    refund_amount DECIMAL(12,2) NOT NULL,
    status ENUM('pending', 'approved', 'rejected') DEFAULT 'pending',
    admin_reply TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    processed_at TIMESTAMP NULL,
    processed_by BIGINT,
    INDEX idx_order_id (order_id),
    INDEX idx_user_id (user_id),
    INDEX idx_status (status),
    FOREIGN KEY (order_id) REFERENCES orders(order_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='退款申请表';
```

---

### 2. 库存管理优化 📦

**现状问题**:
- ✅ 订单创建时已扣减库存
- ❌ 没有低库存预警机制
- ❌ 没有库存变动日志
- ❌ 没有库存不足处理方案

**改进方案**:

```text
库存管理流程:
1. 订单创建 → 扣减库存 + 记录库存变动日志
2. 检查库存阈值 → 低于警戒线自动通知管理员
3. 订单取消/退款 → 返还库存 + 记录日志
4. 库存为0 → 商品状态自动改为 out_of_stock
5. 管理员补货 → 恢复商品状态 + 通知用户
```

**数据库改动**:

```sql
-- 1. 添加库存变动日志表
CREATE TABLE stock_logs (
    log_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    product_id BIGINT NOT NULL,
    change_quantity INT NOT NULL COMMENT '变动数量(正数为增加,负数为减少)',
    before_quantity INT NOT NULL,
    after_quantity INT NOT NULL,
    reason VARCHAR(100) NOT NULL COMMENT '变动原因',
    related_id BIGINT COMMENT '关联ID(订单/退款等)',
    operator_id BIGINT COMMENT '操作人ID',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_product_id (product_id),
    INDEX idx_created_at (created_at),
    FOREIGN KEY (product_id) REFERENCES products(product_id) ON DELETE CASCADE
) ENGINE=InnoDB COMMENT='库存变动日志表';

-- 2. 修改products表,添加库存预警字段(已有min_stock)
-- 确保min_stock字段被正确使用

-- 3. 创建低库存通知触发器
DELIMITER //
CREATE TRIGGER after_stock_update
AFTER UPDATE ON products
FOR EACH ROW
BEGIN
    IF NEW.stock_quantity <= NEW.min_stock AND NEW.stock_quantity > 0 THEN
        -- 插入管理员通知
        INSERT INTO user_notifications (user_id, type, title, content, related_id)
        SELECT user_id, 'low_stock', 
               CONCAT('商品库存不足: ', NEW.name),
               CONCAT('商品ID: ', NEW.product_id, ', 当前库存: ', NEW.stock_quantity, ', 预警值: ', NEW.min_stock),
               NEW.product_id
        FROM users WHERE role = 'admin';
    END IF;
    
    IF NEW.stock_quantity = 0 AND OLD.stock_quantity > 0 THEN
        -- 自动改为缺货状态
        UPDATE products SET status = 'out_of_stock' WHERE product_id = NEW.product_id;
    END IF;
END //
DELIMITER ;
```

---

### 3. 优惠券使用流程优化 🎫

**现状问题**:
- ❌ 用户不清楚优惠券如何使用
- ❌ 计算过程不透明
- ❌ 没有显示可用优惠券列表
- ❌ 没有显示优惠券减免金额明细

**改进方案**:

```text
优惠券使用流程:
1. 下单页面 → 显示用户可用优惠券列表
2. 选择优惠券 → 实时计算优惠金额
3. 显示订单明细:
   - 商品总价: ¥XXX
   - 运费: ¥XXX
   - 优惠券抵扣: -¥XXX (显示优惠券名称和code)
   - 实付金额: ¥XXX
4. 确认下单 → 使用优惠券 + 更新状态
```

**C++代码改进**:

```cpp
// 新增: 获取用户可用优惠券(针对订单金额)
json CouponService::getAvailableCouponsForOrder(long user_id, double order_amount);

// 新增: 计算优惠券优惠金额
json CouponService::calculateCouponDiscount(const string& coupon_code, 
                                             double order_amount);

// 改进: 创建订单时返回详细优惠信息
json OrderService::createOrderWithCouponDetails(...) {
    // ...
    json response_data;
    response_data["order_id"] = order_id;
    response_data["total_amount"] = total_amount;
    response_data["coupon_discount"] = coupon_discount;
    response_data["coupon_name"] = coupon_name;
    response_data["coupon_code"] = coupon_code;
    response_data["final_amount"] = final_amount;
    // ...
}
```

---

### 4. 优惠活动管理简化 🎉

**现状问题**:
- ❌ 创建优惠券需要手动插入数据库
- ❌ 参数理解门槛高(type, value, min_amount等)
- ❌ 没有图形化管理界面
- ❌ 没有活动模板

**改进方案**:

```text
优惠活动简化方案:
1. 预设活动模板:
   - 新用户优惠: 满XX减XX
   - 满减活动: 满XXX减XX
   - 打折活动: X折优惠
   - 包邮券: 免运费

2. 管理员界面:
   - 选择模板
   - 填写简单参数(金额、数量、时间)
   - 自动生成优惠券码
   - 一键发布

3. 批量操作:
   - 批量生成优惠券
   - 批量发放给用户
   - 批量失效
```

**新增C++接口**:

```cpp
// 新增: 创建优惠活动(简化版)
json CouponService::createCouponActivity(
    const string& activity_name,        // 活动名称
    const string& template_type,        // 模板类型: full_reduction, discount, free_shipping
    double discount_value,              // 优惠值
    double min_amount,                  // 最低消费
    int total_quantity,                 // 发行数量
    const string& start_time,           // 开始时间
    const string& end_time              // 结束时间
);

// 新增: 批量发放优惠券给用户
json CouponService::distributeCouponsToUsers(
    long coupon_id,
    const vector<long>& user_ids
);

// 新增: 获取活动模板列表
json CouponService::getCouponTemplates();
```

---

## 🛠️ 实施步骤

### 第一阶段: 数据库改造 (30分钟)

1. 执行SQL脚本创建新表
2. 修改订单状态枚举
3. 创建触发器

### 第二阶段: C++代码改进 (2小时)

1. **退款流程**:
   - 修改`requestRefund`为两阶段(申请+审核)
   - 新增`approveRefund`和`rejectRefund`
   - 新增通知创建函数

2. **库存管理**:
   - 修改库存扣减函数,添加日志记录
   - 新增低库存检查函数
   - 新增库存补充接口

3. **优惠券流程**:
   - 新增`getAvailableCouponsForOrder`
   - 新增`calculateCouponDiscount`
   - 修改订单创建返回值

4. **优惠活动**:
   - 新增`createCouponActivity`
   - 新增`distributeCouponsToUsers`
   - 新增模板管理

### 第三阶段: Java接口适配 (1小时)

1. 在`EmshopNativeInterface`添加新方法声明
2. 在`EmshopNettyServer`添加新路由
3. 测试接口调用

### 第四阶段: Qt客户端适配 (1小时)

1. 订单详情页显示优惠明细
2. 下单页显示可用优惠券
3. 添加退款申请界面
4. 管理员界面添加退款审核

---

## 📊 预期效果

### 1. 退款流程

- ✅ 用户体验提升: 清晰的退款状态和通知
- ✅ 管理规范: 管理员审核机制
- ✅ 可追溯性: 完整的退款记录

### 2. 库存管理

- ✅ 实时预警: 低库存自动通知
- ✅ 数据可靠: 完整的库存变动日志
- ✅ 业务连续性: 缺货自动下架

### 3. 优惠券体验

- ✅ 用户友好: 清晰的优惠计算过程
- ✅ 决策辅助: 可选优惠券列表
- ✅ 信息透明: 详细的优惠明细

### 4. 活动管理

- ✅ 操作简单: 模板化创建
- ✅ 效率提升: 批量操作
- ✅ 降低门槛: 无需理解技术细节

---

## 🎯 下一步行动

1. **立即执行**: 创建数据库脚本
2. **核心改进**: 修改C++服务代码
3. **测试验证**: 单元测试+集成测试
4. **界面适配**: Qt客户端UI改进

**预计总用时**: 4-5小时

---

**方案制定人**: GitHub Copilot Assistant  
**项目**: JLU Emshop System  
**日期**: 2025-10-13
