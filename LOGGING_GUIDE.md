# 日志系统使用指南

## 概述

本项目已完成日志系统统一化,使用 **SLF4J + Logback** 作为Java端的日志框架。

**核心特性**:
- ✅ 统一日志格式(时间戳、线程、Trace ID、级别、类名、消息)
- ✅ 日志分级输出(INFO/ERROR分离,异步写入提升性能)
- ✅ Trace ID追踪(MDC实现,支持全链路追踪)
- ✅ 业务日志独立(便于业务分析和统计)
- ✅ 用户上下文(自动记录userId和username)

---

## 日志配置

### 生产环境配置

**位置**: `src/main/resources/logback.xml`

**日志输出**:
1. **控制台**: 实时查看,开发环境使用
2. **INFO文件**: `logs/emshop-info.log` (仅INFO级别)
3. **ERROR文件**: `logs/emshop-error.log` (ERROR及以上)
4. **业务日志**: `logs/emshop-business.log` (业务操作记录)

**日志滚动策略**:
- 单文件最大50MB
- 按天滚动
- INFO/ERROR保留30天
- 业务日志保留90天

### 测试环境配置

**位置**: `src/test/resources/logback-test.xml`

测试时自动使用,日志级别为DEBUG,保留7天。

---

## 基础使用

### 1. 普通日志记录

```java
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MyService {
    private static final Logger logger = LoggerFactory.getLogger(MyService.class);
    
    public void doSomething() {
        logger.debug("调试信息: 参数={}", param);
        logger.info("普通信息: 操作完成");
        logger.warn("警告信息: 库存不足");
        logger.error("错误信息: 数据库连接失败", exception);
    }
}
```

### 2. Trace ID使用

在请求入口处初始化Trace ID:

```java
import emshop.TraceIdUtil;

// 在处理请求时
String traceId = TraceIdUtil.initTraceId();  // 自动生成并设置

// 后续所有日志自动包含此Trace ID
logger.info("处理请求");  // 输出包含 [traceId=abc123...]

// 请求结束时清理
TraceIdUtil.clear();
```

### 3. 用户上下文

登录后设置用户信息:

```java
// 用户登录成功后
TraceIdUtil.setUserContext(userId, username);

// 后续日志自动包含用户信息
logger.info("创建订单");  // 输出包含 [traceId=...] [userId=123] [username=张三]

// 退出登录时清理
TraceIdUtil.clearUserContext();
```

---

## 业务日志

使用 `BusinessLogger` 记录关键业务操作:

### 用户操作

```java
import emshop.BusinessLogger;

// 用户登录
BusinessLogger.logLogin(userId, username, clientIp, true);

// 用户注册
BusinessLogger.logRegister(userId, username, email, clientIp);
```

### 订单操作

```java
// 订单创建
BusinessLogger.logOrderCreate(orderId, userId, username, totalAmount, finalAmount);

// 订单取消
BusinessLogger.logOrderCancel(orderId, userId, username, "用户主动取消");

// 退款申请
BusinessLogger.logRefundRequest(orderId, userId, username, "商品质量问题");
```

### 库存操作

```java
// 库存变动
BusinessLogger.logStockChange(
    productId, 
    productName, 
    -2,              // 扣减2个
    100,             // 扣减前100
    98,              // 扣减后98
    "order_deduct",  // 变动类型
    "订单创建扣减"
);
```

### 支付操作

```java
// 支付成功/失败
BusinessLogger.logPayment(orderId, userId, "alipay", 99.00, true);
```

### 其他业务

```java
// 优惠券使用
BusinessLogger.logCouponUse(couponId, couponCode, userId, username, orderId, discountAmount);

// 商品查看
BusinessLogger.logProductView(productId, productName, userId);

// 购物车操作
BusinessLogger.logCartOperation("add", userId, productId, quantity);

// 业务异常
BusinessLogger.logBusinessError("createOrder", "E40201", "库存不足", userId, details);
```

---

## Netty服务端集成

### 在ChannelHandler中使用

```java
public class EmshopServerHandler extends SimpleChannelInboundHandler<String> {
    private static final Logger logger = LoggerFactory.getLogger(EmshopServerHandler.class);
    
    @Override
    protected void channelRead0(ChannelHandlerContext ctx, String msg) {
        // 1. 初始化Trace ID
        String traceId = TraceIdUtil.initTraceId();
        
        try {
            logger.info("收到客户端请求: {}", msg.substring(0, Math.min(100, msg.length())));
            
            // 2. 解析请求,提取用户信息
            // ... 处理登录逻辑,获取userId和username
            
            // 3. 设置用户上下文
            TraceIdUtil.setUserContext(userId, username);
            
            // 4. 处理业务逻辑
            String response = handleRequest(msg);
            
            // 5. 记录业务日志
            BusinessLogger.logLogin(userId, username, getClientIp(ctx), true);
            
            logger.info("响应发送成功");
            ctx.writeAndFlush(response + "\n");
            
        } catch (Exception e) {
            logger.error("处理请求异常: traceId={}", traceId, e);
            ctx.writeAndFlush("{\"success\":false,\"message\":\"系统异常\"}\n");
        } finally {
            // 6. 清理MDC
            TraceIdUtil.clear();
        }
    }
    
    private String getClientIp(ChannelHandlerContext ctx) {
        return ctx.channel().remoteAddress().toString();
    }
}
```

### 改造EmshopNettyServer

将现有的 `System.out.println` 替换为日志:

**改造前**:
```java
System.out.println("收到消息: " + msg);
```

**改造后**:
```java
logger.info("收到消息: {}", msg);
```

---

## 日志级别使用建议

### DEBUG (调试级别)
- 详细的程序执行流程
- 方法入参和返回值
- 临时调试信息

**示例**:
```java
logger.debug("进入方法 createOrder, 参数: userId={}, items={}", userId, items);
```

### INFO (信息级别)
- 重要的业务流程节点
- 系统状态变更
- 正常的操作完成

**示例**:
```java
logger.info("订单创建成功: orderId={}, amount={}", orderId, amount);
```

### WARN (警告级别)
- 可恢复的错误
- 业务异常(库存不足、优惠券已用等)
- 性能问题警告

**示例**:
```java
logger.warn("库存不足: productId={}, required={}, available={}", 
           productId, required, available);
```

### ERROR (错误级别)
- 系统级错误
- 数据库连接失败
- 未预期的异常

**示例**:
```java
logger.error("数据库连接失败: {}", e.getMessage(), e);
```

---

## 日志格式

### 标准日志格式

```
2025-10-12 15:30:45.123 [nio-thread-1] [abc123def456] INFO  emshop.EmshopServerHandler - 收到客户端请求
│                        │                │            │     │                          │
│                        │                │            │     │                          └─ 日志消息
│                        │                │            │     └─ Logger名称(类名)
│                        │                │            └─ 日志级别
│                        │                └─ Trace ID (32位UUID)
│                        └─ 线程名
└─ 时间戳
```

### 业务日志格式

```
2025-10-12 15:30:45.123 [nio-thread-1] [abc123def456] [123] [张三] - 用户登录成功 | userId=123 | username=张三 | ip=127.0.0.1
│                        │                │            │     │      │
│                        │                │            │     │      └─ 业务消息(结构化)
│                        │                │            │     └─ 用户名
│                        │                │            └─ 用户ID
│                        │                └─ Trace ID
│                        └─ 线程名
└─ 时间戳
```

---

## 日志查询

### 按Trace ID查询

查找某个请求的所有日志:

```bash
# Linux/Mac
grep "abc123def456" logs/emshop-info.log

# Windows PowerShell
Select-String -Path "logs\emshop-info.log" -Pattern "abc123def456"
```

### 按用户ID查询

查找某个用户的操作日志:

```bash
grep "\[123\]" logs/emshop-business.log
```

### 查询错误日志

```bash
tail -f logs/emshop-error.log
```

### 查询最近的业务日志

```bash
tail -100 logs/emshop-business.log
```

---

## 性能优化

### 1. 异步日志

配置中已启用异步Appender:

```xml
<appender name="ASYNC_INFO" class="ch.qos.logback.classic.AsyncAppender">
    <discardingThreshold>0</discardingThreshold>
    <queueSize>512</queueSize>
    <appender-ref ref="INFO_FILE"/>
</appender>
```

**优点**:
- 日志写入不阻塞主线程
- 提升系统吞吐量
- 队列大小512,足够应对突发流量

### 2. 参数化日志

**推荐**:
```java
logger.info("用户{}创建订单{}", username, orderId);
```

**不推荐**:
```java
logger.info("用户" + username + "创建订单" + orderId);  // 字符串拼接消耗性能
```

### 3. 条件判断

对于复杂的日志(如对象序列化),使用条件判断:

```java
if (logger.isDebugEnabled()) {
    logger.debug("订单详情: {}", JsonUtil.toJson(order));
}
```

---

## 日志分析

### 统计登录次数

```bash
grep "用户登录成功" logs/emshop-business.log | wc -l
```

### 统计错误次数

```bash
grep "ERROR" logs/emshop-error.log | wc -l
```

### 统计最活跃用户

```bash
# 提取userId并统计
grep "userId=" logs/emshop-business.log | \
  sed -n 's/.*userId=\([0-9]*\).*/\1/p' | \
  sort | uniq -c | sort -rn | head -10
```

### 统计库存不足次数

```bash
grep "库存不足" logs/emshop-info.log | wc -l
```

---

## 最佳实践

### 1. 日志内容

✅ **推荐**:
```java
logger.info("订单创建成功: orderId={}, userId={}, amount={}", orderId, userId, amount);
```

❌ **不推荐**:
```java
logger.info("订单创建成功");  // 缺少关键信息
```

### 2. 异常记录

✅ **推荐**:
```java
try {
    // 业务逻辑
} catch (SQLException e) {
    logger.error("数据库操作失败: sql={}", sql, e);  // 包含异常堆栈
    throw new BusinessException("E00002", "数据库查询失败");
}
```

❌ **不推荐**:
```java
catch (SQLException e) {
    logger.error("数据库操作失败");  // 丢失异常信息
}
```

### 3. 敏感信息

❌ **禁止记录**:
- 用户密码
- 支付密码/PIN
- 银行卡号
- 身份证号完整信息

✅ **脱敏记录**:
```java
logger.info("用户注册: email={}", maskEmail(email));  // 脱敏后: abc***@example.com
```

### 4. 业务关键节点

**必须记录的业务操作**:
- 用户登录/注册
- 订单创建/支付/退款
- 库存变动
- 重要配置修改
- 权限变更

---

## 日志监控告警

### 错误日志告警

可集成ELK/Prometheus/Grafana实现:

1. **错误率超过阈值**: 1分钟内ERROR日志>10条
2. **特定错误码**: 数据库连接失败(E00001)
3. **业务异常**: 库存不足频繁出现

### 示例配置(Logback + Sentry)

```xml
<appender name="SENTRY" class="io.sentry.logback.SentryAppender">
    <filter class="ch.qos.logback.classic.filter.ThresholdFilter">
        <level>ERROR</level>
    </filter>
</appender>
```

---

## C++端日志(待实现)

未来计划在C++ JNI层也实现统一日志:

```cpp
// 伪代码示例
LogManager::info("订单创建成功", {
    {"orderId", orderId},
    {"userId", userId},
    {"amount", amount}
});
```

---

## 常见问题

### 1. 日志文件过大

**问题**: `emshop-info.log` 增长过快

**解决**:
- 调整 `maxFileSize` 从50MB降低到20MB
- 缩短 `maxHistory` 从30天降低到7天
- 提升部分日志级别(DEBUG → INFO)

### 2. Trace ID丢失

**问题**: 部分日志没有Trace ID

**原因**: 异步线程没有继承MDC

**解决**:
```java
// 传递MDC到异步线程
Map<String, String> mdcContext = MDC.getCopyOfContextMap();
executor.submit(() -> {
    MDC.setContextMap(mdcContext);
    // 业务逻辑
    MDC.clear();
});
```

### 3. 日志乱码

**问题**: 日志文件中文乱码

**解决**: 确保 `logback.xml` 中指定UTF-8编码:
```xml
<encoder>
    <charset>UTF-8</charset>
</encoder>
```

---

## 迁移指南

### 替换System.out.println

**查找**: 在Java代码中搜索 `System.out.println`

**替换**:
```java
// 之前
System.out.println("收到消息: " + msg);

// 之后
logger.info("收到消息: {}", msg);
```

### 添加Logger声明

每个类添加:
```java
private static final Logger logger = LoggerFactory.getLogger(当前类名.class);
```

---

**文档版本**: v1.0.0  
**最后更新**: 2025-10-12  
**维护者**: JLU Emshop 开发团队
