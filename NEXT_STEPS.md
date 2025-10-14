# 业务逻辑改进 - 下一步操作指南

> **当前进度**: 数据库设计和C++服务层已完成(40%)  
> **下一阶段**: 数据库升级 → JNI适配 → Java API实现 → Qt UI改进

---

## 📋 快速开始

### 步骤 1: 执行数据库升级 ⭐ 必做

**时间**: 5分钟

```bash
# 进入cpp目录
cd d:\codehome\jlu\JLU_Emshop_System\cpp

# 执行升级脚本(替换your_password为实际密码)
run_database_upgrade.bat your_password

# 或者手动执行(如果批处理文件有问题)
# 方式1: 使用PowerShell
Get-Content database_upgrade_v1.1.0.sql | mysql -u root -pyour_password emshop

# 方式2: 使用MySQL客户端
mysql -u root -p
use emshop;
source d:/codehome/jlu/JLU_Emshop_System/cpp/database_upgrade_v1.1.0.sql
```

### 步骤 2: 验证数据库升级 ⭐ 必做

**时间**: 5分钟

```sql
-- 1. 检查新表是否创建(应该有4张新表)
SHOW TABLES LIKE '%notification%';     -- user_notifications
SHOW TABLES LIKE '%refund_request%';   -- refund_requests
SHOW TABLES LIKE '%stock_log%';        -- stock_logs
SHOW TABLES LIKE '%coupon_template%';  -- coupon_templates

-- 2. 检查orders表status枚举是否更新
SHOW COLUMNS FROM orders WHERE Field = 'status';
-- 应该包含: 'pending','confirmed','paid','shipped','delivered','completed','cancelled','refunded','refunding'

-- 3. 检查coupons表新字段
SHOW COLUMNS FROM coupons WHERE Field IN ('template_id', 'description', 'usage_instructions');

-- 4. 检查触发器
SHOW TRIGGERS WHERE `Table` = 'products';
-- 应该有: after_product_stock_update

-- 5. 检查存储过程
SHOW PROCEDURE STATUS WHERE Db = 'emshop' AND Name LIKE '%Log%' OR Name LIKE '%Distribute%';
-- 应该有: LogStockChange, DistributeCoupons, sp_create_refund_request

-- 6. 检查视图
SHOW FULL TABLES WHERE Table_type = 'VIEW';
-- 应该有: v_refund_details, v_stock_summary

-- 7. 测试视图查询
SELECT * FROM v_refund_details LIMIT 5;
SELECT * FROM v_stock_summary LIMIT 5;
```

**预期结果**: 
- ✅ 4张新表
- ✅ orders.status包含'refunding'
- ✅ coupons表有3个新字段
- ✅ 1个触发器
- ✅ 3个存储过程
- ✅ 2个视图

### 步骤 3: 测试触发器和存储过程 (可选)

**时间**: 10分钟

```sql
-- 测试库存变动触发器
-- 1. 查看当前库存
SELECT product_id, name, stock_quantity FROM products LIMIT 1;

-- 2. 手动减少库存到低于10(触发低库存警告)
UPDATE products SET stock_quantity = 8 WHERE product_id = 1;

-- 3. 检查是否生成了管理员通知
SELECT * FROM user_notifications WHERE type = 'system' ORDER BY created_at DESC LIMIT 5;

-- 4. 恢复库存
UPDATE products SET stock_quantity = 100 WHERE product_id = 1;

-- 测试LogStockChange存储过程
CALL LogStockChange(1, -5, 'manual_test', 'test', 0, 0);

-- 检查库存日志
SELECT * FROM stock_logs WHERE product_id = 1 ORDER BY created_at DESC LIMIT 5;

-- 测试DistributeCoupons存储过程
-- 首先确保有可用优惠券
SELECT coupon_id, code FROM coupons WHERE status = 'active' LIMIT 1;

-- 批量分配给用户1,2,3
CALL DistributeCoupons(1, '1,2,3');  -- 替换1为实际coupon_id

-- 检查分配结果
SELECT * FROM user_coupons WHERE coupon_id = 1;
```

---

## 🔧 详细实施步骤

### A. 修改 requestRefund 调用方 (P0 - 破坏性变更)

**问题**: `requestRefund` 方法签名已改变,所有调用处需要更新

**旧签名**:
```cpp
json requestRefund(long order_id, const std::string& reason)
```

**新签名**:
```cpp
json requestRefund(long order_id, long user_id, const std::string& reason)
```

**影响范围**:
1. `cpp/emshop_native_impl_oop.cpp` - JNI层
2. `java/src/main/java/emshop/EmshopNettyServer.java` - 路由层
3. Qt客户端调用(如果有直接调用)

**修改建议** (JNI层):

```cpp
// 在 emshop_native_impl_oop.cpp 中找到 requestRefund 的JNI函数

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_requestRefund
  (JNIEnv *env, jobject obj, jlong order_id, jlong user_id, jstring reason) {
    
    const char* reason_cstr = env->GetStringUTFChars(reason, nullptr);
    
    OrderService orderService;
    json result = orderService.requestRefund(order_id, user_id, std::string(reason_cstr));
    
    env->ReleaseStringUTFChars(reason, reason_cstr);
    
    std::string result_str = result.dump();
    return env->NewStringUTF(result_str.c_str());
}
```

**修改建议** (Java层):

```java
// EmshopNativeInterface.java
public native String requestRefund(long orderId, long userId, String reason);

// EmshopNettyServer.java
if (path.equals("/api/order/refund")) {
    long orderId = jsonParams.get("order_id").getAsLong();
    long userId = jsonParams.get("user_id").getAsLong();  // 新增
    String reason = jsonParams.get("reason").getAsString();
    
    String result = nativeInterface.requestRefund(orderId, userId, reason);
    // ...
}
```

### B. 添加 JNI 函数声明 (P0)

**文件**: `cpp/emshop_EmshopNativeInterface.h`

使用 `javah` 或 `javac -h` 重新生成JNI头文件:

```bash
cd java
# 如果使用JDK 8-10
javah -jni -cp target/classes -d ../cpp emshop.EmshopNativeInterface

# 如果使用JDK 11+
javac -h ../cpp src/main/java/emshop/EmshopNativeInterface.java
```

**手动添加方法**(如果自动生成不可行):

在 `emshop_native_impl_oop.cpp` 中添加:

```cpp
// ==================== 退款审核相关 ====================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_approveRefund
  (JNIEnv *env, jobject obj, jlong refund_id, jlong admin_id, jboolean approve, jstring admin_reply) {
    
    const char* reply_cstr = env->GetStringUTFChars(admin_reply, nullptr);
    
    OrderService orderService;
    json result = orderService.approveRefund(refund_id, admin_id, approve, std::string(reply_cstr));
    
    env->ReleaseStringUTFChars(admin_reply, reply_cstr);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getRefundRequests
  (JNIEnv *env, jobject obj, jstring status, jint page, jint page_size) {
    
    const char* status_cstr = env->GetStringUTFChars(status, nullptr);
    
    OrderService orderService;
    json result = orderService.getRefundRequests(std::string(status_cstr), page, page_size);
    
    env->ReleaseStringUTFChars(status, status_cstr);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserRefundRequests
  (JNIEnv *env, jobject obj, jlong user_id) {
    
    OrderService orderService;
    json result = orderService.getUserRefundRequests(user_id);
    
    return env->NewStringUTF(result.dump().c_str());
}

// ==================== 通知管理相关 ====================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getNotifications
  (JNIEnv *env, jobject obj, jlong user_id, jboolean unread_only) {
    
    OrderService orderService;
    json result = orderService.getNotifications(user_id, unread_only);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_markNotificationRead
  (JNIEnv *env, jobject obj, jlong notification_id, jlong user_id) {
    
    OrderService orderService;
    json result = orderService.markNotificationRead(notification_id, user_id);
    
    return env->NewStringUTF(result.dump().c_str());
}

// ==================== 优惠券增强相关 ====================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getAvailableCouponsForOrder
  (JNIEnv *env, jobject obj, jlong user_id, jdouble order_amount) {
    
    CouponService couponService;
    json result = couponService.getAvailableCouponsForOrder(user_id, order_amount);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_calculateCouponDiscount
  (JNIEnv *env, jobject obj, jstring coupon_code, jdouble order_amount) {
    
    const char* code_cstr = env->GetStringUTFChars(coupon_code, nullptr);
    
    CouponService couponService;
    json result = couponService.calculateCouponDiscount(std::string(code_cstr), order_amount);
    
    env->ReleaseStringUTFChars(coupon_code, code_cstr);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_createCouponActivity
  (JNIEnv *env, jobject obj, jstring name, jstring code, jstring type, jdouble value,
   jdouble min_amount, jint quantity, jstring start_date, jstring end_date, jlong template_id) {
    
    const char* name_cstr = env->GetStringUTFChars(name, nullptr);
    const char* code_cstr = env->GetStringUTFChars(code, nullptr);
    const char* type_cstr = env->GetStringUTFChars(type, nullptr);
    const char* start_cstr = env->GetStringUTFChars(start_date, nullptr);
    const char* end_cstr = env->GetStringUTFChars(end_date, nullptr);
    
    CouponService couponService;
    json result = couponService.createCouponActivity(
        std::string(name_cstr), std::string(code_cstr), std::string(type_cstr),
        value, min_amount, quantity, std::string(start_cstr), std::string(end_cstr), template_id
    );
    
    env->ReleaseStringUTFChars(name, name_cstr);
    env->ReleaseStringUTFChars(code, code_cstr);
    env->ReleaseStringUTFChars(type, type_cstr);
    env->ReleaseStringUTFChars(start_date, start_cstr);
    env->ReleaseStringUTFChars(end_date, end_cstr);
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getCouponTemplates
  (JNIEnv *env, jobject obj) {
    
    CouponService couponService;
    json result = couponService.getCouponTemplates();
    
    return env->NewStringUTF(result.dump().c_str());
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_distributeCouponsToUsers
  (JNIEnv *env, jobject obj, jstring coupon_code, jstring user_ids_json) {
    
    const char* code_cstr = env->GetStringUTFChars(coupon_code, nullptr);
    const char* ids_cstr = env->GetStringUTFChars(user_ids_json, nullptr);
    
    // 解析JSON数组
    json user_ids = json::parse(std::string(ids_cstr));
    
    CouponService couponService;
    json result = couponService.distributeCouponsToUsers(std::string(code_cstr), user_ids);
    
    env->ReleaseStringUTFChars(coupon_code, code_cstr);
    env->ReleaseStringUTFChars(user_ids_json, ids_cstr);
    
    return env->NewStringUTF(result.dump().c_str());
}
```

### C. 编译 JNI DLL (P0)

**时间**: 2分钟

```bash
cd cpp
build_oop_jni.bat
```

**预期结果**: 生成 `cpp/bin/emshop_native_impl_oop.dll`

**常见问题**:
- 编译错误: 检查`#include`路径和JDK路径
- 链接错误: 检查MySQL库路径
- 缺少JSON库: 确保`nlohmann_json.hpp`在cpp目录

### D. 更新 Java 接口和路由 (P0)

#### D1. 更新 EmshopNativeInterface.java

**文件**: `java/src/main/java/emshop/EmshopNativeInterface.java`

在类中添加以下native方法声明:

```java
// ==================== 退款审核相关 ====================

/**
 * 审核退款申请(管理员功能)
 */
public native String approveRefund(long refundId, long adminId, boolean approve, String adminReply);

/**
 * 获取退款申请列表(管理员功能)
 */
public native String getRefundRequests(String status, int page, int pageSize);

/**
 * 获取用户的退款申请
 */
public native String getUserRefundRequests(long userId);

// ==================== 通知管理相关 ====================

/**
 * 获取用户通知列表
 */
public native String getNotifications(long userId, boolean unreadOnly);

/**
 * 标记通知为已读
 */
public native String markNotificationRead(long notificationId, long userId);

// ==================== 优惠券增强相关 ====================

/**
 * 获取订单可用优惠券列表
 */
public native String getAvailableCouponsForOrder(long userId, double orderAmount);

/**
 * 计算优惠券折扣金额
 */
public native String calculateCouponDiscount(String couponCode, double orderAmount);

/**
 * 创建优惠券活动(管理员功能)
 */
public native String createCouponActivity(String name, String code, String type, 
    double value, double minAmount, int quantity, String startDate, String endDate, long templateId);

/**
 * 获取优惠券模板列表
 */
public native String getCouponTemplates();

/**
 * 批量分配优惠券给用户
 */
public native String distributeCouponsToUsers(String couponCode, String userIdsJson);
```

#### D2. 更新 EmshopNettyServer.java

**文件**: `java/src/main/java/emshop/EmshopNettyServer.java`

在 `processRequest` 方法中添加新路由:

```java
// ==================== 退款管理路由 ====================

// POST /api/refund/approve - 审核退款申请
if (path.equals("/api/refund/approve") && "POST".equals(method)) {
    long refundId = jsonParams.get("refund_id").getAsLong();
    long adminId = jsonParams.get("admin_id").getAsLong();
    boolean approve = jsonParams.get("approve").getAsBoolean();
    String adminReply = jsonParams.has("admin_reply") ? 
        jsonParams.get("admin_reply").getAsString() : "";
    
    String result = nativeInterface.approveRefund(refundId, adminId, approve, adminReply);
    sendJsonResponse(ctx, result);
    return;
}

// GET /api/refund/requests - 获取退款申请列表
if (path.equals("/api/refund/requests") && "GET".equals(method)) {
    String status = getQueryParam(uri, "status", "all");
    int page = Integer.parseInt(getQueryParam(uri, "page", "1"));
    int pageSize = Integer.parseInt(getQueryParam(uri, "page_size", "20"));
    
    String result = nativeInterface.getRefundRequests(status, page, pageSize);
    sendJsonResponse(ctx, result);
    return;
}

// GET /api/refund/user/{userId} - 获取用户退款申请
if (path.startsWith("/api/refund/user/") && "GET".equals(method)) {
    long userId = Long.parseLong(path.substring("/api/refund/user/".length()));
    
    String result = nativeInterface.getUserRefundRequests(userId);
    sendJsonResponse(ctx, result);
    return;
}

// ==================== 通知管理路由 ====================

// GET /api/notifications - 获取通知列表
if (path.equals("/api/notifications") && "GET".equals(method)) {
    long userId = Long.parseLong(getQueryParam(uri, "user_id"));
    boolean unreadOnly = Boolean.parseBoolean(getQueryParam(uri, "unread_only", "false"));
    
    String result = nativeInterface.getNotifications(userId, unreadOnly);
    sendJsonResponse(ctx, result);
    return;
}

// POST /api/notification/read - 标记通知已读
if (path.equals("/api/notification/read") && "POST".equals(method)) {
    long notificationId = jsonParams.get("notification_id").getAsLong();
    long userId = jsonParams.get("user_id").getAsLong();
    
    String result = nativeInterface.markNotificationRead(notificationId, userId);
    sendJsonResponse(ctx, result);
    return;
}

// ==================== 优惠券增强路由 ====================

// GET /api/coupon/available-for-order - 获取订单可用优惠券
if (path.equals("/api/coupon/available-for-order") && "GET".equals(method)) {
    long userId = Long.parseLong(getQueryParam(uri, "user_id"));
    double orderAmount = Double.parseDouble(getQueryParam(uri, "order_amount"));
    
    String result = nativeInterface.getAvailableCouponsForOrder(userId, orderAmount);
    sendJsonResponse(ctx, result);
    return;
}

// POST /api/coupon/calculate-discount - 计算优惠券折扣
if (path.equals("/api/coupon/calculate-discount") && "POST".equals(method)) {
    String couponCode = jsonParams.get("coupon_code").getAsString();
    double orderAmount = jsonParams.get("order_amount").getAsDouble();
    
    String result = nativeInterface.calculateCouponDiscount(couponCode, orderAmount);
    sendJsonResponse(ctx, result);
    return;
}

// POST /api/coupon/activity/create - 创建优惠券活动
if (path.equals("/api/coupon/activity/create") && "POST".equals(method)) {
    String name = jsonParams.get("name").getAsString();
    String code = jsonParams.get("code").getAsString();
    String type = jsonParams.get("type").getAsString();
    double value = jsonParams.get("value").getAsDouble();
    double minAmount = jsonParams.get("min_amount").getAsDouble();
    int quantity = jsonParams.get("quantity").getAsInt();
    String startDate = jsonParams.get("start_date").getAsString();
    String endDate = jsonParams.get("end_date").getAsString();
    long templateId = jsonParams.has("template_id") ? 
        jsonParams.get("template_id").getAsLong() : 0;
    
    String result = nativeInterface.createCouponActivity(name, code, type, 
        value, minAmount, quantity, startDate, endDate, templateId);
    sendJsonResponse(ctx, result);
    return;
}

// GET /api/coupon/templates - 获取优惠券模板
if (path.equals("/api/coupon/templates") && "GET".equals(method)) {
    String result = nativeInterface.getCouponTemplates();
    sendJsonResponse(ctx, result);
    return;
}

// POST /api/coupon/distribute - 批量分配优惠券
if (path.equals("/api/coupon/distribute") && "POST".equals(method)) {
    String couponCode = jsonParams.get("coupon_code").getAsString();
    String userIdsJson = jsonParams.get("user_ids").toString();  // JSON数组
    
    String result = nativeInterface.distributeCouponsToUsers(couponCode, userIdsJson);
    sendJsonResponse(ctx, result);
    return;
}
```

**辅助方法**(如果不存在):

```java
private String getQueryParam(String uri, String paramName) {
    return getQueryParam(uri, paramName, null);
}

private String getQueryParam(String uri, String paramName, String defaultValue) {
    try {
        QueryStringDecoder decoder = new QueryStringDecoder(uri);
        List<String> values = decoder.parameters().get(paramName);
        if (values != null && !values.isEmpty()) {
            return values.get(0);
        }
    } catch (Exception e) {
        // ignore
    }
    return defaultValue;
}
```

### E. 编译和运行 Java 服务器 (P0)

**时间**: 2分钟

```bash
cd java
mvn clean compile
mvn exec:java -Dexec.mainClass="emshop.EmshopNettyServer"
```

**测试API**(使用 `curl` 或 Postman):

```bash
# 测试获取优惠券模板
curl http://localhost:8080/api/coupon/templates

# 测试计算优惠券折扣
curl -X POST http://localhost:8080/api/coupon/calculate-discount \
  -H "Content-Type: application/json" \
  -d '{"coupon_code":"SUMMER20","order_amount":100.0}'

# 测试获取通知
curl "http://localhost:8080/api/notifications?user_id=1&unread_only=false"
```

---

## 🎯 后续开发建议

### Phase 1: 核心功能完善 (1-2天)

1. **单元测试** - 为新增的13个方法编写单元测试
2. **集成测试** - 端到端测试退款和优惠券流程
3. **错误处理** - 完善异常捕获和错误码

### Phase 2: Qt UI 开发 (2-3天)

1. **用户端功能**:
   - 订单详情页添加"申请退款"按钮
   - 下单页面显示可用优惠券列表
   - 通知中心页面

2. **管理员功能**:
   - 退款审核页面
   - 优惠券活动创建向导
   - 批量分配优惠券界面

### Phase 3: 性能优化 (1天)

1. **数据库优化**:
   - 为高频查询添加索引
   - 优化视图查询性能

2. **并发优化**:
   - 测试高并发场景下的库存扣减
   - 优化锁策略

### Phase 4: 文档完善 (0.5天)

1. **API 文档** - 使用Swagger生成API文档
2. **用户手册** - 编写用户使用手册
3. **运维文档** - 编写部署和运维文档

---

## 📞 遇到问题?

### 数据库问题

**问题**: 数据库升级失败  
**解决**: 
1. 检查MySQL版本(需要8.0+)
2. 检查数据库用户权限
3. 逐条执行SQL查看哪条出错

### 编译问题

**问题**: JNI编译失败  
**解决**:
1. 检查JDK路径配置
2. 检查MySQL库路径
3. 查看详细编译输出

### 运行问题

**问题**: 调用新API返回404  
**解决**:
1. 检查路由是否正确添加
2. 检查DLL是否重新编译
3. 检查Java服务器是否重启

---

## ✅ 完成检查清单

- [ ] 数据库升级成功(4表+3过程+1触发器+2视图)
- [ ] 修改了 requestRefund 调用(添加user_id参数)
- [ ] 添加了11个JNI函数声明
- [ ] 编译了新的JNI DLL
- [ ] 更新了EmshopNativeInterface.java(11个native方法)
- [ ] 更新了EmshopNettyServer.java(11个路由)
- [ ] Java服务器启动成功
- [ ] 测试了至少3个新API
- [ ] 编写了基础单元测试
- [ ] 更新了项目文档

---

**最后更新**: 2025-01-13  
**维护人**: 开发团队
