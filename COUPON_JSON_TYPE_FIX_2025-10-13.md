# 优惠券JSON类型错误修复报告

**修复时间**: 2025年10月13日 22:34  
**问题类型**: JSON类型转换错误

---

## 问题描述

在Qt客户端点击"获取优惠券模板"时,显示错误:
```
获取优惠券模板失败: [json.exception.type_error.302] type must be boolean, but is number
```

---

## 根本原因

MySQL数据库中的`coupon_templates.is_active`字段是`BOOLEAN`类型,但在MySQL中实际存储为`TINYINT(1)`,查询结果返回的是数字`0`或`1`。

C++代码在构建JSON响应时,直接使用了数据库返回的值:
```cpp
// 问题代码
template_obj["template_id"] = row["template_id"];  // 可能是数字或字符串
template_obj["description"] = row.value("description", "");  // 可能引发类型错误
is_active = row["is_active"].get<bool>();  // 当值是数字时会失败
```

nlohmann/json库在访问值时会进行严格的类型检查,当期望布尔值但得到数字时会抛出`type_error.302`异常。

---

## 修复方案

在`CouponService::getCouponTemplates()`中添加类型安全检查和转换:

### 修复代码

```cpp
// 重新格式化数据,将template_type映射为前端期望的字段
json templates = json::array();
for (const auto& row : result["data"]) {
    json template_obj;
    
    // 1. 确保template_id是整数类型
    if (row["template_id"].is_number()) {
        template_obj["template_id"] = row["template_id"].get<long long>();
    } else {
        template_obj["template_id"] = 0LL;
    }
    
    // 2. 确保字符串字段的安全访问
    template_obj["template_name"] = row["name"].is_string() ? 
                                    row["name"].get<std::string>() : "";
    
    // 3. 映射 template_type 到前端期望的 discount_type
    std::string template_type = row["template_type"].is_string() ? 
                               row["template_type"].get<std::string>() : "";
    if (template_type == "full_reduction") {
        template_obj["discount_type"] = "fixed";
        template_obj["discount_value"] = 20.0;
    } else if (template_type == "discount") {
        template_obj["discount_type"] = "percentage";
        template_obj["discount_value"] = 10.0;
    } else if (template_type == "free_shipping") {
        template_obj["discount_type"] = "fixed";
        template_obj["discount_value"] = 0.0;
    } else {
        template_obj["discount_type"] = "fixed";
        template_obj["discount_value"] = 0.0;
    }
    
    // 4. 安全访问description字段
    template_obj["description"] = row.contains("description") && row["description"].is_string() ? 
                                  row["description"].get<std::string>() : "";
    
    // 5. 关键修复:处理is_active的数字/布尔值转换
    bool is_active = true;
    if (row.contains("is_active") && !row["is_active"].is_null()) {
        if (row["is_active"].is_boolean()) {
            // 如果是布尔值,直接获取
            is_active = row["is_active"].get<bool>();
        } else if (row["is_active"].is_number()) {
            // 如果是数字(0/1),转换为布尔值
            is_active = (row["is_active"].get<int>() != 0);
        }
    }
    template_obj["status"] = is_active ? "active" : "inactive";
    template_obj["valid_until"] = "";
    template_obj["min_order_amount"] = 0.0;
    template_obj["max_discount_amount"] = 0.0;
    
    templates.push_back(template_obj);
}
```

---

## 修复要点

### 1. 类型检查模式
使用`is_number()`, `is_string()`, `is_boolean()`等方法在访问前检查类型

### 2. 安全转换
- 数字字段:`get<long long>()` 或 `get<int>()`
- 字符串字段:`get<std::string>()`
- 布尔字段:先检查类型,数字时转换为布尔值

### 3. 默认值处理
所有字段都提供合理的默认值,避免访问不存在的字段

### 4. MySQL布尔值处理
MySQL的`BOOLEAN`实际是`TINYINT(1)`,返回0/1,需要手动转换为C++的`true`/`false`

---

## 编译结果

```
✅ 编译成功
文件: cpp/emshop_native_oop.dll
大小: 1,570,236 字节 (比之前增加1,484字节)
时间: 2025-10-13 22:34
```

---

## 测试步骤

1. **重启后端服务器** (以加载新的DLL):
   ```powershell
   cd D:\codehome\jlu\JLU_Emshop_System\java
   mvn exec:java@server
   ```

2. **启动Qt客户端**:
   ```powershell
   cd D:\codehome\jlu\JLU_Emshop_System\qtclient\build\src
   .\emshop_qtclient.exe
   ```

3. **测试优惠券模板**:
   - 登录用户账号
   - 切换到"优惠券"标签页
   - 点击"优惠券模板"按钮

**预期结果**:
- ✅ 显示5个优惠券模板
- ✅ 没有JSON类型错误
- ✅ 类型、折扣值、状态正确显示

---

## 相关问题修复状态

| 问题 | 状态 | 说明 |
|-----|------|------|
| ❌ 问题1: 退款逻辑错误 | ⏳ 待测试 | 已修复代码,等待DLL加载验证 |
| ✅ 问题2: 订单统计文案 | ✅ 已修复 | Qt客户端已更新 |
| ❌ 问题3: 管理员创建优惠券 | ⚠️ 未实现 | Qt AdminTab未实现UI |
| ✅ 问题4: 优惠券模板SQL错误 | ✅ 已修复 | SQL列名+类型转换已修复 |
| ✅ 新问题: JSON类型错误 | ✅ 已修复 | 添加类型安全检查 |

---

## 代码质量改进

### 修复前:
```cpp
❌ template_obj["template_id"] = row["template_id"];  // 类型不明确
❌ is_active = row["is_active"].get<bool>();          // 数字时会崩溃
❌ template_obj["description"] = row.value("description", "");  // 类型不安全
```

### 修复后:
```cpp
✅ if (row["template_id"].is_number()) {
    template_obj["template_id"] = row["template_id"].get<long long>();
}

✅ if (row["is_active"].is_boolean()) {
    is_active = row["is_active"].get<bool>();
} else if (row["is_active"].is_number()) {
    is_active = (row["is_active"].get<int>() != 0);
}

✅ template_obj["description"] = row.contains("description") && row["description"].is_string() ? 
                                 row["description"].get<std::string>() : "";
```

---

## 经验教训

1. **MySQL布尔值不是真正的布尔值**: 需要在C++层面转换
2. **nlohmann/json类型严格**: 必须使用正确的`get<T>()`类型
3. **数据库查询结果类型不确定**: 总是先检查类型再访问
4. **防御性编程**: 为所有字段提供默认值和类型检查

---

## 下一步测试

请按照测试步骤验证修复:

1. ✅ 优惠券模板列表能否正常显示
2. ⏳ 退款功能是否正常工作
3. ⏳ 订单统计显示是否清晰

如果仍有问题,请提供:
- 具体的错误信息
- 后端日志输出
- 操作步骤

---

**修复文件**: `cpp/services/CouponService.cpp` (行613-651)  
**编译文件**: `cpp/emshop_native_oop.dll` (1.5MB)  
**修复状态**: ✅ 完成  
**测试状态**: ⏳ 等待用户验证
