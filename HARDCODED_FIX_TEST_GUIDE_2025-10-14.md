# 硬编码数据修复 - 测试指南

## 🎯 测试目标

验证所有硬编码数据已成功替换为数据库实时数据，确保数据的一致性和实时性。

---

## 🚀 启动服务器

```powershell
cd d:\codehome\jlu\JLU_Emshop_System\java
mvn exec:java@server
```

等待服务器启动成功后，使用Qt客户端或测试脚本进行测试。

---

## 📝 测试场景

### ✅ 场景1: 促销活动实时更新

**目的**: 验证创建优惠券后列表立即刷新

**步骤**:
1. 打开Qt客户端，管理员账号登录
2. 进入"促销管理"标签页
3. 点击"刷新"按钮，记录当前优惠券数量
4. 创建新优惠券:
   - 活动名称: 测试优惠券2025
   - 代码: TEST2025
   - 类型: 固定金额
   - 金额: 50元
   - 最低消费: 200元
5. 观察界面是否自动刷新显示新优惠券
6. 手动点击"刷新"按钮验证

**预期结果**:
- ✅ 创建成功后界面自动刷新
- ✅ 新优惠券出现在列表中
- ✅ 优惠券信息正确显示

---

### ✅ 场景2: 支付方式验证

**目的**: 验证支付方式从配置读取

**测试命令**:
```
GET_PAYMENT_METHODS
VALIDATE_PAYMENT alipay 100.0 {}
VALIDATE_PAYMENT wechat 500.0 {}
VALIDATE_PAYMENT invalid_method 100.0 {}
```

**预期结果**:
```json
// GET_PAYMENT_METHODS 返回
{
  "success": true,
  "data": {
    "methods": [
      {"code": "alipay", "name": "支付宝", "enabled": true, "fee_rate": 0.006},
      {"code": "wechat", "name": "微信支付", "enabled": true, "fee_rate": 0.006},
      ...
    ]
  }
}

// VALIDATE_PAYMENT alipay 成功
{
  "success": true,
  "message": "支付方式验证通过",
  "data": {"method": "alipay", "fee_rate": 0.0060}
}

// VALIDATE_PAYMENT invalid_method 失败
{
  "success": false,
  "message": "不支持的支付方式或支付方式已禁用"
}
```

---

### ✅ 场景3: 购物车折扣计算

**目的**: 验证使用真实购物车数据计算折扣

**前置条件**:
1. 用户ID=1已登录
2. 购物车中有商品
3. 数据库中有可用优惠券

**测试步骤**:
1. 添加商品到购物车:
   ```
   ADD_TO_CART 1 101 2
   ADD_TO_CART 1 102 1
   ```

2. 查看购物车摘要:
   ```
   GET_CART_SUMMARY 1
   ```
   记录总价

3. 不使用优惠码计算折扣:
   ```
   CALCULATE_CART_DISCOUNT 1 
   ```

4. 使用优惠码计算折扣:
   ```
   CALCULATE_CART_DISCOUNT 1 TEST2025
   ```

**预期结果**:
- ✅ 总价与购物车摘要一致
- ✅ 无优惠码时折扣为0
- ✅ 有优惠码且满足条件时正确计算折扣
- ✅ 不满足最低消费时不应用折扣

---

### ✅ 场景4: 批量折扣计算

**目的**: 验证使用真实商品价格计算批量折扣

**测试命令**:
```
APPLY_BULK_DISCOUNT 101 1   # 1件商品，无折扣
APPLY_BULK_DISCOUNT 101 3   # 3件商品，5%折扣
APPLY_BULK_DISCOUNT 101 5   # 5件商品，10%折扣
APPLY_BULK_DISCOUNT 101 10  # 10件商品，15%折扣
```

**验证点**:
1. 先查询商品详情获取单价:
   ```
   GET_PRODUCT_DETAIL 101
   ```
   记录单价 `P`

2. 验证计算结果:
   - 1件: `total = P * 1`, `discount = 0`
   - 3件: `total = P * 3`, `discount = P * 3 * 0.05`
   - 5件: `total = P * 5`, `discount = P * 5 * 0.10`
   - 10件: `total = P * 10`, `discount = P * 10 * 0.15`

**预期结果**:
- ✅ 单价与商品详情一致
- ✅ 折扣率按数量正确应用
- ✅ 计算结果准确

---

### ✅ 场景5: 会员折扣检查

**目的**: 验证根据真实用户数据动态计算会员等级

**测试用户**:
- 普通用户 (ID=1): 积分<5000
- 黄金会员 (ID=2): 积分>=5000
- VIP会员 (ID=3): 积分>=10000 或 管理员

**测试命令**:
```
CHECK_MEMBERSHIP_DISCOUNT 1
CHECK_MEMBERSHIP_DISCOUNT 2
CHECK_MEMBERSHIP_DISCOUNT 3
```

**预期结果**:
```json
// 普通用户
{
  "member_level": "REGULAR",
  "discount_rate": 0.0,
  "benefits": "普通会员,满299免运费",
  "points_balance": <真实积分>
}

// 黄金会员
{
  "member_level": "GOLD",
  "discount_rate": 0.05,
  "benefits": "黄金会员95折,免费配送,优先发货",
  "points_balance": <真实积分 >= 5000>
}

// VIP会员/管理员
{
  "member_level": "VIP",
  "discount_rate": 0.10 或 0.15,
  "benefits": "VIP专享...",
  "points_balance": <真实积分>
}
```

---

### ✅ 场景6: 系统状态查询

**目的**: 验证显示真实系统运行状态

**测试命令**:
```
GET_SYSTEM_STATUS
```

**验证点**:
1. `active_sessions`: 应等于当前连接的客户端数量
2. `memory_usage`: 应显示真实内存使用情况（单位MB）
3. `database_status`: 应为"已连接"（如果数据库正常）
4. `last_updated`: 应为当前时间
5. `available_processors`: 应为系统真实CPU核心数

**预期结果**:
- ✅ 所有数据都是实时的
- ✅ 断开客户端后 `active_sessions` 减少
- ✅ 时间戳每次查询都更新

---

## 🔍 数据一致性测试

### 测试流程

1. **创建优惠券**
   ```
   CREATE_PROMOTION {"name":"周末特惠","code":"WEEKEND50","discount_type":"fixed_amount","discount_value":50,"min_amount":300,...}
   ```

2. **立即查询** (应该能看到新创建的)
   ```
   GET_ACTIVE_PROMOTIONS
   ```

3. **使用优惠券**
   ```
   CALCULATE_CART_DISCOUNT 1 WEEKEND50
   ```

4. **修改用户积分** (通过其他方式)
   
5. **重新查询会员等级** (应反映新积分)
   ```
   CHECK_MEMBERSHIP_DISCOUNT 1
   ```

### 一致性要求
- ✅ 任何数据修改后立即可查询到
- ✅ 不同接口返回的相同数据一致
- ✅ 无缓存导致的数据延迟

---

## 🐛 已知问题和限制

### 1. 支付方式配置
- **当前状态**: 支付方式仍在代码中配置
- **后续改进**: 创建 `payment_methods` 表存储配置
- **影响**: 修改支付方式需要重新编译

### 2. 批量折扣规则
- **当前状态**: 折扣规则硬编码在 `applyBulkDiscount` 方法中
- **后续改进**: 创建 `bulk_discount_rules` 表
- **影响**: 修改折扣规则需要重新编译

### 3. 会员等级规则
- **当前状态**: 等级划分标准硬编码
- **后续改进**: 创建 `membership_levels` 表
- **影响**: 修改等级标准需要重新编译

---

## 📊 测试检查清单

完成以下测试项后打勾:

- [ ] 场景1: 促销活动实时更新
- [ ] 场景2: 支付方式验证
- [ ] 场景3: 购物车折扣计算
- [ ] 场景4: 批量折扣计算
- [ ] 场景5: 会员折扣检查
- [ ] 场景6: 系统状态查询
- [ ] 数据一致性测试
- [ ] 并发创建和查询测试
- [ ] 异常情况测试（无效优惠码、空购物车等）

---

## 🎉 测试完成标准

所有测试场景通过，满足以下条件:

1. ✅ 数据创建后立即可查询
2. ✅ 查询结果与数据库一致
3. ✅ 系统状态显示真实运行信息
4. ✅ 无硬编码数据残留
5. ✅ 错误处理正确（返回合理错误消息）
6. ✅ 日志记录完整

---

## 📞 问题反馈

如果测试中发现问题，请记录:
- 测试场景
- 执行的命令
- 预期结果
- 实际结果
- 错误日志（如有）

保存到 `TEST_ISSUES_2025-10-14.md` 文件中。
