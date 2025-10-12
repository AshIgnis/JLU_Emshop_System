# 日志系统集成总结

## 概述

本文档总结了EmshopNettyServer的日志系统集成工作,包括集成的组件、修改的代码位置和关键功能。

## 集成日期

2025-01-13

## 集成组件

### 1. 日志框架
- **SLF4J 1.7.36**: 日志门面接口
- **Logback 1.2.12**: 日志实现框架
- **配置文件**: `java/src/main/resources/logback.xml`

### 2. 日志工具类
- **TraceIdUtil**: 分布式追踪ID管理
  - 位置: `java/src/main/java/emshop/TraceIdUtil.java`
  - 功能: 生成UUID格式的trace ID,存储到MDC

- **BusinessLogger**: 业务操作日志记录
  - 位置: `java/src/main/java/emshop/BusinessLogger.java`
  - 功能: 记录关键业务操作(登录、注册、订单、支付等)

- **ErrorCode**: 错误码枚举
  - 位置: `java/src/main/java/emshop/ErrorCode.java`
  - 功能: 统一错误码定义,支持中英文消息

## 核心修改

### EmshopNettyServer.java 修改详情

#### 1. 引入Logger
```java
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class EmshopNettyServer {
    private static final Logger logger = LoggerFactory.getLogger(EmshopNettyServer.class);
    // ...
}

private static class EmshopServerHandler extends SimpleChannelInboundHandler<String> {
    private static final Logger handlerLogger = LoggerFactory.getLogger(EmshopServerHandler.class);
    // ...
}
```

#### 2. 服务器启动和关闭
- **位置**: `start()` 和 `shutdown()` 方法
- **修改**: 
  - 替换 `System.out.println` 为 `logger.info()`
  - 添加参数化日志输出

**示例**:
```java
logger.info("Emshop Netty Server started successfully - port={}", port);
logger.info("Emshop Netty Server shutdown completed");
```

#### 3. 连接生命周期
- **位置**: `channelActive()` 和 `channelInactive()` 方法
- **修改**:
  - 添加 `TraceIdUtil.initTraceId()` 初始化trace ID
  - 使用 `handlerLogger.info()` 记录连接状态
  - 添加 `TraceIdUtil.clear()` 清理MDC
  - 登出时调用 `BusinessLogger.logLogout()`

**示例**:
```java
@Override
public void channelActive(ChannelHandlerContext ctx) {
    TraceIdUtil.initTraceId();
    try {
        String remoteAddr = ctx.channel().remoteAddress().toString();
        handlerLogger.info("Client connected - remoteAddress={}", remoteAddr);
        userSessions.put(ctx.channel().id(), new UserSession());
        ctx.writeAndFlush("Welcome to Emshop Server! Please login to access features.\n");
    } finally {
        TraceIdUtil.clear();
    }
}
```

#### 4. 请求处理
- **位置**: `channelRead0()` 方法
- **修改**:
  - 每个请求初始化独立的trace ID
  - 使用 `handlerLogger.debug()` 记录请求/响应
  - 使用 `handlerLogger.error()` 记录异常
  - finally块清理MDC

**示例**:
```java
@Override
protected void channelRead0(ChannelHandlerContext ctx, String request) {
    TraceIdUtil.initTraceId();
    try {
        String remoteAddr = ctx.channel().remoteAddress().toString();
        handlerLogger.debug("Received request - remoteAddress={}, request={}", remoteAddr, request);
        String response = processRequest(ctx, request.trim());
        handlerLogger.debug("Sending response - remoteAddress={}, response={}", remoteAddr, response);
        ctx.writeAndFlush(response + "\n");
    } catch (Exception e) {
        handlerLogger.error("Error processing request - error={}", e.getMessage(), e);
        ctx.writeAndFlush("ERROR: " + e.getMessage() + "\n");
    } finally {
        TraceIdUtil.clear();
    }
}
```

#### 5. 用户认证
- **位置**: `LOGIN` 和 `REGISTER` case
- **修改**:
  - 登录成功后调用 `TraceIdUtil.setUserContext()`
  - 使用 `handlerLogger.info()` 记录成功
  - 使用 `handlerLogger.warn()` 记录失败
  - 调用 `BusinessLogger.logLogin()` / `logRegister()`

**LOGIN示例**:
```java
case "LOGIN":
    if (parts.length >= 3) {
        String username = parts[1];
        String loginResult = EmshopNativeInterface.login(username, parts[2]);
        
        if (loginResult.contains("\"success\":true")) {
            try {
                long userId = extractUserIdFromResponse(loginResult);
                String role = extractRoleFromResponse(loginResult);
                
                session = new UserSession(userId, username, role);
                userSessions.put(ctx.channel().id(), session);
                TraceIdUtil.setUserContext(userId, username);
                
                handlerLogger.info("User login successful - username={}, userId={}, role={}", 
                    username, userId, role);
                BusinessLogger.logLogin(userId, username, true);
            } catch (Exception e) {
                handlerLogger.error("Failed to parse login response - username={}, error={}", 
                    username, e.getMessage(), e);
            }
        } else {
            handlerLogger.warn("User login failed - username={}", username);
            BusinessLogger.logLogin(-1, username, false);
        }
        return loginResult;
    }
    break;
```

**REGISTER示例**:
```java
case "REGISTER":
    if (parts.length >= 4) {
        String regUsername = parts[1];
        String regResult = EmshopNativeInterface.register(regUsername, parts[2], parts[3]);
        
        if (regResult.contains("\"success\":true")) {
            handlerLogger.info("User registration successful - username={}", regUsername);
            BusinessLogger.logRegister(-1, regUsername, true);
        } else {
            handlerLogger.warn("User registration failed - username={}", regUsername);
            BusinessLogger.logRegister(-1, regUsername, false);
        }
        return regResult;
    }
    break;
```

#### 6. 订单管理
- **位置**: `CREATE_ORDER` 和 `CANCEL_ORDER` case
- **修改**:
  - 调用 `BusinessLogger.logOrderCreate()` / `logOrderCancel()`
  - 记录订单ID、用户ID等关键信息

**CREATE_ORDER示例**:
```java
case "CREATE_ORDER":
    if (parts.length >= 2) {
        if (session != null && session.getUserId() != -1) {
            long addressId = Long.parseLong(parts[1]);
            String couponCode = parts.length > 2 && !parts[2].equals("0") ? parts[2] : null;
            String remark = parts.length > 3 ? parts[3] : "";
            String createResult = EmshopNativeInterface.createOrderFromCart(
                session.getUserId(), addressId, couponCode, remark);
            
            if (createResult.contains("\"success\":true")) {
                String orderId = HumanReadable.extract(createResult, "order_id");
                handlerLogger.info("Order created successfully - userId={}, orderId={}, addressId={}, couponCode={}", 
                    session.getUserId(), orderId, addressId, couponCode);
                BusinessLogger.logOrderCreate(session.getUserId(), session.getUsername(), 
                    orderId != null ? orderId : "unknown", true);
            } else {
                handlerLogger.warn("Order creation failed - userId={}", session.getUserId());
                BusinessLogger.logOrderCreate(session.getUserId(), session.getUsername(), 
                    "unknown", false);
            }
            return createResult;
        }
    }
    // ... 兼容旧版本代码
    break;
```

**CANCEL_ORDER示例**:
```java
case "CANCEL_ORDER":
    if (parts.length >= 2) {
        long orderId;
        long userId;
        // ... 解析参数逻辑
        
        String cancelResult = EmshopNativeInterface.cancelOrder(userId, orderId);
        if (cancelResult.contains("\"success\":true")) {
            handlerLogger.info("Order cancelled successfully - userId={}, orderId={}", userId, orderId);
            BusinessLogger.logOrderCancel(userId, 
                session != null ? session.getUsername() : "user_" + userId, 
                String.valueOf(orderId), true);
        } else {
            handlerLogger.warn("Order cancellation failed - userId={}, orderId={}", userId, orderId);
            BusinessLogger.logOrderCancel(userId, 
                session != null ? session.getUsername() : "user_" + userId, 
                String.valueOf(orderId), false);
        }
        return cancelResult;
    }
    break;
```

#### 7. 支付处理
- **位置**: `PROCESS_PAYMENT` case
- **修改**:
  - 调用 `BusinessLogger.logPayment()`
  - 记录订单ID、金额、支付方式

**示例**:
```java
case "PROCESS_PAYMENT":
    if (parts.length >= 4) {
        long orderId = Long.parseLong(parts[1]);
        String paymentMethod = parts[2];
        double amount = Double.parseDouble(parts[3]);
        String paymentDetails = parts.length > 4 ? parts[4] : "{}";
        
        String paymentResult = EmshopNativeInterface.processPayment(orderId, paymentMethod, amount, paymentDetails);
        if (paymentResult.contains("\"success\":true")) {
            long userId = session != null ? session.getUserId() : -1;
            handlerLogger.info("Payment processed successfully - userId={}, orderId={}, amount={}, method={}", 
                userId, orderId, amount, paymentMethod);
            BusinessLogger.logPayment(userId, 
                session != null ? session.getUsername() : "unknown", 
                String.valueOf(orderId), amount, paymentMethod, true);
        } else {
            long userId = session != null ? session.getUserId() : -1;
            handlerLogger.warn("Payment processing failed - userId={}, orderId={}, amount={}, method={}", 
                userId, orderId, amount, paymentMethod);
            BusinessLogger.logPayment(userId, 
                session != null ? session.getUsername() : "unknown", 
                String.valueOf(orderId), amount, paymentMethod, false);
        }
        return paymentResult;
    }
    break;
```

#### 8. 异常处理
- **位置**: `processRequest()` 和 `processJsonRequest()` catch块
- **修改**:
  - 替换 `System.err.println` 为 `handlerLogger.error()`
  - 包含堆栈跟踪

**示例**:
```java
} catch (UnsatisfiedLinkError e) {
    handlerLogger.error("Missing native method - error={}", e.getMessage(), e);
    return "{\"success\":false,\"message\":\"Server native module not available for request\",\"error_code\":500}";
} catch (NumberFormatException e) {
    handlerLogger.error("Invalid number format - error={}", e.getMessage(), e);
    return "{\"success\":false,\"message\":\"Invalid number format: " + e.getMessage() + "\"}";
} catch (Exception e) {
    handlerLogger.error("Error processing request - error={}", e.getMessage(), e);
    return "{\"success\":false,\"message\":\"Error processing request: " + e.getMessage() + "\"}";
} catch (Throwable t) {
    handlerLogger.error("Unexpected server error - error={}", t.getMessage(), t);
    return "{\"success\":false,\"message\":\"Server internal error\",\"error_code\":500}";
}
```

#### 9. Main方法
- **位置**: `main()` 方法
- **修改**:
  - 使用 `logger.info()` 记录启动信息
  - 使用 `logger.error()` 记录错误
  - 添加shutdown hook日志

**示例**:
```java
public static void main(String[] args) {
    int port = 8081;
    if (args.length > 0) {
        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            logger.error("Invalid port number, using default port 8081 - error={}", e.getMessage());
            port = 8081;
        }
    }

    EmshopNettyServer server = new EmshopNettyServer(port);
    
    Runtime.getRuntime().addShutdownHook(new Thread(() -> {
        logger.info("Shutdown hook triggered, stopping server...");
        server.shutdown();
    }));
    
    try {
        logger.info("Starting Emshop Netty Server on port {}...", port);
        server.start();
    } catch (InterruptedException e) {
        logger.error("Server interrupted - error={}", e.getMessage(), e);
        Thread.currentThread().interrupt();
    }
}
```

## 日志输出位置

### 1. 控制台日志
- **配置**: logback.xml中的CONSOLE appender
- **输出级别**: INFO及以上
- **格式**: `[时间] [级别] [线程] [类名:行号] - 消息`

### 2. INFO日志文件
- **位置**: `logs/emshop-info.log`
- **配置**: INFO_FILE appender (异步)
- **滚动策略**: 50MB/文件,保留30天
- **内容**: INFO及以上级别的所有日志

### 3. ERROR日志文件
- **位置**: `logs/emshop-error.log`
- **配置**: ERROR_FILE appender (异步)
- **滚动策略**: 50MB/文件,保留30天
- **内容**: ERROR级别的日志

### 4. 业务日志文件
- **位置**: `logs/emshop-business.log`
- **配置**: BUSINESS_FILE appender (异步)
- **滚动策略**: 50MB/文件,保留90天
- **内容**: BusinessLogger记录的业务操作

## MDC上下文变量

所有日志都包含以下MDC变量(如果可用):

- **traceId**: 32字符UUID格式的追踪ID
- **userId**: 用户ID
- **username**: 用户名

## 日志级别使用规范

### DEBUG级别
- 请求接收和响应发送
- 详细的参数信息

### INFO级别
- 服务器启动和关闭
- 客户端连接和断开
- 用户登录成功
- 订单创建成功
- 支付成功

### WARN级别
- 登录失败
- 注册失败
- 订单创建失败

### ERROR级别
- 异常捕获
- JNI调用失败
- 数据解析错误
- 系统错误

## 业务日志记录

### 已集成的业务操作

1. **用户认证**
   - `BusinessLogger.logLogin()`: 登录
   - `BusinessLogger.logRegister()`: 注册
   - `BusinessLogger.logLogout()`: 登出 (在channelInactive中)

2. **订单管理**
   - `BusinessLogger.logOrderCreate()`: 订单创建
   - `BusinessLogger.logOrderCancel()`: 订单取消

3. **支付系统**
   - `BusinessLogger.logPayment()`: 支付处理

### 待集成的业务操作

以下业务操作在BusinessLogger中已定义,但尚未在服务器代码中集成:

- `logRefundRequest()`: 退款请求
- `logStockChange()`: 库存变化
- `logCouponUse()`: 优惠券使用

## 统计数据

### 修改范围
- **修改文件**: 1个 (EmshopNettyServer.java)
- **新增导入**: 2个 (Logger, LoggerFactory)
- **logger实例**: 2个 (类级别和Handler级别)
- **替换System.out.println**: 6处
- **替换System.err.println**: 13处
- **新增TraceIdUtil调用**: 6处
- **新增BusinessLogger调用**: 8处
- **总代码行数变化**: +约120行

### 集成位置
- channelActive: 添加trace ID初始化
- channelInactive: 添加trace ID和业务日志
- channelRead0: 添加trace ID和请求/响应日志
- LOGIN (2处): 添加业务日志和用户上下文
- REGISTER: 添加业务日志
- CREATE_ORDER (2处): 添加业务日志
- CANCEL_ORDER: 添加业务日志
- PROCESS_PAYMENT: 添加业务日志
- 异常处理 (6处): 替换为结构化日志
- main方法: 添加启动和错误日志

## 测试建议

### 1. 基础功能测试
```bash
# 启动服务器
cd java
mvn clean compile exec:java -Dexec.mainClass="emshop.EmshopNettyServer"

# 检查日志文件是否创建
ls logs/
# 应该看到: emshop-info.log, emshop-error.log, emshop-business.log
```

### 2. 追踪ID测试
- 连接服务器并执行操作
- 检查日志文件,验证同一请求的日志具有相同的traceId
- 验证不同请求具有不同的traceId

### 3. 业务日志测试
- 执行登录、注册、订单创建等操作
- 检查 `emshop-business.log` 文件
- 验证业务日志包含所有必要字段

### 4. 错误日志测试
- 触发各种异常场景
- 检查 `emshop-error.log` 文件
- 验证堆栈跟踪完整性

## 性能考虑

- 使用异步appender避免日志IO阻塞业务线程
- 队列大小: 256
- discardingThreshold: 0 (不丢弃日志)
- 所有文件appender都使用异步包装

## 后续工作

### 待完成
1. 集成退款请求日志
2. 集成库存变化日志
3. 集成优惠券使用日志
4. 添加性能监控日志(响应时间等)
5. 添加数据库查询日志

### 优化建议
1. 考虑使用ELK堆栈进行日志聚合
2. 添加日志采样机制(高流量场景)
3. 实现日志脱敏(敏感信息)
4. 添加日志压缩
5. 配置日志告警规则

## 参考文档

- [LOGGING_GUIDE.md](LOGGING_GUIDE.md): 完整日志使用指南
- [ERROR_CODES.md](ERROR_CODES.md): 错误码规范
- [Logback官方文档](http://logback.qos.ch/manual/)
- [SLF4J用户手册](http://www.slf4j.org/manual.html)

## 版本历史

- **v1.0.0** (2025-01-13): 初始集成
  - 完成所有System.out/err替换
  - 集成TraceIdUtil和BusinessLogger
  - 添加核心业务操作日志
