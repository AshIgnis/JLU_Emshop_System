# Bug 修复报告 - 第二部分
**日期**: 2025年10月14日 20:20  
**修复人**: GitHub Copilot  
**版本**: v1.1.1

---

## 修复问题概述

本次修复解决了以下两个关键问题：

1. **促销创建 JSON 解析错误** - CREATE_PROMOTION 命令仍然无法正确解析 JSON
2. **管理员退款审批功能** - 界面显示"未找到该订单的退款申请"

---

## 问题 1: 促销创建 JSON 解析错误

### 问题描述
尽管之前已经修复过 JSON 解析顺序问题，但创建促销活动时仍然报错：

```
Unexpected character ('\' (code 92)): was expecting double-quote to start field name
at [Source: (String)"{\"code\":\"5\",\"discount_type\":\"percentage\",...
```

### 根本原因
反转义逻辑过于复杂，先处理 `\\` 再处理 `\"` 的顺序导致了问题。实际上，从 Qt 客户端发送的 JSON 字符串只需要简单地反转义 `\"` 为 `"` 即可。

### 修复方案

**文件**: `java/src/main/java/emshop/EmshopNettyServer.java`  
**位置**: 第 1114-1131 行

**修改内容**:
```java
// 修改前 (错误的反转义逻辑)
if (jsonStr.startsWith("\"") && jsonStr.endsWith("\"")) {
    jsonStr = jsonStr.substring(1, jsonStr.length() - 1);
    jsonStr = jsonStr.replace("\\\\", "\\");  // 先处理反斜杠
    jsonStr = jsonStr.replace("\\\"", "\"");   // 再处理引号
}

// 修改后 (简化的反转义逻辑)
handlerLogger.info("CREATE_PROMOTION - 原始JSON: {}", jsonStr);

if (jsonStr.startsWith("\"") && jsonStr.endsWith("\"")) {
    jsonStr = jsonStr.substring(1, jsonStr.length() - 1);
    handlerLogger.info("CREATE_PROMOTION - 移除外层引号后: {}", jsonStr);
}

// 只需要简单地反转义引号
jsonStr = jsonStr.replace("\\\"", "\"");
handlerLogger.info("CREATE_PROMOTION - 反转义后: {}", jsonStr);
```

**关键改进**:
1. 移除了对 `\\` 的反转义处理（不需要）
2. 简化为只处理 `\"` → `"`
3. 添加了详细的调试日志以便追踪问题

---

## 问题 2: 管理员退款审批功能

### 问题描述
当管理员尝试审批退款时：
- 服务器日志显示：`获取用户退款申请，用户ID: 28`
- 但界面显示：`未找到该订单的退款申请`

### 根本原因
1. `GET_USER_REFUND_REQUESTS` 命令接受的参数是 `user_id`，不是 `order_id`
2. 管理员界面传递的是 `order_id`，但后端函数按 `user_id` 查询
3. 导致查询不到正确的退款申请记录

### 修复方案

#### 1. 后端修改: 支持按 order_id 查询

**文件**: `cpp/services/OrderService.cpp`  
**位置**: 第 1548-1573 行

**修改策略**: 使用负数作为特殊标记
- 如果传入的 `user_id` 为负数，则将其绝对值作为 `order_id` 查询
- 如果传入的 `user_id` 为正数，则按原逻辑以 `user_id` 查询

**修改内容**:
```cpp
json OrderService::getUserRefundRequests(long user_id) {
    // 特殊处理：如果 user_id 为负数，则将其绝对值作为 order_id 来查询
    bool query_by_order_id = (user_id < 0);
    long actual_id = query_by_order_id ? -user_id : user_id;
    
    if (query_by_order_id) {
        logInfo("获取订单退款申请，订单ID: " + std::to_string(actual_id));
    } else {
        logInfo("获取用户退款申请，用户ID: " + std::to_string(actual_id));
    }
    
    try {
        std::string query_sql = "SELECT r.refund_id, r.order_id, r.reason, r.refund_amount, "
                               "r.status, r.admin_reply, r.created_at, r.processed_at, o.order_no "
                               "FROM refund_requests r "
                               "JOIN orders o ON r.order_id = o.order_id "
                               "WHERE ";
        
        if (query_by_order_id) {
            query_sql += "r.order_id = " + std::to_string(actual_id);
        } else {
            query_sql += "r.user_id = " + std::to_string(actual_id);
        }
        
        query_sql += " ORDER BY r.created_at DESC";
        
        json result = executeQuery(query_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        return createSuccessResponse(result["data"], "获取退款申请成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取退款申请异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}
```

#### 2. 前端修改: 传递负数的 order_id

**文件**: `qtclient/src/ui/tabs/AdminTab.cpp`  
**位置**: 第 770-804 行

**修改内容**:
```cpp
void AdminTab::approveRefund(qlonglong orderId, bool approve)
{
    // 首先获取该订单的refund_id
    // 传递负数的orderId，让后端按order_id查询
    QString getCmd = QString("GET_USER_REFUND_REQUESTS %1").arg(-orderId);
    sendCommand(getCmd, [this, orderId, approve](const QJsonDocument &doc){
        // 从返回结果中查找该订单的退款申请
        QJsonArray refunds = JsonUtils::extract(doc, "data").toArray();
        
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
        
        // 调用审批接口
        QString approveStr = approve ? "true" : "false";
        QString adminReply = approve ? "管理员已同意退款" : "管理员已拒绝退款";
        QString cmd = QString("APPROVE_REFUND %1 %2 %3").arg(refundId).arg(approveStr).arg(adminReply);
        sendCommand(cmd, [this, approve](const QJsonDocument&){
            emit statusMessage(approve ? tr("退款已审批通过") : tr("退款已拒绝"), true);
            refreshAllOrders();
        }, approve ? tr("审批退款") : tr("拒绝退款"));
    }, tr("获取退款信息"));
}
```

**关键改进**:
1. 传递 `-orderId` 而不是 `orderId`
2. 简化了 JSON 解析逻辑（直接从 `data` 数组获取）
3. 移除了不必要的字段查找逻辑

---

## 编译和部署

### 编译步骤

1. **编译 C++ DLL**:
   ```powershell
   cd cpp
   .\build_oop_jni.bat
   ```
   结果: `emshop_native_oop.dll` (1,575,724 字节) - 2025/10/14 20:18

2. **复制 DLL 到 Java 项目**:
   ```powershell
   Copy-Item cpp\emshop_native_oop.dll java\src\ -Force
   ```

3. **清理并重新编译 Qt 客户端**:
   ```powershell
   # 清理旧的构建文件
   Remove-Item qtclient\build\* -Recurse -Force
   
   # 创建构建目录并复制 DLL
   mkdir qtclient\build -Force
   Copy-Item cpp\emshop_native_oop.dll qtclient\build\ -Force
   Copy-Item cpp\libmysql.dll qtclient\build\ -Force
   
   # 配置和构建 Qt 项目
   cmake -S qtclient -B qtclient/build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/6.9.1/mingw_64
   cmake --build qtclient/build -- -j
   
   # 复制 DLL 到最终输出目录
   Copy-Item cpp\emshop_native_oop.dll qtclient\build\src\ -Force
   Copy-Item cpp\libmysql.dll qtclient\build\src\ -Force
   ```

4. **重启 Java 后端服务器**:
   ```powershell
   cd java
   mvn exec:java@server
   ```

### 部署验证

编译成功后的文件：
- ✅ `cpp/emshop_native_oop.dll` (1,575,724 字节)
- ✅ `java/src/emshop_native_oop.dll` (已复制)
- ✅ `qtclient/build/src/emshop_qtclient.exe` (重新编译)
- ✅ `qtclient/build/src/emshop_native_oop.dll` (已复制)

---

## 测试指南

### 测试 1: 促销创建功能

1. 以管理员身份登录 Qt 客户端
2. 进入"优惠券"标签
3. 创建新的促销活动：
   - 名称: 十一大促
   - 代码: 5
   - 类型: 百分比折扣
   - 折扣值: 10%
   - 最低消费: 0元
   - 有效期: 2025/10/14 - 2025/11/14
4. 点击"创建促销/优惠券"
5. **预期结果**: 成功创建，不再出现 JSON 解析错误

### 测试 2: 退款审批功能

**前置条件**: 用户已提交退款申请

1. 以管理员身份登录 Qt 客户端
2. 进入"管理员"标签
3. 在订单列表中找到有退款申请的订单
4. 点击"审批退款"或"拒绝退款"按钮
5. **预期结果**: 
   - 成功获取退款申请信息
   - 显示确认对话框
   - 审批后订单状态更新
   - 不再显示"未找到该订单的退款申请"错误

---

## 技术要点

### JSON 反转义最佳实践

在处理从客户端传来的 JSON 字符串时：
1. **最简原则**: 只做必要的反转义
2. **单次转换**: 避免多次字符串替换导致的问题
3. **添加日志**: 在每个处理步骤添加日志便于调试

### 函数参数设计技巧

当需要让一个函数支持多种查询方式时：
1. **使用特殊值**: 如负数表示不同的语义
2. **保持向后兼容**: 原有调用方式不受影响
3. **清晰的日志**: 记录使用的是哪种查询方式

---

## 影响范围

### 修改的文件

1. **Java 后端** (1 个文件):
   - `java/src/main/java/emshop/EmshopNettyServer.java`

2. **C++ 服务层** (1 个文件):
   - `cpp/services/OrderService.cpp`

3. **Qt 客户端** (1 个文件):
   - `qtclient/src/ui/tabs/AdminTab.cpp`

### 影响的功能模块

1. **促销管理模块**:
   - ✅ 创建促销活动
   - ✅ JSON 数据解析
   
2. **订单管理模块**:
   - ✅ 退款申请查询
   - ✅ 退款审批功能
   
3. **管理员功能**:
   - ✅ 审批/拒绝退款
   - ✅ 查看退款详情

---

## 已知问题

目前没有已知问题。

---

## 后续建议

1. **添加单元测试**: 为 JSON 解析逻辑添加单元测试
2. **API 文档**: 更新 `GET_USER_REFUND_REQUESTS` 命令的文档，说明负数参数的用法
3. **代码重构**: 考虑将 `getUserRefundRequests` 拆分为两个独立的函数：
   - `getUserRefundRequests(user_id)` - 按用户ID查询
   - `getOrderRefundRequest(order_id)` - 按订单ID查询

---

## 总结

本次修复解决了两个关键的业务功能问题：

1. **促销创建**: 简化了 JSON 反转义逻辑，确保促销活动可以正常创建
2. **退款审批**: 修改了后端查询逻辑和前端调用方式，确保管理员可以正确审批退款

所有修改都经过编译验证，可以立即部署测试。

---

**修复完成时间**: 2025年10月14日 20:20  
**状态**: ✅ 已完成并可测试
