# JLU Emshop 电商系统# JLU  2024级卓越工程师班面向对象程序课程设计



> 吉林大学 2024级卓越工程师班 - 面向对象程序课程设计项目## 0.概述



[![项目状态](https://img.shields.io/badge/状态-已完成-success)](https://github.com/AshIgnis/JLU_Emshop_System)### 1.背景描述

[![技术栈](https://img.shields.io/badge/技术栈-C%2B%2B%20%7C%20Java%20%7C%20Qt-blue)](https://github.com/AshIgnis/JLU_Emshop_System)

[![完成度](https://img.shields.io/badge/完成度-95%25-brightgreen)](https://github.com/AshIgnis/JLU_Emshop_System)以交易为主要内容，完成类似手机淘宝、京东的程序  

---### 2.C/S框架

## 📋 目录项目要求使用C/S框架  

服务器和客户端都要安装程序  

- [项目概述](#-项目概述)可参考MVC模式：  

- [系统架构](#-系统架构)服务端和客户端之间要求以TCP/IP通信  

- [核心功能](#-核心功能)其中：

- [技术栈](#-技术栈)

- [快速开始](#-快速开始)#### 服务端

- [项目结构](#-项目结构)

- [开发文档](#-开发文档)分为UI、业务层、持久层

- [完成度评估](#-完成度评估)并与内置数据库链接

- [未来展望](#-未来展望)

#### 客户端

---

分为UI、业务层  

## 🎯 项目概述

- **高性能核心**: 使用 C++ JNI 实现核心业务逻辑,性能卓越

- **现代化界面**: Qt 6 框架打造美观流畅的桌面客户端

- **企业级后端**: Java + Netty 实现高并发网络通信

- **完整功能**: 涵盖用户、商品、订单、支付、优惠券、评价等完整业务流程

- **数据持久化**: MySQL 数据库确保数据安全可靠查看产品列表  

**购物车**  

### 设计理念促销（混合策略）（反射+策略+装修等模式）打折、降价、劵  

**结算**  

本项目严格遵循面向对象设计原则,采用分层架构、模块化设计、接口标准化等软件工程最佳实践,确保系统的可扩展性、可维护性和高性能。售后服务（如退货）  

**风格不同UI选择**  

---

#### 性能

## 🏗️ 系统架构

考虑并发，如双9.9  

### 整体架构**考虑互斥，如爱马仕限量**  

同步或异步通信  

```**分层设计体柔性**  

┌─────────────────────────────────────────────────────────┐需求+设计+实现 软件工程生命周期  

│                    Qt 桌面客户端 (C++)                    │

│  ┌──────────┬──────────┬──────────┬──────────┬────────┐ │#### 架构

│  │ 商品浏览  │  购物车  │  订单管理 │  用户中心 │ 管理员 │ │

│  └──────────┴──────────┴──────────┴──────────┴────────┘ │![架构流程](/img/架构.jpg "架构流程")  

└───────────────────────┬─────────────────────────────────┘

                        │ TCP/IP 通信#### 服务端-业务层

                        ↓

┌─────────────────────────────────────────────────────────┐![服务端业务流程](/img/service.jpg "服务端业务")

│              Java 服务端 (Netty + JNI)                   │

│  ┌──────────────────────────────────────────────────┐   │#### 服务端-持久层

│  │           Netty TCP 服务器                        │   │

│  │  • 会话管理  • 命令路由  • 并发控制               │   │数据库是非面向对象的  

│  └─────────────────────┬────────────────────────────┘   │**持久层完成映射**  

│                        │ JNI 调用                        │windows体系,java体系  

│                        ↓                                 │**需要SQL语言**  

│  ┌──────────────────────────────────────────────────┐   │

│  │         C++ 核心业务逻辑 (8194 行)                │   │#### 服务端-数据库

│  │  ┌──────────┬──────────┬──────────┬──────────┐   │   │

│  │  │用户服务  │商品服务  │订单服务  │优惠券服务│   │   │数据库的本质是文件存储  

│  │  ├──────────┼──────────┼──────────┼──────────┤   │   │**库**  

│  │  │购物车服务│评价服务  │地址服务  │支付服务  │   │   │表（列，行）  

│  │  └──────────┴──────────┴──────────┴──────────┘   │   │**支持SQL语言**  

│  └─────────────────────┬────────────────────────────┘   │保存商品信息，用户信息，销售情况等

└────────────────────────┼────────────────────────────────┘

                         │ MySQL Driver#### 客户端细节

                         ↓

               ┌──────────────────┐UI实现全部商品展示，用户交互  

               │   MySQL 数据库    │**购物车+结算+折扣+售后**

               │  • 用户数据       │发起交易请求  

               │  • 商品数据       │**展现交易结果**

               │  • 订单数据       │

               │  • 交易记录       │#### TCP/IP通信

               └──────────────────┘

```协议编码  

**双向（请求+应答）**  

### 技术架构分层**可以定长也可不定长**  

实例：  

| 层次 | 技术 | 职责 |> 1001（商品展示），10020001（1号商品加入购物车），1003（结算当前购物车）;  

|------|------|------|> 20020001（1号商品加入购物车成功），2003结算成功  

| **表示层** | Qt 6.9.1 (C++) | 用户界面、交互逻辑、数据展示 |

| **通信层** | Netty 4.x (Java) | TCP 网络通信、会话管理、消息路由 |### 5.约定  

| **业务层** | C++ (JNI) | 核心业务逻辑、数据处理、事务控制 |

| **持久层** | MySQL 8.0 | 数据存储、查询优化、事务管理 |坚决不建议web编程  

PC下实现即可，不考虑手机  

---**不考虑多店，但要考虑多用户**



## 🚀 核心功能## 1.基本方案



### 1. 用户管理系统 ✅### 项目核心理念



- **用户注册登录**: 安全的用户认证机制本项目采用面向对象的设计思想，构建一个完整的电商交易系统。以C/S架构为基础，实现服务端和客户端的分离设计，确保系统的可扩展性和维护性。

- **会话管理**: 基于 Token 的会话控制

- **个人信息管理**: 用户资料修改、密码管理### 技术栈选择

- **收货地址管理**: 多地址管理、默认地址设置

- **主体框架**：C++（服务端核心业务逻辑）

### 2. 商品管理系统 ✅- **数据处理**：Python（数据分析、日志处理、辅助工具）

- **界面开发**：Qt with C++（高性能GUI客户端）

- **商品浏览**: 分页展示、分类筛选、搜索功能- **Web服务**：Java（Spring Boot Web API服务）

- **商品详情**: 完整的商品信息展示(名称、价格、库存、描述)- **数据存储**：SQL（数据库操作和查询）

- **库存管理**: 实时库存监控、库存预警、库存同步

- **商品图片**: 支持商品图片展示## 2.项目流程



### 3. 购物车系统 ✅### 2.1 开发阶段



- **购物车操作**: 添加、删除、修改商品数量```text

- **实时计算**: 自动计算总价、优惠金额需求分析 → 系统设计 → 数据库设计 → 服务端开发 → 客户端开发 → 通信协议实现 → 测试调优 → 部署上线

- **批量管理**: 支持批量删除、全选操作```

- **持久化存储**: 购物车数据持久化保存

### 2.2 系统架构流程

### 4. 订单处理系统 ✅

1. **客户端启动** → 连接服务器 → 用户登录/注册

- **订单创建**: 从购物车快速生成订单2. **商品浏览** → 商品详情查看 → 加入购物车

- **订单管理**: 查看订单列表、订单详情3. **购物车管理** → 结算处理 → 支付确认

- **状态流转**: 完整的订单生命周期管理4. **订单生成** → 售后服务 → 交易完成

  - `pending` → `confirmed` → `paid` → `shipped` → `delivered` → `completed`

  - 支持 `cancelled` 和 `refunded` 状态### 2.3 技术实现流程

- **物流跟踪**: 快递单号记录、物流信息查询

- **订单搜索**: 按状态、时间筛选订单- **C++服务端**：处理核心业务逻辑、数据库连接、TCP/IP通信

- **Qt客户端**：实现用户界面、用户交互、网络通信

### 5. 支付系统 ✅- **Java Web服务**：提供RESTful API接口、Web管理后台

- **SQL数据库**：存储商品、用户、订单等数据

- **多支付方式**: 支付宝、微信支付、银行卡、信用卡- **Python脚本**：数据分析、系统监控、自动化测试

- **支付验证**: 金额验证、状态检查

- **交易记录**: 完整的支付流水记录## 3.项目流程

- **退款处理**: 订单退款、退款审核、退款金额计算

### 3.1 开发优先级

### 6. 优惠券系统 ✅

1. **第一阶段**：基础框架搭建

- **优惠券类型**: 固定金额券、折扣券、满减券   - C++服务端核心框架

- **优惠券管理**: 创建、发放、使用、过期管理   - 数据库设计和SQL脚本

- **用户优惠券**: 我的优惠券、可用优惠券筛选   - 基本的TCP/IP通信协议

- **自动计算**: 结算时自动应用最优惠券   - Java Web服务基础API



### 7. 评价系统 ✅2. **第二阶段**：核心功能实现

   - 用户管理系统

- **商品评价**: 用户可对已完成订单进行评价   - 商品管理系统

- **评分系统**: 5 星评分制度   - 购物车功能

- **评价展示**: 商品详情页展示用户评价   - Web管理后台界面

- **评价管理**: 评价审核、删除不当评价

3. **第三阶段**：高级功能

### 8. 管理员功能 ✅   - 促销策略系统

   - 并发处理

- **用户管理**: 查看用户列表、用户状态管理   - 售后服务

- **商品管理**: 商品上下架、库存调整、价格修改   - Web API完善和优化

- **订单管理**: 订单审核、发货操作、退款审批

- **系统统计**: 销售数据统计、用户活跃度分析4. **第四阶段**：优化完善

   - 性能优化

---   - 界面美化

   - 异常处理

## 💻 技术栈   - 系统集成测试



### 客户端 (Qt Desktop)### 3.2 架构设计



- **框架**: Qt 6.9.1 (C++17)- **模块化设计**：每个功能模块独立开发，便于维护

- **构建工具**: CMake 3.30+- **接口标准化**：定义清晰的API接口，确保模块间解耦

- **编译器**: MinGW-w64 GCC 15.2.0- **异常处理机制**：完善的错误处理和日志记录

- **UI 设计**: Qt Widgets + QSS 样式- **并发控制**：使用线程池和锁机制处理高并发情况

- **网络通信**: QTcpSocket

- **JSON 处理**: nlohmann/json### 3.3 数据库设计



### 服务端 (Java + C++)```sql

-- 核心表结构

#### Java 层用户表(users): user_id, username, password, phone, create_time

- **JDK**: OpenJDK 21商品表(products): product_id, name, price, stock, category, description

- **网络框架**: Netty 4.1.115订单表(orders): order_id, user_id, total_amount, status, create_time

- **构建工具**: Maven 3.9+订单详情表(order_items): item_id, order_id, product_id, price

- **日志框架**: Logback 1.5.12购物车表(cart): cart_id, user_id, product_id, add_time

- **JSON 处理**: Gson 2.11.0```



#### C++ 层 (JNI)## 4.UI语言推荐

- **编译器**: MinGW GCC 15.2.0

- **标准**: C++11### 4.1 选定方案：Qt with C++vs

- **数据库驱动**: MySQL Connector/C++ 8.0

- **JSON 处理**: nlohmann/json**选择理由**：

- **构建**: Windows DLL (JNI 动态库)

- 统一使用C++语言，技术栈一致性好

### 数据库- 高性能GUI框架，适合桌面应用

- 原生外观体验，界面美观

- **数据库**: MySQL 8.0- 丰富的控件和布局系统

- **字符集**: utf8mb4- 跨平台支持（Windows/Linux/macOS）

- **存储引擎**: InnoDB- 内建网络模块支持TCP/IP通信

- **表数量**: 15 张核心业务表

### 4.2 Qt框架优势

---

**技术优势**：

## 🎬 快速开始

- **信号槽机制**：事件驱动编程模式

### 前置要求- **Model/View架构**：数据展示与逻辑分离

- **样式表支持**：CSS-like样式定制

- Windows 10/11 64-bit- **多线程支持**：GUI与业务逻辑分离

- MySQL 8.0+ (已安装并运行)- **国际化支持**：多语言界面切换

- Java JDK 21+

- Qt 6.9.1+ (如需重新编译客户端)**开发优势**：



### 数据库初始化- **Qt Designer**：可视化界面设计工具

- **Qt Creator**：专业C++集成开发环境

```bash- **丰富文档**：完善的API文档和示例

# 1. 登录 MySQL- **活跃社区**：大量第三方组件和教程

mysql -u root -p

### 4.3 UI设计

# 2. 创建数据库

CREATE DATABASE emshop CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;- **响应式布局**：适应不同屏幕分辨率

- **模块化界面**：商品展示、购物车、用户中心等独立模块

# 3. 导入初始化脚本- **主题切换功能**：满足"风格不同UI选择"的需求

mysql -u root -p emshop < cpp/emshop_database_init.sql- **交互友好**：清晰的导航和操作反馈



# 4. (可选) 导入测试数据### 4.4 界面模块

mysql -u root -p emshop < cpp/init_coupons.sql

```1. **登录/注册界面**

2. **商品浏览界面**（支持分类、搜索、排序）

### 配置文件设置3. **商品详情界面**

4. **购物车界面**

编辑 `config.json`:5. **结算界面**

6. **订单管理界面**

```json7. **用户中心界面**

{8. **设置界面**（主题切换、系统设置）

  "database": {

    "host": "localhost",## 6.程序搭建基本流程

    "port": 3306,

    "username": "root",### 6.1 开发环境准备

    "password": "your_password",

    "database": "emshop"#### 服务器端环境

  },

  "server": {1. **安装C++开发工具**

    "host": "localhost",   - Visual Studio 2019/2022 或 MinGW-w64

    "port": 8888,   - CMake 3.20+（构建工具）

    "backlog": 128   - Git（版本控制）

  },

  "jni": {2. **安装数据库**

    "library_path": "./cpp/bin/emshop_native_impl_oop.dll"   - SQLite（轻量级，推荐开发阶段）

  }   - MySQL 8.0+（生产环境推荐）

}

```3. **安装第三方库**

   - **网络库**：Boost.Asio 或 原生Socket API

### 启动服务端   - **JSON库**：nlohmann/json 或 RapidJSON

   - **数据库驱动**：sqlite3 或 MySQL Connector/C++

```bash   - **日志库**：spdlog（可选）

# 方式 1: 使用启动脚本

start_backend.bat#### 客户端环境



# 方式 2: 手动启动1. **安装Qt开发环境**

cd java   - Qt 6.5+ LTS版本

java -jar target/emshop-server.jar   - Qt Creator IDE

```   - MinGW或MSVC编译器



服务端成功启动后会显示:2. **配置Qt模块**

```   - Qt Core（核心功能）

[INFO] Netty TCP 服务器已启动,监听端口: 8888   - Qt Widgets（GUI控件）

[INFO] JNI 本地库加载成功: emshop_native_impl_oop   - Qt Network（网络通信）

```   - Qt SQL（数据库支持，可选）



### 启动客户端### 6.2 服务器端搭建流程



```bash#### 第一步：项目结构创建

# 方式 1: 使用启动脚本

启动Qt客户端.bat```text

Server/

# 方式 2: 直接运行可执行文件├── src/

cd qtclient/build/src│   ├── main.cpp              # 服务器入口

emshop_qtclient.exe│   ├── server/

```│   │   ├── TCPServer.h/cpp   # TCP服务器类

│   │   └── ClientHandler.h/cpp # 客户端连接处理

### 默认登录账号│   ├── business/

│   │   ├── UserManager.h/cpp    # 用户管理

| 角色 | 用户名 | 密码 | 说明 |│   │   ├── ProductManager.h/cpp # 商品管理

|------|--------|------|------|│   │   └── OrderManager.h/cpp   # 订单管理

| 管理员 | admin | admin123 | 拥有全部管理权限 |│   ├── database/

| 普通用户 | test | test123 | 普通用户权限 |│   │   ├── DatabaseManager.h/cpp # 数据库管理

│   │   └── SQLExecutor.h/cpp     # SQL执行器

---│   └── protocol/

│       ├── Protocol.h           # 通信协议定义

## 📁 项目结构│       └── MessageParser.h/cpp  # 消息解析器

├── include/                     # 头文件目录

```├── lib/                        # 第三方库

JLU_Emshop_System/├── CMakeLists.txt              # CMake构建文件

├── cpp/                                # C++ JNI 核心业务层└── config.json                 # 配置文件

│   ├── services/                       # 业务服务模块```

│   │   ├── UserService.cpp/h          # 用户服务

│   │   ├── ProductService.cpp/h       # 商品服务#### 第二步：核心类设计

│   │   ├── CartService.cpp/h          # 购物车服务

│   │   ├── OrderService.cpp/h         # 订单服务```cpp

│   │   ├── CouponService.cpp/h        # 优惠券服务// TCPServer.h - 服务器核心类

│   │   ├── ReviewService.cpp/h        # 评价服务class TCPServer {

│   │   └── AddressService.cpp/h       # 地址服务public:

│   ├── emshop_native_impl_oop.cpp     # JNI 实现主文件    TCPServer(int port);

│   ├── ConfigLoader.h                  # 配置加载器    void start();

│   ├── ErrorCodes.h                    # 错误码定义    void stop();

│   ├── emshop_database_init.sql       # 数据库初始化脚本    

│   └── bin/                            # 编译产物private:

│       └── emshop_native_impl_oop.dll # JNI 动态库    void acceptConnections();

│    void handleClient(int clientSocket);

├── java/                               # Java 服务端    

│   ├── src/main/java/emshop/    int serverSocket;

│   │   ├── EmshopNettyServer.java     # Netty 服务器    bool isRunning;

│   │   ├── SessionManager.java        # 会话管理    std::vector<std::thread> clientThreads;

│   │   └── ...                        # 其他 Java 类};

│   ├── pom.xml                         # Maven 配置

│   └── target/// Protocol.h - 通信协议

│       └── emshop-server.jar          # 可执行 JARenum class MessageType {

│    LOGIN_REQUEST = 1001,

├── qtclient/                           # Qt 桌面客户端    PRODUCT_LIST_REQUEST = 1002,

│   ├── src/    ADD_TO_CART = 1003,

│   │   ├── main.cpp                   # 客户端入口    CHECKOUT = 1004,

│   │   ├── core/                      # 核心模块    

│   │   │   └── ApplicationContext.cpp # 应用上下文    LOGIN_RESPONSE = 2001,

│   │   ├── network/                   # 网络通信    PRODUCT_LIST_RESPONSE = 2002,

│   │   │   └── NetworkClient.cpp     # 网络客户端    CART_RESPONSE = 2003,

│   │   ├── ui/                        # 界面模块    CHECKOUT_RESPONSE = 2004

│   │   │   ├── MainWindow.cpp         # 主窗口};

│   │   │   ├── LoginDialog.cpp        # 登录对话框

│   │   │   ├── dialogs/               # 对话框struct Message {

│   │   │   │   ├── PaymentDialog.cpp  # 支付对话框    MessageType type;

│   │   │   │   └── AddressDialog.cpp  # 地址对话框    int userId;

│   │   │   └── tabs/                  # 标签页    std::string data;

│   │   │       ├── ProductsTab.cpp    # 商品页    int dataSize;

│   │   │       ├── CartTab.cpp        # 购物车页};

│   │   │       ├── OrdersTab.cpp      # 订单页```

│   │   │       ├── CouponsTab.cpp     # 优惠券页

│   │   │       ├── NotificationsTab.cpp # 通知页#### 第三步：数据库设计实现

│   │   │       ├── DashboardTab.cpp   # 仪表盘

│   │   │       └── AdminTab.cpp       # 管理员页```sql

│   │   └── utils/                     # 工具类-- 创建数据库表

│   │       └── JsonUtils.cpp          # JSON 工具CREATE TABLE users (

│   ├── CMakeLists.txt                 # CMake 配置    user_id INTEGER PRIMARY KEY AUTOINCREMENT,

│   └── build/src/    username VARCHAR(50) UNIQUE NOT NULL,

│       └── emshop_qtclient.exe       # 可执行文件    password VARCHAR(255) NOT NULL,

│    email VARCHAR(100),

├── config.json                         # 系统配置文件    phone VARCHAR(20),

├── config.example.json                 # 配置示例    create_time DATETIME DEFAULT CURRENT_TIMESTAMP

├── start_backend.bat                   # 后端启动脚本);

├── 启动Qt客户端.bat                    # 客户端启动脚本

├── cleanup_project.ps1                 # 项目清理脚本CREATE TABLE products (

├── cleanup_full.bat                    # 完全清理脚本    product_id INTEGER PRIMARY KEY AUTOINCREMENT,

├── ERROR_CODES.md                      # 错误码文档    name VARCHAR(200) NOT NULL,

├── PROJECT_CLEANUP_GUIDE.md           # 清理指南    description TEXT,

└── README.md                           # 本文件    price DECIMAL(10,2) NOT NULL,

```    stock INTEGER DEFAULT 0,

    category VARCHAR(50),

---    image_url VARCHAR(500),

    create_time DATETIME DEFAULT CURRENT_TIMESTAMP

## 📖 开发文档);



### 核心文档CREATE TABLE orders (

    order_id INTEGER PRIMARY KEY AUTOINCREMENT,

- [错误码定义](ERROR_CODES.md) - 系统错误码说明    user_id INTEGER NOT NULL,

- [项目清理指南](PROJECT_CLEANUP_GUIDE.md) - 清理无用文件指南    total_amount DECIMAL(10,2) NOT NULL,

    status VARCHAR(20) DEFAULT 'pending',

### 数据库设计    create_time DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(user_id)

#### 核心表结构);



| 表名 | 说明 | 记录数 |CREATE TABLE order_items (

|------|------|--------|    item_id INTEGER PRIMARY KEY AUTOINCREMENT,

| `users` | 用户基本信息 | 动态 |    order_id INTEGER NOT NULL,

| `products` | 商品信息 | 45+ |    product_id INTEGER NOT NULL,

| `cart` | 购物车 | 动态 |    quantity INTEGER NOT NULL,

| `orders` | 订单主表 | 动态 |    price DECIMAL(10,2) NOT NULL,

| `order_items` | 订单详情 | 动态 |    FOREIGN KEY (order_id) REFERENCES orders(order_id),

| `addresses` | 收货地址 | 动态 |    FOREIGN KEY (product_id) REFERENCES products(product_id)

| `reviews` | 商品评价 | 动态 |);

| `coupons` | 优惠券模板 | 10+ |

| `user_coupons` | 用户优惠券 | 动态 |CREATE TABLE cart (

| `notifications` | 系统通知 | 动态 |    cart_id INTEGER PRIMARY KEY AUTOINCREMENT,

    user_id INTEGER NOT NULL,

详细的表结构请查看 `cpp/emshop_database_init.sql`    product_id INTEGER NOT NULL,

    quantity INTEGER NOT NULL,

### 通信协议    add_time DATETIME DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id) REFERENCES users(user_id),

#### 消息格式    FOREIGN KEY (product_id) REFERENCES products(product_id)

);

``````

┌────────────────────────────────────────┐

│  命令字符串 (UTF-8)                     │### 6.3 Qt客户端搭建流程

│  格式: COMMAND_NAME [param1] [param2]  │

└────────────────────────────────────────┘#### 第一步：Qt项目创建

         ↓

┌────────────────────────────────────────┐1. **创建项目**

│  JSON 响应 (UTF-8)                      │   - 打开Qt Creator

│  {                                      │   - 选择"Application" → "Qt Widgets Application"

│    "code": 200,                         │   - 项目名：EmshopClient

│    "message": "success",                │   - 选择合适的套件（Kit）

│    "data": { ... }                      │

│  }                                      │2. **项目结构规划**

└────────────────────────────────────────┘

``````text

EmshopClient/

#### 主要命令├── main.cpp                    # 程序入口

├── mainwindow.h/cpp/ui        # 主窗口

| 命令 | 参数 | 说明 |├── ui/                        # 界面文件

|------|------|------|│   ├── loginwindow.h/cpp/ui   # 登录窗口

| `LOGIN` | username password | 用户登录 |│   ├── productview.h/cpp/ui   # 商品展示

| `REGISTER` | username password phone | 用户注册 |│   ├── cartwindow.h/cpp/ui    # 购物车窗口

| `GET_PRODUCTS` | [page] [category] | 获取商品列表 |│   └── orderwindow.h/cpp/ui   # 订单窗口

| `ADD_TO_CART` | product_id quantity | 添加到购物车 |├── network/

| `GET_CART` | - | 获取购物车 |│   ├── NetworkManager.h/cpp   # 网络管理类

| `CREATE_ORDER` | address_id [coupon_id] | 创建订单 |│   └── ProtocolHandler.h/cpp  # 协议处理

| `GET_ORDERS` | [status] [page] | 获取订单列表 |├── models/

| `PAY_ORDER` | order_id payment_method | 支付订单 |│   ├── User.h/cpp            # 用户模型

| `GET_USER_COUPONS` | - | 获取用户优惠券 |│   ├── Product.h/cpp         # 商品模型

│   └── Order.h/cpp           # 订单模型

更多命令请参考源代码中的命令处理逻辑。├── resources/

│   ├── images/               # 图片资源

---│   └── styles/               # 样式表文件

└── EmshopClient.pro          # 项目文件

## 📊 完成度评估```



### 综合完成度: **95%** ✅#### 第二步：网络通信模块



### 功能模块完成度```cpp

// NetworkManager.h - 网络管理类

| 模块 | 完成度 | 说明 |class NetworkManager : public QObject {

|------|--------|------|    Q_OBJECT

| 用户管理 | 100% ✅ | 注册、登录、信息管理、地址管理 |    

| 商品管理 | 100% ✅ | 浏览、搜索、分类、库存管理 |public:

| 购物车 | 100% ✅ | 增删改查、批量操作、持久化 |    NetworkManager(QObject *parent = nullptr);

| 订单系统 | 95% ✅ | 订单流转、状态管理、物流跟踪 |    

| 支付系统 | 90% ✅ | 多支付方式、退款处理(模拟支付) |    bool connectToServer(const QString &host, int port);

| 优惠券 | 100% ✅ | 创建、发放、使用、过期管理 |    void sendMessage(const Message &message);

| 评价系统 | 100% ✅ | 评价发布、查看、管理 |    void disconnectFromServer();

| 管理员 | 95% ✅ | 用户管理、商品管理、订单管理 |    

| UI 界面 | 98% ✅ | Qt 桌面客户端完整实现 |signals:

| 网络通信 | 100% ✅ | TCP 稳定通信、会话管理 |    void messageReceived(const Message &message);

    void connectionStatusChanged(bool connected);

### 代码质量统计    

private slots:

| 指标 | 数值 |    void onReadyRead();

|------|------|    void onDisconnected();

| C++ 核心代码 | 8,194 行 |    

| Java 服务端代码 | 2,500+ 行 |private:

| Qt 客户端代码 | 6,000+ 行 |    QTcpSocket *socket;

| 总代码量 | 16,000+ 行 |    bool isConnected;

| 数据库表 | 15 张 |};

| API 接口 | 80+ 个 |```



### 技术亮点#### 第三步：主要界面设计



✨ **高性能**: C++ JNI 核心,性能卓越  ```cpp

✨ **高并发**: Netty 框架支持大量并发连接  // MainWindow.h - 主窗口类

✨ **现代化 UI**: Qt 6 + QSS 打造美观界面  class MainWindow : public QMainWindow {

✨ **完整业务**: 覆盖电商核心业务流程      Q_OBJECT

✨ **规范设计**: 面向对象、分层架构、模块化      

✨ **易于维护**: 清晰的代码结构和文档public:

    MainWindow(QWidget *parent = nullptr);

### 已实现的核心需求    ~MainWindow();

    

✅ **C/S 框架**: 完整的客户端-服务器架构  private slots:

✅ **TCP/IP 通信**: 基于 Netty 的稳定通信      void onLoginSuccess();

✅ **购物车**: 完整的购物车功能      void showProductView();

✅ **结算**: 订单创建和支付流程      void showCartView();

✅ **促销**: 优惠券系统      void showOrderView();

✅ **售后服务**: 退货退款功能      void onThemeChanged(const QString &theme);

✅ **并发处理**: 支持多用户并发访问      

✅ **分层设计**: UI、业务层、持久层分离  private:

    void setupUI();

### 待完善功能(可选扩展)    void loadTheme(const QString &theme);

    

🔄 第三方支付接口对接(支付宝、微信真实 API)      Ui::MainWindow *ui;

🔄 第三方物流接口对接(实时物流查询)      NetworkManager *networkManager;

🔄 Web 管理后台(浏览器访问)      QStackedWidget *centralWidget;

🔄 复杂促销策略(满减、阶梯价格、组合优惠)      

🔄 移动端适配(Android/iOS)      // 各个视图窗口

    LoginWindow *loginWindow;

---    ProductView *productView;

    CartWindow *cartWindow;

## 🔮 未来展望    OrderWindow *orderWindow;

};

### 短期计划```



- [ ] 完善日志分析和监控系统### 6.4 Java Web服务搭建流程

- [ ] 添加数据备份和恢复功能

- [ ] 优化数据库查询性能#### 第一步：开发环境准备

- [ ] 增加单元测试和集成测试

1. **安装Java开发环境**

### 中期计划   - JDK 17+ (推荐使用LTS版本)

   - Maven 3.8+（依赖管理和构建工具）

- [ ] 开发 Web 管理后台   - IntelliJ IDEA 或 Eclipse（IDE）

- [ ] 实现 RESTful API 接口

- [ ] 对接真实的第三方支付2. **创建Spring Boot项目**

- [ ] 增加商品推荐算法   - 使用Spring Initializr创建项目

   - 选择依赖：Spring Web、Spring Data JPA、MySQL Driver、Spring Security

### 长期规划

#### 第二步：项目结构设计

- [ ] 微服务架构改造

- [ ] 容器化部署(Docker)```text

- [ ] 云平台部署(阿里云/腾讯云)EmshopWebService/

- [ ] 移动端 App 开发├── src/main/java/com/jlu/emshop/

│   ├── EmshopApplication.java          # 启动类

---│   ├── controller/

│   │   ├── UserController.java         # 用户管理API

## 👥 开发团队│   │   ├── ProductController.java      # 商品管理API

│   │   ├── OrderController.java        # 订单管理API

**项目负责人**: AshIgnis  │   │   └── AdminController.java        # 管理员API

**课程**: 面向对象程序设计  │   ├── service/

**学期**: 2024-2025 学年第一学期  │   │   ├── UserService.java           # 用户业务逻辑

**院校**: 吉林大学 软件学院  │   │   ├── ProductService.java        # 商品业务逻辑

│   │   └── OrderService.java          # 订单业务逻辑

---│   ├── repository/

│   │   ├── UserRepository.java        # 用户数据访问

## 📄 许可证│   │   ├── ProductRepository.java     # 商品数据访问

│   │   └── OrderRepository.java       # 订单数据访问

本项目仅用于课程设计学习,未经授权不得用于商业用途。│   ├── entity/

│   │   ├── User.java                  # 用户实体

---│   │   ├── Product.java               # 商品实体

│   │   ├── Order.java                 # 订单实体

## 🙏 致谢│   │   └── OrderItem.java             # 订单项实体

│   ├── dto/

感谢以下开源项目和技术社区:│   │   ├── UserDTO.java               # 用户数据传输对象

│   │   ├── ProductDTO.java            # 商品数据传输对象

- [Qt Framework](https://www.qt.io/) - 跨平台 C++ 图形界面框架│   │   └── OrderDTO.java              # 订单数据传输对象

- [Netty](https://netty.io/) - 高性能网络应用框架│   └── config/

- [MySQL](https://www.mysql.com/) - 开源关系型数据库│       ├── SecurityConfig.java        # 安全配置

- [nlohmann/json](https://github.com/nlohmann/json) - 现代 C++ JSON 库│       └── DatabaseConfig.java        # 数据库配置

- [GitHub Copilot](https://github.com/features/copilot) - AI 编程助手├── src/main/resources/

│   ├── application.yml                # 应用配置

---│   ├── data.sql                       # 初始化数据

│   └── static/                        # 静态资源

## 📞 联系方式│       └── admin/                     # 管理后台页面

└── pom.xml                           # Maven配置文件

- **GitHub**: [@AshIgnis](https://github.com/AshIgnis)```

- **项目地址**: [JLU_Emshop_System](https://github.com/AshIgnis/JLU_Emshop_System)

#### 第三步：RESTful API控制器

---

```java

<div align="center">// ProductController.java - 商品管理API

@RestController

**⭐ 如果这个项目对你有帮助,请给个 Star! ⭐**@RequestMapping("/api/products")

@CrossOrigin(origins = "*")

Made with ❤️ by AshIgnispublic class ProductController {

    

</div>    @Autowired

    private ProductService productService;
    
    // 获取商品列表
    @GetMapping
    public ResponseEntity<Page<ProductDTO>> getProducts(
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size,
            @RequestParam(required = false) String category) {
        
        Page<ProductDTO> products = productService.getProducts(page, size, category);
        return ResponseEntity.ok(products);
    }
    
    // 获取商品详情
    @GetMapping("/{id}")
    public ResponseEntity<ProductDTO> getProduct(@PathVariable Long id) {
        ProductDTO product = productService.getProductById(id);
        return ResponseEntity.ok(product);
    }
    
    // 添加商品（管理员功能）
    @PostMapping
    @PreAuthorize("hasRole('ADMIN')")
    public ResponseEntity<ProductDTO> addProduct(@RequestBody ProductDTO productDTO) {
        ProductDTO createdProduct = productService.addProduct(productDTO);
        return ResponseEntity.status(HttpStatus.CREATED).body(createdProduct);
    }
}
```

#### 第四步：配置文件设置

```yaml
# application.yml - 应用配置
server:
  port: 8080
  servlet:
    context-path: /emshop

spring:
  application:
    name: emshop-web-service
  
  # 数据库配置
  datasource:
    url: jdbc:mysql://localhost:3306/emshop?useSSL=false&serverTimezone=UTC&characterEncoding=utf8
    username: root
    password: your_password
    driver-class-name: com.mysql.cj.jdbc.Driver
  
  # JPA配置
  jpa:
    hibernate:
      ddl-auto: update
    show-sql: true
    properties:
      hibernate:
        dialect: org.hibernate.dialect.MySQL8Dialect

# 自定义配置
emshop:
  security:
    jwt:
      secret: your_jwt_secret_key
      expiration: 86400000 # 24小时
```

#### 第五步：C++与Java服务集成

```cpp
// C++服务端调用Java Web API的HTTP客户端
class WebServiceClient {
private:
    std::string baseUrl;
    
public:
    WebServiceClient(const std::string& url) : baseUrl(url) {}
    
    // 同步商品信息到Web服务
    bool syncProductToWeb(const Product& product) {
        // HTTP请求实现
        return true;
    }
    
    // 从Web服务获取商品信息
    std::vector<Product> getProductsFromWeb() {
        // HTTP请求实现
        std::vector<Product> products;
        return products;
    }
};
```

### 6.5 通信协议实现

#### 消息格式定义

```cpp
// 统一消息格式
struct NetworkMessage {
    quint32 messageType;    // 消息类型
    quint32 userId;         // 用户ID
    quint32 dataSize;       // 数据长度
    QByteArray data;        // JSON格式数据
};

// JSON数据示例
{
    "type": "product_list_request",
    "parameters": {
        "category": "electronics",
        "page": 1,
        "pageSize": 20
    }
}
```

#### 协议处理流程

```cpp
// 发送消息
void NetworkManager::sendMessage(const Message &message) {
    QJsonObject json;
    json["type"] = static_cast<int>(message.type);
    json["userId"] = message.userId;
    json["data"] = message.data;
    
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    
    quint32 dataSize = jsonData.size();
    socket->write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    socket->write(jsonData);
}

// 接收消息
void NetworkManager::onReadyRead() {
    while (socket->bytesAvailable() >= sizeof(quint32)) {
        quint32 dataSize;
        socket->read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
        
        if (socket->bytesAvailable() >= dataSize) {
            QByteArray jsonData = socket->read(dataSize);
            QJsonDocument doc = QJsonDocument::fromJson(jsonData);
            
            Message message = parseJsonMessage(doc.object());
            emit messageReceived(message);
        }
    }
}
```

## 5.开发工具

- **IDE**：Qt Creator（Qt专用）+ Visual Studio（服务端C++）+ IntelliJ IDEA（Java Web服务）
- **数据库**：MySQL（功能完整）
- **版本控制**：Git
- **构建工具**：CMake（服务端C++）、qmake/CMake（Qt客户端）、Maven（Java Web服务）
- **测试工具**：Google Test（C++）、Qt Test（Qt单元测试）、JUnit（Java单元测试）
- **调试工具**：Qt Creator调试器、Visual Studio调试器、IntelliJ IDEA调试器
- **设计工具**：Qt Designer（界面设计）
- **API测试**：Postman（Web API测试）、Swagger（API文档）

## 7.实施

### 7.1 开发顺序

1. **服务端基础框架**（1-2周）
   - TCP服务器搭建
   - 数据库连接和基本操作
   - 基础通信协议

2. **客户端基础框架**（1-2周）
   - Qt项目搭建
   - 基本界面设计
   - 网络通信模块

3. **核心功能实现**（2-3周）
   - 用户登录注册
   - 商品展示和管理
   - 购物车功能

4. **高级功能和优化**（2-3周）
   - 订单处理和售后
   - 促销策略
   - 界面美化和性能优化

### 7.2 学习资源

**C++服务端开发**：

- 《C++ Primer》- C++基础
- 《Unix网络编程》- 网络编程
- Boost库文档 - 高级C++库

**Qt客户端开发**：

- Qt官方文档和示例
- 《Qt 6 C++ GUI Programming Cookbook》
- Qt社区论坛和Stack Overflow

**Java Web服务开发**：

- 《Spring Boot实战》- Spring Boot框架
- Spring官方文档和指南
- 《Java核心技术》- Java基础知识
- Maven官方文档 - 项目管理工具

**数据库设计**：

- 《数据库系统概念》
- MySQL官方文档

### 7.3 调试和测试

**调试策略**：

- 使用日志记录关键操作
- 分模块单元测试
- 网络通信抓包分析
- 多客户端并发测试

**性能测试**：

- 服务端压力测试
- 客户端响应时间测试
- Web API性能测试
- 内存使用情况监控
- 数据库查询优化

## 8.Java Web服务实施建议

### 8.1 开发部署流程

**开发环境搭建**：

1. 安装JDK 17+和IntelliJ IDEA
2. 配置Maven仓库和依赖
3. 创建Spring Boot项目骨架
4. 配置数据库连接

### 8.2 系统架构优势

**多端支持**：

- C++桌面客户端：高性能本地应用
- Web管理后台：便于系统管理和维护
- RESTful API：支持未来移动端扩展

**技术栈互补**：

- C++：高性能核心业务处理
- Java：企业级Web服务和管理
- Qt：专业桌面应用界面
- MySQL：可靠的数据持久化

### 8.3 安全性考虑

**API安全**：

- JWT token认证机制
- CORS跨域请求控制
- 输入参数验证和过滤
- SQL注入防护

**数据安全**：

- 密码加密存储
- 敏感数据传输加密
- 访问日志记录
- 定期数据备份

### 8.4 监控和维护

**系统监控**：

- API响应时间监控
- 数据库性能监控
- 系统资源使用监控
- 错误日志收集分析

**运维管理**：

- 自动化部署脚本
- 配置文件管理
- 数据库迁移脚本
- 系统健康检查

### 已实现功能完成度评估

#### 1. 订单管理

**已完成功能**：
- ✅ 基础订单创建（从购物车创建订单）
- ✅ 完整的订单状态流转系统
  - pending（待确认）→ confirmed（已确认）→ paid（已支付）→ shipped（已发货）→ delivered（已送达）→ completed（已完成）
  - 支持取消订单（cancelled）和退款（refunded）状态
- ✅ 订单状态验证和转换规则
- ✅ 发货和物流跟踪功能
  - 快递单号记录
  - 物流信息跟踪
  - 发货时间和送达时间记录
- ✅ 退款处理机制
  - 订单退款申请
  - 退款状态管理
  - 退款金额验证

**待完善**：
- 🔄 高级物流集成（第三方物流API）
- 🔄 订单自动确认机制

#### 2. 支付系统

**已完成功能**：
- ✅ 支付接口框架完整实现
  - `processPayment` 方法已实现
  - 支持多种支付方式（微信、支付宝、银行卡等）
  - 交易ID生成和管理
- ✅ 支付状态验证和管理
  - 支付前状态检查
  - 支付金额验证
  - 支付成功后状态更新
- ✅ 退款接口实现
  - `refundPayment` 方法完整
  - 退款金额验证
  - 退款状态跟踪

**待完善**：
- 🔄 第三方支付平台真实对接（目前为模拟支付）
- 🔄 支付回调处理
- 🔄 支付安全加密

#### 3. 促销系统

**已完成功能**：
- ✅ 优惠券系统完整实现
  - 优惠券创建、查询、使用
  - 用户优惠券管理
  - 优惠券状态验证
- ✅ 基础促销策略框架
  - 折扣计算接口（`calculateDiscount`）
  - 促销码验证
  - 促销活动管理
- ✅ 价格计算逻辑
  - 原价、折扣价、最终价格计算

**待完善**：
- 🔄 复杂促销策略（满减、买N送M、阶梯价格）
- 🔄 促销活动时间限制
- 🔄 用户群体定向促销

#### 4. UI界面

**已完成功能**：
- ✅ 完整的控制台界面系统
  - Netty TCP服务器
  - 会话管理系统
  - 命令行客户端
  - 实时交互功能

**待完善**：
- 🔄 图形化用户界面（Qt/JavaFX）
- 🔄 Web前端界面
- 🔄 移动端适配

### 核心技术架构完成度

#### 1. 后端服务
- ✅ JNI C++核心业务逻辑（8194行代码）
- ✅ Netty网络通信框架
- ✅ MySQL数据库完整集成
- ✅ 会话管理和用户认证
- ✅ 完整的服务端错误处理

#### 2. 数据持久化 (完成度: 95%)
- ✅ 完整的数据库表结构
- ✅ 数据库连接池管理
- ✅ SQL操作封装
- ✅ 事务处理机制

#### 3. 业务功能模块 (完成度: 80%)
- ✅ 用户管理系统
- ✅ 商品管理系统
- ✅ 购物车功能
- ✅ 订单处理系统
- ✅ 优惠券系统
- ✅ 评价系统
- ✅ 地址管理

### 总体系统完成度评估

**综合完成度：75%**

**系统优势**：
1. 核心业务逻辑完整且健壮
2. 数据库设计规范完善
3. 网络通信稳定可靠
4. 支持高并发和会话管理
5. 代码结构清晰，面向对象设计良好

**主要差距**：
1. 图形化界面开发
2. 第三方服务集成（支付、物流）
3. 前端Web界面开发
4. 高级促销策略实现
