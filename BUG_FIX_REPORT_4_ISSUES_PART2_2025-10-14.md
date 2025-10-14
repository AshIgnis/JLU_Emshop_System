# 4个新问题修复报告 - 2025-10-14 19:31

**修复时间**: 2025年10月14日 19:31  
**修复范围**: 促销创建JSON错误、优惠券轮询、退款审批、通知已读

---

## 问题1: 促销创建JSON解析失败 ✅

### 错误日志
```
Failed to parse CREATE_PROMOTION JSON - error=Unexpected character ('\' (code 92))
JsonParseException: Unexpected character ('\' (code 92)): was expecting double-quote
Source: "{\"code\":\"5\",\"discount_type\":\"fixed_amount\",...}"
```

### 根本原因

**JSON被双重转义**:
1. Qt客户端使用`quoteForCommand()`包装JSON:
   ```cpp
   QString quoteForCommand(const QString &value) {
       QString escaped = value;
       escaped.replace("\\", "\\\\");   // 反斜杠转义
       escaped.replace('"', "\\\"");    // 引号转义
       return QStringLiteral("\"%1\"").arg(escaped);  // 添加外层引号
   }
   ```

2. JSON字符串 `{"code":"5",...}` 被转义为 `"{\"code\":\"5\",...}"`

3. Jackson无法解析这种转义后的JSON

### 修复方案

**在Java端反转义JSON字符串**:

```java
// EmshopNettyServer.java line 1108-1123
case "CREATE_PROMOTION":
    if (parts.length >= 2) {
        try {
            String jsonStr = parts[1];
            // Qt的quoteForCommand()会添加外层引号并转义内部引号,需要先处理
            if (jsonStr.startsWith("\"") && jsonStr.endsWith("\"")) {
                jsonStr = jsonStr.substring(1, jsonStr.length() - 1); // 移除外层引号
                jsonStr = jsonStr.replace("\\\"", "\""); // 反转义引号
                jsonStr = jsonStr.replace("\\\\", "\\"); // 反转义反斜杠
            }
            com.fasterxml.jackson.databind.ObjectMapper mapper = new com.fasterxml.jackson.databind.ObjectMapper();
            com.fasterxml.jackson.databind.JsonNode jsonObj = mapper.readTree(jsonStr);
            // ... 解析JSON字段并调用createCouponActivity
        }
    }
```

**文件**: `java/src/main/java/emshop/EmshopNettyServer.java` 行1108-1123  
**编译状态**: ✅ 已编译成功 (19:31)

---

## 问题2: 优惠券仍在轮询 ✅

### 问题日志
```
2025-10-14 19:20:16 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
2025-10-14 19:20:24 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
2025-10-14 19:20:32 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
(每8秒一次)
```

### 根本原因

**CartTab中有8秒定时器调用GET_USER_COUPONS**:

```cpp
// CartTab.cpp line 325-333
m_refreshTimer = new QTimer(this);
m_refreshTimer->setInterval(8000);  // 8秒定时器
connect(m_refreshTimer, &QTimer::timeout, this, [this]{
    if (!m_loggedIn) {
        return;
    }
    refreshCart();
    refreshUserCoupons();  // ⬅️ 每8秒调用一次!
});
```

**为什么CouponsTab修复后仍有问题**:
- 18:53修复的是`CouponsTab::handleSessionChanged()`
- 但`CartTab::refreshUserCoupons()`仍在定时器中每8秒调用一次
- 这是一个**独立的轮询源**!

### 修复方案

**移除定时器中的优惠券刷新**:

```cpp
// CartTab.cpp line 325-333 (修复后)
m_refreshTimer = new QTimer(this);
m_refreshTimer->setInterval(8000);
connect(m_refreshTimer, &QTimer::timeout, this, [this]{
    if (!m_loggedIn) {
        return;
    }
    refreshCart();
    // ✅ 不要每8秒都刷新优惠券,只在需要时手动刷新
    // refreshUserCoupons();
});
```

**保留的调用时机**:
- 用户登录时 (`handleSessionChanged()`)
- 用户点击刷新按钮时

**文件**: `qtclient/src/ui/tabs/CartTab.cpp` 行325-333  
**编译状态**: ✅ 已编译成功 (19:31)

---

## 问题3: 管理员无法处理refunding订单 ✅

### 问题描述
- 用户申请退款后,订单状态变为`refunding`
- 管理员在订单列表中只能看到"改状态"、"详情"、"退款"按钮
- 没有"审批退款"和"拒绝退款"的按钮
- 点击"退款"按钮无反应(因为订单已经在退款流程中)

### 根本原因

**AdminTab未实现退款审批功能**:
- 后端已有`APPROVE_REFUND`命令
- 但Qt客户端未实现UI和调用逻辑
- 管理员无法审批/拒绝用户的退款申请

### 修复方案

#### 1. 根据订单状态动态显示按钮

```cpp
// AdminTab.cpp line 705-731
QString orderStatus = o.value("status").toString();
if (orderStatus == "refunding") {
    // 退款申请中:显示审批/拒绝按钮
    auto *btnApprove = new QPushButton(tr("审批退款"), ops);
    auto *btnReject = new QPushButton(tr("拒绝退款"), ops);
    opsLayout->addWidget(btnStatus); opsLayout->addWidget(btnDetail);
    opsLayout->addWidget(btnApprove); opsLayout->addWidget(btnReject);
    connect(btnApprove, &QPushButton::clicked, this, [this, orderId]{ 
        approveRefund(orderId, true); 
    });
    connect(btnReject, &QPushButton::clicked, this, [this, orderId]{ 
        approveRefund(orderId, false); 
    });
} else {
    // 其他状态:显示普通退款按钮
    auto *btnRefund = new QPushButton(tr("退款"), ops);
    opsLayout->addWidget(btnStatus); opsLayout->addWidget(btnDetail); 
    opsLayout->addWidget(btnRefund);
    connect(btnRefund, &QPushButton::clicked, this, [this, orderId]{ 
        refundOrder(orderId); 
    });
}
```

#### 2. 实现approveRefund函数

```cpp
// AdminTab.cpp line 770-813
void AdminTab::approveRefund(qlonglong orderId, bool approve)
{
    // 1. 先获取该订单的refund_id
    QString getCmd = QString("GET_USER_REFUND_REQUESTS %1").arg(orderId);
    sendCommand(getCmd, [this, orderId, approve](const QJsonDocument &doc){
        // 2. 从返回结果中查找该订单的退款申请
        QJsonArray refunds = JsonUtils::extract(doc, "data.refunds").toArray();
        if (refunds.isEmpty()) {
            refunds = JsonUtils::extract(doc, "data").toArray();
        }
        
        long refundId = 0;
        for (const QJsonValue &val : refunds) {
            QJsonObject refund = val.toObject();
            if (refund.value("order_id").toVariant().toLongLong() == orderId) {
                refundId = refund.value("refund_id").toVariant().toLongLong();
                break;
            }
        }
        
        if (refundId == 0) {
            emit statusMessage(tr("未找到该订单的退款申请"), false);
            return;
        }
        
        // 3. 调用审批接口
        QString approveStr = approve ? "true" : "false";
        QString adminReply = approve ? "管理员已同意退款" : "管理员已拒绝退款";
        QString cmd = QString("APPROVE_REFUND %1 %2 %3")
                           .arg(refundId).arg(approveStr).arg(adminReply);
        sendCommand(cmd, [this, approve](const QJsonDocument&){
            emit statusMessage(approve ? tr("退款已审批通过") : tr("退款已拒绝"), true);
            refreshAllOrders();
        }, approve ? tr("审批退款") : tr("拒绝退款"));
    }, tr("获取退款信息"));
}
```

**后端API** (已存在):
```java
case "APPROVE_REFUND":  // EmshopNettyServer.java line 1016
    // refund_id approve admin_reply
    long refundId = Long.parseLong(parts[1]);
    boolean approve = Boolean.parseBoolean(parts[2]);
    String adminReply = parts[3];
    return EmshopNativeInterface.approveRefund(refundId, userId, approve, adminReply);
```

**文件**: 
- `qtclient/src/ui/tabs/AdminTab.cpp` 行705-731, 770-813
- `qtclient/src/ui/tabs/AdminTab.h` 行46

**编译状态**: ✅ 已编译成功 (19:31)

---

## 问题4: 通知标记已读仍失败 ⏳

### 问题描述
- 用户点击"标记已读"按钮
- 通知仍然显示"未读"状态
- 切换到"仅未读"时通知消失(说明数据库已更新)

### 代码检查结果

```cpp
// NotificationsTab.cpp line 126-129
QString command = QStringLiteral("MARK_NOTIFICATION_READ %1").arg(notificationId);
sendNotificationCommand(command, tr("标记已读"), [this](const QJsonDocument &doc) {
    Q_UNUSED(doc);
    refreshNotifications();  // ✅ 已经调用刷新
});
```

**代码逻辑正确**,已经在回调中调用了`refreshNotifications()`。

### 可能原因

1. **后端markNotificationRead返回失败但UI未显示错误**
2. **数据库更新失败但未抛出异常**
3. **刷新时机问题**:UI刷新在数据库更新前执行
4. **Qt事件循环问题**:信号槽未正确执行

### 调试步骤

**需要查看后端日志**:
```
# 标记已读时应该看到:
[INFO] [OrderService] 通知已标记为已读: notification_id=X

# 如果看到错误:
[ERROR] [OrderService] ...
```

**需要检查数据库**:
```sql
SELECT notification_id, is_read FROM user_notifications WHERE user_id=1;
-- is_read应该从0变为1
```

### 临时解决方案

如果刷新时机有问题,可以添加延迟:

```cpp
// 备选方案
sendNotificationCommand(command, tr("标记已读"), [this](const QJsonDocument &doc) {
    if (JsonUtils::isSuccess(doc)) {
        QTimer::singleShot(200, this, &NotificationsTab::refreshNotifications);
    } else {
        emit statusMessage(JsonUtils::message(doc), false);
    }
});
```

**状态**: ⏳ **需要用户提供后端日志以确定根本原因**

---

## 编译结果总结

### Java后端
```
[INFO] BUILD SUCCESS
[INFO] Total time:  2.586 s
[INFO] Finished at: 2025-10-14T19:31:14+08:00
修复: 问题1(CREATE_PROMOTION JSON反转义)
```

### Qt客户端
```
[100%] Built target emshop_qtclient
文件: D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src\emshop_qtclient.exe
时间: 2025-10-14 19:31:XX
修复: 问题2(优惠券轮询), 问题3(退款审批)
警告: 1个 (multi-character constant, 非关键)
```

---

## 完整测试步骤

### 步骤0: 必须重启! ⚠️

```powershell
# 1. 重启后端服务器
cd D:\codehome\jlu\JLU_Emshop_System\java
mvn exec:java@server

# 2. 关闭旧Qt客户端,启动新版本
cd D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
.\emshop_qtclient.exe
```

### 步骤1: 测试问题1 - 促销创建

1. 用admin账号登录
2. 管理员标签页 → 促销/优惠券
3. 填写:
   - 名称: `十一大庆`
   - 代码: `5`
   - 类型: 固定减免
   - 折扣值: 50元
   - 最低消费: 0元
4. 点击"创建促销/优惠券"
5. **预期**:
   - ✅ 显示"促销活动创建成功"
   - ✅ 后端日志无JsonParseException错误
   - ✅ 可以在促销列表中看到新创建的促销

### 步骤2: 测试问题2 - 优惠券轮询停止

1. 用普通用户登录
2. 切换到"购物车"标签页
3. 等待30秒,观察后端日志
4. **预期**:
   - ✅ **不再**看到每8秒一次的"获取用户优惠券"日志
   - ✅ 只在登录时看到一次查询
   - ✅ 定时器只刷新购物车,不刷新优惠券

### 步骤3: 测试问题3 - 管理员审批退款

#### 准备:用户申请退款
1. 用普通用户登录
2. 订单标签页 → 选择status='paid'的订单
3. 点击"申请退款" → 输入原因 → 提交
4. 订单状态应变为"refunding"

#### 测试:管理员审批
1. 切换到admin账号
2. 管理员标签页 → 订单管理
3. 找到status='refunding'的订单
4. **验证UI**:
   - ✅ 应该看到"审批退款"和"拒绝退款"按钮
   - ✅ 没有普通的"退款"按钮
5. 点击"审批退款"
6. **预期**:
   - ✅ 显示"退款已审批通过"
   - ✅ 订单状态变为"refunded"
   - ✅ 后端日志显示APPROVE_REFUND调用成功

#### 测试:管理员拒绝
1. 找到另一个refunding订单
2. 点击"拒绝退款"
3. **预期**:
   - ✅ 显示"退款已拒绝"
   - ✅ 订单状态可能变为"paid"(取决于后端逻辑)

### 步骤4: 测试问题4 - 通知已读

1. 用普通用户登录
2. 通知标签页 → 选择未读通知
3. 点击"标记已读"
4. **观察**:
   - 检查通知状态是否立即变为"已读"
   - 如果仍失败,查看后端日志中的错误信息
   - 切换到"仅未读"筛选,验证通知是否消失

**如果仍失败,请提供**:
- 后端日志中的错误信息
- 数据库`user_notifications`表的is_read字段值
- 前端是否显示任何错误消息

---

## 问题状态总结

| 问题 | 根本原因 | 修复状态 | 需要操作 |
|------|---------|---------|---------|
| 1. 促销创建JSON错误 | quoteForCommand转义JSON | ✅ 已修复 | 重启后端 |
| 2. 优惠券轮询 | CartTab定时器每8秒调用 | ✅ 已修复 | 重启客户端 |
| 3. 管理员无法审批退款 | 缺少审批UI和逻辑 | ✅ 已修复 | 重启客户端 |
| 4. 通知标记已读失败 | 原因待查 | ⏳ 需要后端日志 | 提供日志 |

---

## 技术细节总结

### 学到的教训

1. **JSON转义问题**:
   - Qt的`quoteForCommand()`会转义JSON
   - 需要在Java端反转义
   - 或者不对JSON使用quoteForCommand

2. **定时器轮询问题**:
   - 多个Tab可能有独立的定时器
   - 修复一个地方不够,需要全局搜索
   - 应该使用事件驱动而不是轮询

3. **UI状态管理**:
   - 需要根据数据状态动态调整UI
   - refunding状态需要特殊处理
   - 按钮功能应该明确且一致

4. **异步刷新问题**:
   - 回调中的刷新可能在数据更新前执行
   - 可以添加小延迟确保数据已写入
   - 需要检查API返回值

---

**修复完成时间**: 2025年10月14日 19:31  
**修改文件**: 3个 (EmshopNettyServer.java, CartTab.cpp, AdminTab.cpp/h)  
**需要重启**: 后端服务器 + Qt客户端  
**预期结果**: 问题1-3完全解决,问题4需要进一步排查
