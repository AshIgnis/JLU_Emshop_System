# Emshop WebSocket 服务器使用指南

## 概述

本项目为 JLU Emshop 系统添加了 WebSocket (WSS) 支持，允许 Qt 客户端使用 QWebSocket 进行实时异步通信。

## 架构组件

### 1. 服务器端 (Java + Netty)

#### 核心类

- **EmshopWebSocketServer**: 主要的 WebSocket 服务器类
- **WebSocketMessageHandler**: 处理 WebSocket 消息的业务逻辑
- **WebSocketPushService**: 管理实时推送和订阅功能

#### 功能特性

- 支持 WSS (WebSocket Secure) 连接
- 基于 JSON 的消息格式
- 用户认证和会话管理
- 实时消息推送
- 心跳检测和自动重连
- 多客户端连接管理

### 2. 客户端 (Qt + QWebSocket)

#### 核心类

- **EmshopClient**: WebSocket 客户端封装
- **LoginDialog**: 登录界面
- **MainWindow**: 主窗口
- **ProductListWidget**: 商品列表组件
- **CartWidget**: 购物车组件
- **OrderWidget**: 订单管理组件

## 服务器部署

### 1. 编译服务器

```bash
cd java
mvn clean compile
```

### 2. 启动 WebSocket 服务器

```bash
# 使用默认端口 8081 和 SSL
mvn exec:java -Dexec.mainClass="emshop.EmshopWebSocketServer" -Dexec.args="8081 true"

# 或者指定自定义端口和 SSL 设置
mvn exec:java -Dexec.mainClass="emshop.EmshopWebSocketServer" -Dexec.args="8082 false"
```

### 3. 服务器配置

#### SSL 证书配置

服务器默认使用自签名证书，仅用于开发测试。生产环境中应该：

1. 获取真实的 SSL 证书
2. 修改 `EmshopWebSocketServer.initSSL()` 方法
3. 配置证书文件路径

#### 端口配置

- 默认端口: 8081
- 可以通过命令行参数修改
- 确保防火墙开放相应端口

## 客户端构建

### 1. 环境要求

- Qt 6.2 或更高版本
- CMake 3.16 或更高版本
- C++17 支持

### 2. 构建步骤

```bash
cd qt_client
mkdir build
cd build
cmake ..
cmake --build .
```

### 3. 运行客户端

```bash
./EmshopQtClient
```

## 使用说明

### 1. 连接服务器

1. 启动 Qt 客户端
2. 在登录对话框中输入服务器地址 (例如: `localhost:8081`)
3. 点击"连接"按钮
4. 等待连接成功提示

### 2. 用户认证

1. 连接成功后，输入用户名和密码
2. 点击"登录"按钮
3. 认证成功后将显示主界面

### 3. 使用功能

- **商品浏览**: 在"商品"标签页查看和搜索商品
- **购物车**: 在"购物车"标签页管理购物车
- **订单管理**: 在"订单"标签页查看订单状态

## 消息格式

### 客户端到服务器消息格式

```json
{
  "type": "message_type",
  "param1": "value1",
  "param2": "value2"
}
```

### 服务器到客户端响应格式

```json
{
  "type": "message_type",
  "success": true,
  "message": "操作结果描述",
  "timestamp": 1234567890,
  "data": {
    // 业务数据
  }
}
```

### 支持的消息类型

#### 认证相关

- `auth`: 用户认证
- `welcome`: 欢迎消息

#### 心跳检测

- `ping`: 客户端心跳
- `pong`: 服务器心跳响应

#### 商品管理

- `get_products`: 获取商品列表
- `search_products`: 搜索商品

#### 购物车

- `add_to_cart`: 添加到购物车
- `get_cart`: 获取购物车
- `remove_from_cart`: 从购物车移除

#### 订单管理

- `create_order`: 创建订单
- `get_user_orders`: 获取用户订单
- `get_order_detail`: 获取订单详情
- `process_payment`: 处理支付

#### 实时推送

- `order_status_update`: 订单状态更新
- `stock_update`: 库存更新
- `price_update`: 价格更新
- `system_notification`: 系统通知

## 测试工具

### WebSocket 连接测试

```bash
# 编译测试工具
mvn compile

# 运行 WebSocket 测试客户端
mvn exec:java -Dexec.mainClass="emshop.WebSocketTestClient" -Dexec.args="wss://localhost:8081/ws admin admin123"
```

### 测试步骤

1. 连接测试
2. 认证测试
3. 心跳测试
4. 基本业务功能测试

## 故障排除

### 常见问题

1. **连接被拒绝**
   - 检查服务器是否正在运行
   - 确认端口号正确
   - 检查防火墙设置

2. **SSL 证书错误**
   - 在开发环境中，客户端会忽略自签名证书错误
   - 生产环境需要配置有效的 SSL 证书

3. **认证失败**
   - 确认用户名和密码正确
   - 检查数据库连接状态
   - 查看服务器日志

4. **消息无法发送**
   - 确认 WebSocket 连接状态
   - 检查消息格式是否正确
   - 查看客户端和服务器日志

### 日志查看

- 服务器日志: 控制台输出
- 客户端日志: Qt 调试输出
- 使用 `qDebug()` 查看详细信息

## 性能优化

### 服务器端

1. 调整线程池大小
2. 优化数据库查询
3. 实现连接池管理
4. 配置合适的心跳间隔

### 客户端

1. 实现消息缓存
2. 优化 UI 更新频率
3. 使用异步操作
4. 合理的重连策略

## 安全考虑

1. **SSL/TLS 加密**: 所有通信使用 WSS 加密
2. **用户认证**: JWT 令牌验证
3. **会话管理**: 安全的会话超时处理
4. **输入验证**: 客户端和服务器双重验证

## 扩展功能

### 计划中的功能

1. 多语言支持
2. 离线消息缓存
3. 文件上传支持
4. 语音和视频通话
5. 群组聊天功能

### 自定义扩展

1. 新增消息类型处理
2. 自定义推送逻辑
3. 集成第三方服务
4. 扩展 UI 组件

## 联系和支持

- 项目仓库: [GitHub Repository]
- 技术支持: [Support Email]
- 文档更新: 请查看最新版本