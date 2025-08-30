# JLU Emshop System - Netty版本使用说明

## 项目结构

```
JLU_Emshop_System/
├── java/
│   ├── EmshopNettyServer.java    # Netty服务器端
│   ├── EmshopNettyClient.java    # Netty客户端
│   ├── EmshopServer.java         # 业务逻辑处理类
│   └── MockDataProvider.java    # 模拟数据提供者
├── pom.xml                       # Maven配置文件
├── run.bat                       # Windows启动脚本
└── README_Netty.md              # 本说明文档
```

## 功能特性

### 服务器端 (EmshopNettyServer.java)
- 基于Netty NIO框架，支持高并发连接
- 使用Boss-Worker线程模型
- 支持TCP长连接和Keep-Alive
- 自动协议解析和业务分发
- 优雅关闭和资源管理

### 客户端 (EmshopNettyClient.java)
- 支持异步连接和消息处理
- 交互式命令行界面
- 自动重连机制
- 用户友好的命令提示

### 业务处理 (EmshopServer.java)
- 完整的协议分发系统
- 支持所有电商业务功能
- Native方法与C++后端集成
- 错误处理和参数验证

## 安装和运行

### 方法1：使用启动脚本（推荐）
```bash
# 运行启动脚本
run.bat

# 按提示选择：
# 1. 首次使用选择"1"编译项目
# 2. 选择"2"启动服务器
# 3. 新开命令窗口，选择"3"启动客户端
```

### 方法2：使用Maven命令
```bash
# 编译项目
mvn clean compile

# 启动服务器（端口8080）
mvn exec:java@server

# 启动客户端（连接localhost:8080）
mvn exec:java@client
```

### 方法3：指定端口和地址
```bash
# 启动服务器到指定端口
java -cp target/classes EmshopNettyServer 9999

# 客户端连接到指定服务器
java -cp target/classes EmshopNettyClient 192.168.1.100 9999
```

## 支持的命令

### 用户管理
```
LOGIN <username> <password>              # 用户登录
REGISTER <username> <password> <phone>   # 用户注册
LOGOUT <userId>                          # 用户登出
GET_USER_INFO <userId>                   # 获取用户信息
```

### 商品管理
```
GET_PRODUCTS <category> <page> <pageSize>  # 获取商品列表
GET_PRODUCT_DETAIL <productId>             # 获取商品详情
ADD_PRODUCT <jsonProduct>                  # 添加商品（管理员）
UPDATE_PRODUCT <productId> <jsonProduct>   # 更新商品
DELETE_PRODUCT <productId>                 # 删除商品
```

### 购物车
```
ADD_TO_CART <userId> <productId> <quantity>  # 加入购物车
GET_CART <userId>                           # 获取购物车内容
REMOVE_FROM_CART <userId> <productId>       # 移除购物车商品
```

### 订单管理
```
CHECKOUT <userId>                          # 结算购物车
GET_ORDERS <userId>                        # 获取订单列表
GET_ORDER_DETAIL <orderId>                 # 获取订单详情
CANCEL_ORDER <userId> <orderId>            # 取消订单
```

### 系统命令
```
GET_SERVER_STATUS                          # 获取服务器状态
HELP                                       # 显示帮助信息
```

## 使用示例

### 1. 启动服务器
```
Server started on port 8080
Emshop Netty Server started on port: 8080
```

### 2. 客户端连接和使用
```
Connected to Emshop Server at localhost:8080
Server: Welcome to Emshop Server!

=== Emshop Client Console ===
Available commands:
LOGIN <username> <password>
REGISTER <username> <password> <phone>
...
Type 'quit' to exit
=============================

# 用户输入示例：
LOGIN admin 123456
Server: Login successful for user: admin

GET_PRODUCTS electronics 1 10  
Server: [Product list for electronics category...]

ADD_TO_CART 1001 2001 3
Server: Added 3 items of product 2001 to cart for user 1001
```

## 网络协议

### 消息格式
- 使用文本协议，以换行符（\n）分隔消息
- 命令格式：`COMMAND param1 param2 ...`
- 响应格式：服务器返回处理结果字符串

### 连接管理
- 支持TCP长连接
- 客户端断线自动清理资源
- 支持多客户端并发连接

## 开发和扩展

### 添加新的业务命令
1. 在`EmshopServer.java`的`dispatch`方法中添加新的case分支
2. 添加对应的native方法声明
3. 在C++端实现具体的业务逻辑

### 修改网络协议
1. 修改`EmshopNettyServer`和`EmshopNettyClient`中的编解码器
2. 当前使用字符串协议，可替换为JSON、Protobuf等

### 性能调优
1. 调整Boss和Worker线程池大小
2. 修改Channel缓冲区大小
3. 启用压缩和加密

## 故障排除

### 常见问题

1. **连接被拒绝**
   - 确保服务器已启动
   - 检查端口是否被占用
   - 验证防火墙设置

2. **编译错误**
   - 确保已安装Java 8+和Maven
   - 运行`mvn clean compile`重新编译

3. **Native方法错误**
   - 当前版本使用模拟数据
   - 需要C++动态库支持完整功能

### 日志查看
- 服务器和客户端都会在控制台输出详细日志
- 可以重定向到文件：`java ... > server.log 2>&1`

## 技术架构

### Netty特性
- NIO非阻塞I/O
- 事件驱动架构
- 零拷贝数据传输
- 内存池管理

### 线程模型
- Boss Group：处理连接接受
- Worker Group：处理I/O读写
- 业务线程池：处理具体业务逻辑

### 扩展性
- 支持水平扩展（多服务器实例）
- 支持负载均衡
- 支持集群部署

---

**注意**：本版本是基于Netty的重构版本，保持了与原有业务逻辑的兼容性，同时提供了更好的性能和扩展性。
