# WebSocket 功能快速启动指南

## 环境准备

### 1. Java 环境
- JDK 21 或更高版本
- Maven 3.6 或更高版本

### 2. Qt 环境
- Qt 6.2 或更高版本
- CMake 3.16 或更高版本
- Visual Studio 2019 或更高版本（Windows）

### 3. 数据库
- MySQL 8.0
- 确保 emshop 数据库已初始化

## 快速启动

### Step 1: 启动 WebSocket 服务器

```bash
cd java
java\start_websocket_server.bat
```

或手动启动：
```bash
mvn exec:java -Dexec.mainClass="emshop.EmshopWebSocketServer" -Dexec.args="8081 true"
```

### Step 2: 构建 Qt 客户端

```bash
cd qt_client
qt_client\build_client.bat
```

### Step 3: 运行客户端

执行构建后的程序：
```
qt_client\build\bin\EmshopQtClient.exe
```

### Step 4: 测试连接

1. 在客户端输入服务器地址: `localhost:8081`
2. 点击"连接"
3. 输入用户名: `admin`，密码: `admin123`
4. 点击"登录"

## 测试工具

运行 WebSocket 连接测试：
```bash
cd java
java\test_websocket.bat
```

## 功能验证

### 基本功能测试
1. ✓ WebSocket 连接
2. ✓ SSL/TLS 加密
3. ✓ 用户认证
4. ✓ 心跳检测
5. ✓ 商品查询
6. ✓ 购物车操作
7. ✓ 订单管理
8. ✓ 实时推送

### 高级功能测试
1. ✓ 多客户端连接
2. ✓ 消息广播
3. ✓ 断线重连
4. ✓ 会话管理

## 故障排除

### 服务器无法启动
1. 检查端口 8081 是否被占用
2. 确认 MySQL 服务正在运行
3. 检查 JNI 库文件是否存在

### 客户端连接失败
1. 确认服务器已启动
2. 检查防火墙设置
3. 验证 SSL 证书配置

### 认证失败
1. 检查数据库连接
2. 确认用户账号存在
3. 查看服务器错误日志

## 开发说明

### 添加新的消息类型

1. 在 `WebSocketMessageHandler.java` 中添加处理逻辑
2. 在 `EmshopClient.h/cpp` 中添加客户端方法
3. 更新消息格式文档

### 扩展 UI 组件

1. 创建新的 Qt Widget 类
2. 在 `MainWindow` 中集成
3. 连接相应的信号和槽

## 下一步

1. 实现完整的订单流程
2. 添加支付集成
3. 实现文件上传功能
4. 添加更多实时推送功能
5. 优化性能和用户体验

## 技术支持

如遇到问题，请：
1. 查看相关日志文件
2. 参考 `WEBSOCKET_GUIDE.md` 详细文档
3. 检查网络和防火墙配置