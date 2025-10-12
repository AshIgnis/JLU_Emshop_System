# 错误码规范文档 (ERROR_CODES.md)

## 概述

本文档定义了JLU Emshop System的统一错误码规范,用于标准化前后端错误处理和客户端错误提示。

**错误码格式**: `EXXXYY`
- `XX`: 模块代码 (00-99)
- `Y`: 错误类型 (0-9)
- `YY`: 具体错误序号 (00-99)

**示例**: `E10201` = 用户模块(10) + 业务错误(2) + 密码错误(01)

---

## 错误码分类

### 错误类型代码

| 类型代码 | 含义 | 示例 |
|---------|------|------|
| 0x | 系统级错误 | 数据库连接失败、内存不足 |
| 1x | 验证错误 | 参数缺失、格式错误 |
| 2x | 业务错误 | 库存不足、重复注册 |
| 3x | 权限错误 | 未登录、无权限 |
| 4x | 资源错误 | 记录不存在、文件未找到 |
| 9x | 未知错误 | 异常捕获 |

---

## 模块代码分配

| 模块代码 | 模块名称 | 负责功能 |
|---------|---------|---------|
| 00 | 通用 | 通用错误、系统级错误 |
| 10 | 用户认证 | 注册、登录、Token验证 |
| 20 | 商品管理 | 商品查询、库存管理 |
| 30 | 购物车 | 添加购物车、修改数量 |
| 40 | 订单管理 | 订单创建、状态管理 |
| 50 | 支付系统 | 支付流程、退款处理 |
| 60 | 优惠券 | 优惠券查询、使用 |
| 70 | 促销活动 | 促销规则、价格计算 |
| 80 | 地址管理 | 收货地址CRUD |
| 90 | 系统管理 | 日志、审计、配置 |

---

## 错误码定义

### 00 - 通用模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E00001 | 500 | 数据库连接失败 | 数据库连接失败,请稍后重试 | Database connection failed |
| E00002 | 500 | 数据库查询错误 | 数据库查询失败 | Database query error |
| E00003 | 500 | 配置文件加载失败 | 配置加载失败 | Config file load error |
| E00004 | 500 | JNI调用错误 | 系统内部错误 | JNI invocation error |
| E00005 | 500 | 内存不足 | 系统资源不足 | Out of memory |
| E00101 | 400 | 请求参数缺失 | 缺少必要参数 | Missing required parameter |
| E00102 | 400 | JSON格式错误 | 请求格式错误 | Invalid JSON format |
| E00103 | 400 | 缺少action字段 | 缺少操作类型 | Missing action field |
| E00104 | 400 | 无效的action值 | 不支持的操作 | Invalid action value |
| E00105 | 400 | 参数类型错误 | 参数类型不正确 | Invalid parameter type |
| E00901 | 500 | 未知错误 | 系统异常,请联系管理员 | Unknown error |

### 10 - 用户认证模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E10101 | 400 | 用户名为空 | 请输入用户名 | Username is required |
| E10102 | 400 | 密码为空 | 请输入密码 | Password is required |
| E10103 | 400 | 邮箱格式错误 | 邮箱格式不正确 | Invalid email format |
| E10104 | 400 | 手机号格式错误 | 手机号格式不正确 | Invalid phone number |
| E10105 | 400 | 用户名长度不符 | 用户名需3-20个字符 | Username length invalid |
| E10106 | 400 | 密码长度不符 | 密码需6-20个字符 | Password length invalid |
| E10201 | 401 | 用户名或密码错误 | 用户名或密码错误 | Invalid username or password |
| E10202 | 409 | 用户名已存在 | 该用户名已被注册 | Username already exists |
| E10203 | 409 | 邮箱已注册 | 该邮箱已被注册 | Email already registered |
| E10204 | 409 | 手机号已注册 | 该手机号已被注册 | Phone already registered |
| E10301 | 401 | Token缺失 | 请先登录 | Token is missing |
| E10302 | 401 | Token无效 | 登录已失效,请重新登录 | Invalid token |
| E10303 | 401 | Token已过期 | 登录已过期,请重新登录 | Token expired |
| E10304 | 403 | 无权限 | 您没有权限执行此操作 | Permission denied |
| E10401 | 404 | 用户不存在 | 用户不存在 | User not found |

### 20 - 商品管理模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E20101 | 400 | 商品ID缺失 | 请提供商品ID | Product ID is required |
| E20102 | 400 | 商品名称为空 | 请输入商品名称 | Product name is required |
| E20103 | 400 | 商品价格无效 | 商品价格必须大于0 | Invalid product price |
| E20104 | 400 | 库存数量无效 | 库存数量不能为负 | Invalid stock quantity |
| E20105 | 400 | 分类ID无效 | 无效的分类ID | Invalid category ID |
| E20201 | 400 | 库存不足 | 商品库存不足 | Insufficient stock |
| E20202 | 409 | 商品已下架 | 商品已下架,无法购买 | Product is unavailable |
| E20203 | 409 | 商品重复 | 商品已存在 | Product already exists |
| E20401 | 404 | 商品不存在 | 商品不存在 | Product not found |
| E20402 | 404 | 分类不存在 | 分类不存在 | Category not found |

### 30 - 购物车模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E30101 | 400 | 商品ID缺失 | 请选择商品 | Product ID is required |
| E30102 | 400 | 数量无效 | 商品数量必须大于0 | Invalid quantity |
| E30103 | 400 | 购物车ID无效 | 购物车项不存在 | Invalid cart item ID |
| E30201 | 400 | 购物车为空 | 购物车为空 | Cart is empty |
| E30202 | 400 | 库存不足 | 商品库存不足,请调整数量 | Insufficient stock for cart items |
| E30203 | 409 | 商品已在购物车 | 商品已在购物车中 | Product already in cart |
| E30401 | 404 | 购物车项不存在 | 购物车项不存在 | Cart item not found |

### 40 - 订单管理模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E40101 | 400 | 订单ID缺失 | 请提供订单ID | Order ID is required |
| E40102 | 400 | 收货地址为空 | 请填写收货地址 | Shipping address is required |
| E40103 | 400 | 订单金额错误 | 订单金额计算错误 | Invalid order amount |
| E40104 | 400 | 订单明细为空 | 订单明细不能为空 | Order items cannot be empty |
| E40201 | 409 | 库存不足 | 部分商品库存不足 | Insufficient stock for some items |
| E40202 | 409 | 订单状态错误 | 当前订单状态不允许此操作 | Invalid order status |
| E40203 | 409 | 订单已支付 | 订单已支付,无法取消 | Order already paid |
| E40204 | 409 | 订单已发货 | 订单已发货,无法取消 | Order already shipped |
| E40205 | 409 | 订单不可退款 | 订单状态不允许退款 | Order cannot be refunded |
| E40301 | 403 | 无权操作订单 | 您无权操作此订单 | No permission for this order |
| E40401 | 404 | 订单不存在 | 订单不存在 | Order not found |
| E40402 | 404 | 订单明细不存在 | 订单明细不存在 | Order items not found |

### 50 - 支付系统模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E50101 | 400 | 支付方式无效 | 请选择支付方式 | Invalid payment method |
| E50102 | 400 | 支付金额错误 | 支付金额与订单金额不符 | Payment amount mismatch |
| E50201 | 402 | 支付失败 | 支付失败,请重试 | Payment failed |
| E50202 | 402 | 余额不足 | 账户余额不足 | Insufficient balance |
| E50203 | 409 | 支付超时 | 支付超时,请重新下单 | Payment timeout |
| E50204 | 409 | 重复支付 | 订单已支付 | Duplicate payment |
| E50301 | 500 | 支付网关错误 | 支付系统异常 | Payment gateway error |
| E50302 | 500 | 退款失败 | 退款处理失败 | Refund failed |

### 60 - 优惠券模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E60101 | 400 | 优惠券代码为空 | 请输入优惠券代码 | Coupon code is required |
| E60201 | 400 | 优惠券不可用 | 优惠券不可用 | Coupon is unavailable |
| E60202 | 400 | 优惠券已过期 | 优惠券已过期 | Coupon expired |
| E60203 | 400 | 优惠券已用完 | 优惠券已领完 | Coupon out of stock |
| E60204 | 400 | 不满足使用条件 | 订单金额不满足优惠券使用条件 | Order amount does not meet coupon requirement |
| E60205 | 409 | 优惠券已领取 | 您已领取过此优惠券 | Coupon already claimed |
| E60206 | 409 | 优惠券已使用 | 优惠券已使用 | Coupon already used |
| E60401 | 404 | 优惠券不存在 | 优惠券不存在 | Coupon not found |

### 70 - 促销活动模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E70101 | 400 | 促销ID无效 | 促销活动ID无效 | Invalid promotion ID |
| E70201 | 400 | 促销未开始 | 促销活动尚未开始 | Promotion not started |
| E70202 | 400 | 促销已结束 | 促销活动已结束 | Promotion ended |
| E70203 | 409 | 促销冲突 | 该商品已参与其他促销 | Promotion conflict |
| E70401 | 404 | 促销不存在 | 促销活动不存在 | Promotion not found |

### 80 - 地址管理模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E80101 | 400 | 地址ID缺失 | 请提供地址ID | Address ID is required |
| E80102 | 400 | 收货人为空 | 请填写收货人姓名 | Receiver name is required |
| E80103 | 400 | 手机号为空 | 请填写手机号 | Phone number is required |
| E80104 | 400 | 详细地址为空 | 请填写详细地址 | Detailed address is required |
| E80201 | 400 | 地址数量超限 | 最多只能添加10个地址 | Address limit exceeded |
| E80401 | 404 | 地址不存在 | 地址不存在 | Address not found |

### 90 - 系统管理模块

| 错误码 | HTTP状态码 | 错误描述 | 中文提示 | 英文提示 |
|-------|-----------|---------|---------|---------|
| E90301 | 403 | 需要管理员权限 | 需要管理员权限 | Admin permission required |
| E90302 | 403 | 操作被拒绝 | 操作被拒绝 | Operation denied |
| E90001 | 500 | 日志写入失败 | 日志记录失败 | Log write error |
| E90002 | 500 | 审计记录失败 | 审计记录失败 | Audit log error |

---

## 成功响应格式

```json
{
  "success": true,
  "code": "S00000",
  "message": "操作成功",
  "data": {
    // 业务数据
  },
  "timestamp": 1705123456789
}
```

## 错误响应格式

```json
{
  "success": false,
  "code": "E10201",
  "message": "用户名或密码错误",
  "error_detail": "Password verification failed",
  "timestamp": 1705123456789,
  "request_id": "req_1234567890"
}
```

---

## 客户端错误处理示例

### C++ (JNI层)

```cpp
// Constants.h
class Constants {
public:
    // 通用错误
    static constexpr const char* SUCCESS_CODE = "S00000";
    static constexpr const char* DB_ERROR_CODE = "E00001";
    static constexpr const char* INVALID_JSON_CODE = "E00102";
    
    // 用户模块
    static constexpr const char* INVALID_CREDENTIALS_CODE = "E10201";
    static constexpr const char* USER_EXISTS_CODE = "E10202";
    static constexpr const char* INVALID_TOKEN_CODE = "E10302";
    
    // 商品模块
    static constexpr const char* INSUFFICIENT_STOCK_CODE = "E20201";
    static constexpr const char* PRODUCT_NOT_FOUND_CODE = "E20401";
    
    // 订单模块
    static constexpr const char* INVALID_ORDER_STATUS_CODE = "E40202";
    static constexpr const char* ORDER_NOT_FOUND_CODE = "E40401";
};

// 使用示例
json createErrorResponse(const std::string& message, const char* errorCode) {
    json response;
    response["success"] = false;
    response["code"] = errorCode;
    response["message"] = message;
    response["timestamp"] = getCurrentTimestamp();
    return response;
}

// 调用
return createErrorResponse("库存不足", Constants::INSUFFICIENT_STOCK_CODE);
```

### Java (Netty服务端)

```java
public enum ErrorCode {
    // 通用错误
    SUCCESS("S00000", "操作成功"),
    DB_ERROR("E00001", "数据库连接失败"),
    INVALID_JSON("E00102", "JSON格式错误"),
    
    // 用户模块
    INVALID_CREDENTIALS("E10201", "用户名或密码错误"),
    USER_EXISTS("E10202", "用户名已存在"),
    INVALID_TOKEN("E10302", "Token无效"),
    
    // 商品模块
    INSUFFICIENT_STOCK("E20201", "库存不足"),
    PRODUCT_NOT_FOUND("E20401", "商品不存在"),
    
    // 订单模块
    INVALID_ORDER_STATUS("E40202", "订单状态错误"),
    ORDER_NOT_FOUND("E40401", "订单不存在");
    
    private final String code;
    private final String message;
    
    ErrorCode(String code, String message) {
        this.code = code;
        this.message = message;
    }
    
    public String getCode() { return code; }
    public String getMessage() { return message; }
}

// 使用示例
public JsonObject createErrorResponse(ErrorCode errorCode, String detail) {
    JsonObject response = new JsonObject();
    response.addProperty("success", false);
    response.addProperty("code", errorCode.getCode());
    response.addProperty("message", errorCode.getMessage());
    response.addProperty("error_detail", detail);
    response.addProperty("timestamp", System.currentTimeMillis());
    return response;
}
```

### Qt (客户端)

```cpp
// ErrorHandler.h
class ErrorHandler {
public:
    static QString getLocalizedMessage(const QString& errorCode) {
        static const QMap<QString, QString> errorMessages = {
            {"E10201", "用户名或密码错误,请重试"},
            {"E10302", "登录已失效,请重新登录"},
            {"E20201", "商品库存不足,请减少购买数量"},
            {"E40202", "当前订单状态不允许此操作"},
            // ...更多错误码映射
        };
        
        return errorMessages.value(errorCode, "操作失败,请稍后重试");
    }
    
    static void showError(QWidget* parent, const QJsonObject& response) {
        QString code = response["code"].toString();
        QString message = getLocalizedMessage(code);
        
        QMessageBox::warning(parent, "操作失败", message);
        
        // 特殊错误码处理
        if (code == "E10302" || code == "E10303") {
            // Token失效,跳转到登录页面
            emit needRelogin();
        }
    }
};
```

---

## 错误码使用规范

### 1. 错误码选择原则

- **优先使用已定义的错误码**: 避免重复定义
- **模块内连续编号**: 便于管理和查找
- **预留扩展空间**: 每个子类型预留10-20个编号

### 2. 新增错误码流程

1. 在本文档中添加错误码定义
2. 在C++端 `Constants.h` 中添加常量
3. 在Java端 `ErrorCode.java` 枚举中添加
4. 在Qt端 `ErrorHandler` 中添加本地化消息
5. 提交PR并通知团队成员

### 3. 错误日志记录

所有错误都应记录到日志:

```cpp
logError("订单创建失败 [" + std::string(Constants::INSUFFICIENT_STOCK_CODE) + 
         "]: 商品ID=" + std::to_string(productId) + 
         ", 需求=" + std::to_string(required) + 
         ", 库存=" + std::to_string(available));
```

### 4. 多语言支持

未来支持英文时,在 `error_messages_en.json` 中添加:

```json
{
  "E10201": "Invalid username or password",
  "E20201": "Insufficient stock",
  "E40202": "Invalid order status for this operation"
}
```

---

## 测试建议

### 单元测试

每个错误码都应有对应的单元测试:

```java
@Test
void testInsufficientStockError() {
    // 构造库存不足场景
    String response = createOrderWithInsufficientStock();
    
    // 验证错误码
    assertThat(response).contains("\"code\":\"E20201\"");
    assertThat(response).contains("\"success\":false");
}
```

### 集成测试

测试错误在多层之间的传递:

1. C++ JNI层返回错误码
2. Java服务端转发错误码
3. Qt客户端正确显示本地化消息

---

## 常见错误场景快速查询

| 场景 | 错误码 | 处理建议 |
|------|-------|---------|
| 用户未登录 | E10301 | 跳转到登录页面 |
| Token过期 | E10303 | 刷新Token或重新登录 |
| 库存不足 | E20201 | 提示用户减少数量或稍后再试 |
| 订单不存在 | E40401 | 刷新订单列表 |
| 支付失败 | E50201 | 重试或更换支付方式 |
| 优惠券已用 | E60206 | 移除优惠券,使用其他优惠 |

---

**文档版本**: v1.0.0  
**最后更新**: 2025-01-13  
**维护者**: JLU Emshop 开发团队

**变更日志**:
- 2025-01-13: 初始版本,定义90个错误码,覆盖10个模块
