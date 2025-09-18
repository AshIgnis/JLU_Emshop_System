# JLU 电商系统 - 项目状态总览

## 项目概述

这是一个基于Netty WebSocket的电商系统，包含Java后端服务器和Qt前端客户端，实现实时通信和在线购物功能。

## 项目结构

```
JLU_Emshop_System/
├── java/                    # Java后端 (Netty WebSocket服务器)
│   ├── src/main/java/emshop/
│   │   ├── EmshopWebSocketServer.java    # WebSocket服务器主类
│   │   ├── WebSocketMessageHandler.java  # 消息处理逻辑
│   │   ├── WebSocketPushService.java     # 推送服务
│   │   ├── WebSocketTestClient.java      # 测试客户端
│   │   └── ... (其他核心类)
│   └── target/                           # 编译输出
├── qt_client/             # Qt前端客户端
│   └── src/
│       ├── main.cpp                      # 程序入口
│       ├── MainWindow.cpp/h              # 主窗口
│       ├── LoginDialog.cpp/h             # 登录对话框
│       ├── EmshopClient.cpp/h            # WebSocket客户端
│       ├── ProductListWidget.cpp/h       # 商品列表组件
│       ├── CartWidget.cpp/h              # 购物车组件
│       └── OrderWidget.cpp/h             # 订单组件
├── cpp/                   # C++原生接口 (JNI)
└── web/                   # Web界面 (Spring Boot)
```

## 当前状态

### ✅ 已完成的功能

1. **WebSocket服务器**
   - 基于Netty 4.1.100实现
   - 支持WebSocket协议 (ws://)
   - 端口: 8081
   - 消息路由和处理逻辑完整

2. **Qt客户端**
   - 基于Qt 6.x实现
   - WebSocket连接功能正常
   - UI组件基本完整
   - 支持用户登录、商品浏览、购物车、订单管理

3. **连接测试**
   - ✅ WebSocket连接建立成功
   - ✅ 客户端可以连接到服务器
   - ✅ 服务器可以接收客户端消息 (subscribe, ping)

### ❌ 当前问题

1. **SSL配置问题**
   - 服务器启动时禁用SSL (`false`参数)
   - Qt客户端URL配置为`ws://`而非`wss://`
   - Java 21环境下自签名证书生成失败

2. **数据库连接**
   - JNI接口调用可能存在问题
   - 需要验证MySQL数据库连接状态
   - 用户认证依赖JNI调用`EmshopNativeInterface.login()`

3. **消息处理**
   - 服务器收到"unknown message type: subscribe"警告
   - subscribe消息类型未在服务器端完全实现
   - 心跳(ping)消息处理正常

## 测试进度

### 已测试项目
- [x] WebSocket服务器启动
- [x] Qt客户端编译
- [x] Qt客户端运行
- [x] WebSocket连接建立
- [x] 基础消息传输 (ping)

### 待测试项目
- [ ] 用户登录认证
- [ ] 商品列表获取
- [ ] 商品搜索功能
- [ ] 购物车操作 (添加/移除商品)
- [ ] 订单创建和管理
- [ ] 用户地址管理
- [ ] 支付处理
- [ ] 实时推送通知

## 关键配置

### 服务器配置
```bash
# 启动WebSocket服务器
cd java
java -cp "target\classes;target\dependency\*" emshop.EmshopWebSocketServer 8081 false
```

### 客户端配置
```cpp
// LoginDialog.cpp 中的服务器URL
url = "ws://" + url;  // 使用非SSL WebSocket
url += "/ws";         // WebSocket路径
```

### 数据库配置
- MySQL服务器
- 数据库名: emshop
- 需要验证JNI库加载和数据库连接

## 技术栈

- **后端**: Java 21, Netty 4.1.100, Jackson, SLF4J
- **前端**: Qt 6.x, QWebSocket, JSON处理
- **数据库**: MySQL (通过JNI接口)
- **通信**: WebSocket (JSON-RPC风格消息)
- **构建**: Maven (Java), CMake (Qt)

## 消息协议

### 请求消息格式
```json
{
  "type": "message_type",
  "username": "user",
  "password": "pass",
  // ... 其他字段
}
```

### 响应消息格式
```json
{
  "type": "message_type",
  "success": true/false,
  "message": "response message",
  "data": { /* response data */ },
  "timestamp": 1234567890
}
```

### 支持的消息类型

**公共消息** (无需认证):
- `auth` - 用户认证
- `ping` - 心跳检测

**需要认证的消息**:
- `get_products` - 获取商品列表
- `search_products` - 搜索商品
- `add_to_cart` - 添加到购物车
- `get_cart` - 获取购物车
- `remove_from_cart` - 从购物车移除
- `create_order` - 创建订单
- `get_user_orders` - 获取用户订单
- `get_order_detail` - 获取订单详情
- `process_payment` - 处理支付
- `get_user_addresses` - 获取用户地址
- `add_address` - 添加地址

**推送消息**:
- `order_status_update` - 订单状态更新
- `stock_update` - 库存更新
- `price_update` - 价格更新
- `system_notification` - 系统通知
- `promotion_notification` - 促销通知

## 问题排查清单

### 1. 服务器启动问题
```bash
# 检查端口占用
netstat -ano | findstr :8081

# 检查Java版本
java -version

# 检查依赖项
ls target/dependency/
```

### 2. 客户端连接问题
```bash
# 检查防火墙设置
# 确认服务器IP和端口正确
# 验证URL格式 (ws:// vs wss://)
```

### 3. 数据库连接问题
```bash
# 检查MySQL服务状态
# 验证JNI库文件存在
# 检查数据库配置
```

### 4. 消息处理问题
```bash
# 使用WebSocketTestClient测试
java -cp "target\classes;target\dependency\*" emshop.WebSocketTestClient ws://localhost:8081/ws
```

## 下一步计划

1. **修复认证流程**
   - 验证JNI接口调用
   - 测试用户登录功能
   - 检查数据库连接

2. **完善消息处理**
   - 实现subscribe消息处理
   - 添加更多业务逻辑验证

3. **功能测试**
   - 端到端测试所有业务功能
   - 性能和并发测试

4. **部署准备**
   - SSL证书配置 (生产环境)
   - 错误处理和日志完善
   - 文档更新

## 联系信息

项目维护者: AshIgnis
仓库: https://github.com/AshIgnis/JLU_Emshop_System

---

*最后更新: 2025年9月18日*</content>
<parameter name="filePath">d:\codehome\jlu\JLU_Emshop_System\PROJECT_STATUS_README.md