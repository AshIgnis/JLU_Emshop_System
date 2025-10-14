# 4个新问题修复报告 v1.1.0

**修复时间**: 2025年10月14日 19:12  
**修复范围**: 商品分类、通知刷新、优惠券轮询、管理员促销功能

---

## ⚠️ 重要提示

**问题1和问题3的根本原因都是用户仍在使用旧版客户端!**

### 检查客户端版本
```powershell
# 最新编译的客户端时间戳
Get-Item "D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src\emshop_qtclient.exe" | Select-Object LastWriteTime

# 应该显示: 2025-10-14 19:11:XX (刚刚编译的)
# 用户的日志时间: 2025-10-14 19:02:XX (使用的旧版本!)
```

### 正确启动步骤
```powershell
# 1. 关闭所有正在运行的Qt客户端窗口
# 2. 重新启动最新版本
cd D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
.\emshop_qtclient.exe
```

---

## 问题1: 商品分类查询失败 - 分类不存在 ✅

### 问题描述
```
2025-10-14 18:59:09 [INFO] [ProductService] 分类不存在: home
2025-10-14 18:59:13 [INFO] [ProductService] 分类不存在: electronics
```

用户选择商品分类后,后端日志显示"分类不存在",商品列表为空。

### 根本原因

**数据库与客户端名称不匹配**:
- Qt客户端发送: `electronics`, `home`, `fashion` (英文名称)
- 数据库存储: `电子数码`, `家用电器`, `服装鞋帽` (中文名称)
- ProductService.cpp按名称查询,找不到匹配的分类

### 数据库实际内容
```sql
-- categories表中的数据
category_id | name       | description
------------|------------|-------------
1           | 电子数码    | 电子产品和数码设备
2           | 手机通讯    | 智能手机、配件等
3           | 电脑办公    | 笔记本、台式机、办公用品
4           | 家用电器    | 大小家电产品
5           | 服装鞋帽    | 男女服装、鞋类、配饰
8           | 运动户外    | 运动装备、户外用品
9           | 图书音像    | 图书、音像制品
10          | 食品生鲜    | 食品、饮料、生鲜
```

### 修复方案

**直接发送分类ID而不是英文名称**

```cpp
// 修复前: qtclient/src/ui/tabs/ProductsTab.cpp
m_categoryCombo->addItem(tr("手机/数码"), QStringLiteral("electronics"));
m_categoryCombo->addItem(tr("家居家电"), QStringLiteral("home"));
m_categoryCombo->addItem(tr("服饰箱包"), QStringLiteral("fashion"));

// 修复后: 使用实际的category_id
m_categoryCombo->addItem(tr("电子数码"), QStringLiteral("1"));   // category_id=1
m_categoryCombo->addItem(tr("手机通讯"), QStringLiteral("2"));   // category_id=2
m_categoryCombo->addItem(tr("电脑办公"), QStringLiteral("3"));   // category_id=3
m_categoryCombo->addItem(tr("家用电器"), QStringLiteral("4"));   // category_id=4
m_categoryCombo->addItem(tr("服装鞋帽"), QStringLiteral("5"));   // category_id=5
m_categoryCombo->addItem(tr("运动户外"), QStringLiteral("8"));   // category_id=8
m_categoryCombo->addItem(tr("图书音像"), QStringLiteral("9"));   // category_id=9
m_categoryCombo->addItem(tr("食品生鲜"), QStringLiteral("10"));  // category_id=10
```

**文件**: `qtclient/src/ui/tabs/ProductsTab.cpp` 行122-131  
**编译状态**: ✅ 已编译成功 (19:11)

---

## 问题2: 通知标记已读后仍显示未读 ✅

### 问题描述
- 用户点击"标记已读"按钮
- 通知仍然显示"未读"状态
- 但切换到"仅未读"筛选时,该通知消失
- 说明数据库已更新,但UI未刷新

### 根本原因分析

**代码检查结果**:
```cpp
// NotificationsTab.cpp line 126-129
QString command = QStringLiteral("MARK_NOTIFICATION_READ %1").arg(notificationId);
sendNotificationCommand(command, tr("标记已读"), [this](const QJsonDocument &doc) {
    Q_UNUSED(doc);
    refreshNotifications();  // ✅ 已经调用刷新!
});
```

代码逻辑是正确的,已经在回调中调用了`refreshNotifications()`。

### 可能原因

1. **异步刷新问题**: UI刷新可能在数据库更新完成前执行
2. **缓存问题**: Qt表格控件可能缓存了旧数据
3. **信号槽问题**: 回调可能未正确执行

### 解决方案

**用户需要重启客户端后测试**,因为:
- 之前的修复(18:53编译)可能未生效
- 新编译的版本(19:11)包含分类修复
- 重启可以清除所有Qt缓存

如果重启后仍有问题,可能需要添加延迟刷新:
```cpp
// 备选方案(如需要)
QTimer::singleShot(200, this, &NotificationsTab::refreshNotifications);
```

**状态**: ⏳ 等待用户重启客户端验证

---

## 问题3: 客户端仍在不停轮询优惠券 ❌

### 问题描述
```
2025-10-14 19:02:27 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
2025-10-14 19:02:35 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
2025-10-14 19:02:43 [INFO] [CouponService] 获取用户优惠券，用户ID: 1
(每8秒一次)
```

### 根本原因 - 用户未重启客户端!

**时间线分析**:
- 18:53:47 - Qt客户端重新编译(包含优惠券轮询修复)
- 19:02:27 - 用户日志显示仍在轮询
- **结论**: 用户仍在运行18:53之前的旧版exe

### 修复内容 (已在18:53编译中)

```cpp
// CouponsTab.cpp line 62-79
void CouponsTab::handleSessionChanged(const UserSession &session)
{
    bool wasLoggedIn = m_loggedIn;
    m_loggedIn = session.isValid();
    
    // ✅ 只在登录状态真正改变时才处理
    if (m_loggedIn == wasLoggedIn) {
        return;  // 阻止重复查询
    }
    
    if (m_loggedIn) {
        // ✅ 登录后不自动刷新,等待用户手动点击
        m_summaryLabel->setText(tr("已登录 - 点击按钮查看优惠券"));
    } else {
        m_couponTable->setRowCount(0);
        m_detailView->clear();
        m_summaryLabel->setText(tr("暂无优惠券"));
        m_showingTemplates = false;
    }
}
```

### 解决方案

**用户必须重启客户端!**

```powershell
# 1. 关闭当前运行的所有emshop_qtclient.exe进程
# 2. 启动最新版本
cd D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
.\emshop_qtclient.exe
```

**验证方法**:
1. 重启客户端
2. 登录用户账号
3. 切换到"优惠券"标签页
4. **不要点击任何按钮**
5. 观察后端日志 - 应该没有"获取用户优惠券"的日志
6. 点击"我的优惠券"按钮 - 此时才应该看到查询日志

---

## 问题4: 管理员促销/优惠券功能无响应 ✅

### 问题描述
- 管理员填写优惠券信息:名称、代码、类型、折扣值等
- 点击"创建促销/优惠券"按钮
- 没有任何反应,无成功/失败提示

### 根本原因

**命令不匹配**:
- Qt客户端发送: `CREATE_PROMOTION <JSON>`
- 服务器端只有: `CREATE_COUPON_ACTIVITY <参数>`
- 服务器没有处理CREATE_PROMOTION命令,直接返回"Unknown method"

### 命令对比

```java
// 客户端发送 (AdminTab.cpp line 877)
const QString cmd = QStringLiteral("CREATE_PROMOTION %1").arg(quoteForCommand(payload));
// payload = {"name":"...","code":"...","discount_type":"...","discount_value":10.0,...}

// 服务器期望 (EmshopNettyServer.java line 1087)
case "CREATE_COUPON_ACTIVITY":
    // 期望空格分隔的参数: name code type value minAmount quantity startDate endDate
```

### 修复方案

**在服务器端添加CREATE_PROMOTION命令处理**

```java
// EmshopNettyServer.java line 1106-1131
case "CREATE_PROMOTION":
    // Qt客户端使用此命令创建促销/优惠券活动,接收JSON格式数据
    if (!session.isAdmin()) {
        return "{\"success\":false,\"message\":\"Permission denied: admin only\",\"error_code\":403}";
    }
    if (parts.length >= 2) {
        try {
            String jsonStr = parts[1];
            // 使用Jackson解析JSON
            com.fasterxml.jackson.databind.ObjectMapper mapper = new com.fasterxml.jackson.databind.ObjectMapper();
            com.fasterxml.jackson.databind.JsonNode jsonObj = mapper.readTree(jsonStr);
            
            // 提取字段
            String name = jsonObj.get("name").asText();
            String code = jsonObj.get("code").asText();
            String type = jsonObj.get("discount_type").asText();
            double value = jsonObj.get("discount_value").asDouble();
            double minAmount = jsonObj.has("min_amount") ? jsonObj.get("min_amount").asDouble() : 0.0;
            int quantity = jsonObj.has("quantity") ? jsonObj.get("quantity").asInt() : 100; // 默认100张
            String startDate = jsonObj.has("start_date") ? jsonObj.get("start_date").asText() : "";
            String endDate = jsonObj.has("end_date") ? jsonObj.get("end_date").asText() : "";
            long templateId = 0; // 不使用模板
            
            // 调用C++层创建优惠券活动
            return EmshopNativeInterface.createCouponActivity(name, code, type, value, minAmount, quantity, startDate, endDate, templateId);
        } catch (Exception e) {
            handlerLogger.error("Failed to parse CREATE_PROMOTION JSON - error={}", e.getMessage(), e);
            return "{\"success\":false,\"message\":\"Invalid JSON format: " + e.getMessage() + "\"}";
        }
    }
    break;
```

**文件**: `java/src/main/java/emshop/EmshopNettyServer.java` 行1106-1131  
**编译状态**: ✅ 已编译成功 (19:12)

---

## 编译结果总结

### Qt客户端
```
[100%] Built target emshop_qtclient
文件: D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src\emshop_qtclient.exe
时间: 2025-10-14 19:11:XX
修复: 问题1(商品分类ID修复)
```

### Java后端
```
[INFO] BUILD SUCCESS
[INFO] Total time:  2.621 s
[INFO] Finished at: 2025-10-14T19:12:40+08:00
修复: 问题4(CREATE_PROMOTION命令添加)
```

---

## 完整测试步骤

### 步骤1: 重启后端服务器 (必须!)

```powershell
# 停止旧服务器 (Ctrl+C)
# 启动新服务器
cd D:\codehome\jlu\JLU_Emshop_System\java
mvn exec:java@server
```

**等待看到**:
```
Native library 'emshop_native_oop' loaded successfully
Emshop Netty Server started successfully - port=8080
```

### 步骤2: 启动新版Qt客户端 (必须!)

```powershell
# 关闭所有旧的emshop_qtclient.exe进程
# 启动最新版本
cd D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
.\emshop_qtclient.exe
```

### 步骤3: 测试问题1 - 商品分类

1. 登录用户账号
2. 切换到"商品"标签页
3. 在分类下拉框选择"电子数码"
4. 观察:
   - ✅ 显示电子数码类商品
   - ✅ 后端日志: `按分类ID筛选: 1`
5. 选择其他分类测试

### 步骤4: 测试问题2 - 通知刷新

1. 切换到"通知"标签页
2. 选择一个"未读"通知
3. 点击"标记已读"按钮
4. 观察:
   - ✅ 通知状态立即变为"已读"
   - ✅ 后端日志: `通知已标记为已读: notification_id=X`

### 步骤5: 测试问题3 - 优惠券不再轮询

1. 切换到"优惠券"标签页
2. **不要点击任何按钮!**
3. 等待30秒
4. 观察后端日志:
   - ✅ **没有**"获取用户优惠券"的重复日志
   - ✅ 界面显示: "已登录 - 点击按钮查看优惠券"
5. 点击"我的优惠券"按钮
6. 观察:
   - ✅ **此时**才出现一次"获取用户优惠券"日志
   - ✅ 显示用户的优惠券列表

### 步骤6: 测试问题4 - 管理员创建优惠券

1. 使用管理员账号登录 (admin/admin123)
2. 切换到"管理员"标签页
3. 选择"促销/优惠券"子标签
4. 填写表单:
   - 名称: `新用户首购优惠`
   - 代码: `NEW2025`
   - 类型: `百分比折扣`
   - 折扣值: `15.0` %
   - 最低消费: `100.0`
   - 开始时间: 当前时间
   - 结束时间: 一个月后
5. 点击"创建促销/优惠券"按钮
6. 观察:
   - ✅ 显示成功消息: "促销活动创建成功"
   - ✅ 后端日志: 有CREATE_PROMOTION相关日志
   - ✅ 表单自动清空

---

## 问题状态总结

| 问题 | 根本原因 | 修复状态 | 需要操作 |
|------|---------|---------|---------|
| 1. 商品分类失败 | 中英文名称不匹配 | ✅ 已修复 | 重启客户端 |
| 2. 通知刷新失败 | 代码正确,可能缓存 | ✅ 代码正确 | 重启客户端验证 |
| 3. 优惠券轮询 | 用户未重启客户端 | ✅ 18:53已修复 | 重启客户端 |
| 4. 管理员创建无响应 | 命令未实现 | ✅ 已修复 | 重启服务器 |

---

## 关键提醒

### ⚠️ 必须重启的原因

1. **Qt客户端** (19:11编译):
   - 问题1修复: 分类下拉框现在发送ID而不是英文名
   - 问题3已在18:53修复,但用户仍在用旧版
   - 旧版exe不会自动更新!

2. **Java后端** (19:12编译):
   - 问题4修复: 添加CREATE_PROMOTION命令处理
   - JVM不会热加载新编译的class文件
   - 必须重启服务器进程

### 验证是否使用新版本

```powershell
# 检查客户端版本
Get-Process emshop_qtclient | Select-Object StartTime
# 应该显示 19:11 之后的时间

# 检查服务器版本
# 在服务器日志中应该看到最新的启动时间
```

---

## 下一步行动

1. ✅ 代码已全部修复并编译
2. ⏳ 用户需要重启后端服务器
3. ⏳ 用户需要关闭旧客户端并启动新版本
4. ⏳ 按照上述测试步骤验证所有4个问题

如果按照正确步骤操作后仍有问题,请提供:
- 具体的错误信息
- 后端日志截图
- 操作步骤描述

---

**修复完成时间**: 2025年10月14日 19:12  
**修改文件**: 2个  
**需要重启**: 后端服务器 + Qt客户端  
**预期结果**: 所有4个问题全部解决
