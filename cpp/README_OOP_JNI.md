# JLU Emshop System - 面向对象JNI实现

## 概览

这是JLU电商系统的面向对象JNI（Java Native Interface）实现，采用现代C++17标准和Java 21，提供高性能的本地数据库操作和业务逻辑处理。

## 技术栈

- **C++标准**: C++17
- **Java版本**: Java 21 (JDK 21)
- **数据库**: MySQL 8.0+
- **JSON库**: nlohmann/json 3.12.0
- **编译器**: MinGW-w64 GCC 15.2.0
- **架构模式**: 面向对象设计 + 设计模式

## 核心特性

### 🏗️ 架构设计
- **单例模式**: 数据库配置、连接池、服务管理器
- **工厂模式**: 服务实例创建和管理
- **RAII原则**: 自动资源管理，防止内存泄漏
- **模板方法模式**: 基础服务类提供通用功能

### 🔒 线程安全
- 线程安全的数据库连接池
- 互斥锁保护关键资源
- 原子操作确保数据一致性

### 📊 数据库功能
- 智能连接池管理（初始5个连接，最大20个）
- 自动重连和连接验证
- SQL注入防护
- 事务支持

### 🎯 业务服务
- **用户服务**: 注册、登录、信息管理、会话管理
- **商品服务**: 商品管理、库存控制、搜索、分类
- **购物车服务**: 购物车操作、商品数量管理
- **订单服务**: 订单创建、状态管理、历史查询

## 文件结构

```
cpp/
├── emshop_native_impl_oop.cpp    # 主实现文件 (97KB)
├── emshop_EmshopNativeInterface.h # JNI接口头文件
├── nlohmann_json.hpp             # JSON处理库
├── libmysql.dll                  # MySQL客户端库
├── emshop_native_oop.dll         # 编译生成的JNI库
├── build_oop_jni.bat            # 自动编译脚本
├── test_dll.bat                 # DLL测试脚本
└── init_database.sql            # 数据库初始化脚本
```

## 编译说明

### 自动编译
运行编译脚本：
```bash
./build_oop_jni.bat
```

### 手动编译
```bash
g++ -std=c++17 -shared -O2 -DNDEBUG \
    "-IC:\Program Files\Java\jdk-21\include" \
    "-IC:\Program Files\Java\jdk-21\include\win32" \
    -I"D:\MySQL\include" \
    -o emshop_native_oop.dll \
    emshop_native_impl_oop.cpp libmysql.dll
```

### 编译要求
- Java 21 JDK
- MinGW-w64 GCC 15.2.0+
- MySQL 8.0+ 客户端库
- Windows 10/11 x64

## 使用方法

### 1. 环境配置
```sql
-- 运行数据库初始化脚本
source init_database.sql;
```

### 2. Java集成
```java
// 加载本地库
System.loadLibrary("emshop_native_oop");

// 初始化系统
String result = EmshopNativeInterface.initSystem();

// 用户登录示例
String loginResult = EmshopNativeInterface.login("testuser", "password123");

// 获取商品列表
String products = EmshopNativeInterface.getProductList("all", 1, 20);
```

### 3. 响应格式
所有方法返回JSON格式字符串：
```json
{
    "success": true,
    "message": "操作成功",
    "data": {
        "user_id": 12345,
        "username": "testuser",
        "token": "sess_1234567890_1"
    },
    "timestamp": 1640995200
}
```

## 配置参数

### 数据库配置
```cpp
namespace Constants {
    const char* const DB_HOST = "127.0.0.1";
    const int DB_PORT = 3306;
    const char* const DB_NAME = "emshop_db";
    const char* const DB_USER = "root";
    const char* const DB_PASSWORD = "";
}
```

### 连接池配置
```cpp
const int INITIAL_POOL_SIZE = 5;    // 初始连接数
const int MAX_POOL_SIZE = 20;       // 最大连接数
const int CONNECTION_TIMEOUT = 30;   // 连接超时（秒）
```

## API接口

### 用户管理
- `login(username, password)` - 用户登录
- `register(username, password, phone)` - 用户注册
- `logout(userId)` - 用户登出
- `getUserInfo(userId)` - 获取用户信息

### 商品管理
- `getProductList(category, page, pageSize)` - 获取商品列表
- `getProductDetail(productId)` - 获取商品详情
- `addProduct(jsonProduct)` - 添加商品
- `searchProducts(keyword, category, page, pageSize)` - 搜索商品

### 购物车管理
- `addToCart(userId, productId, quantity)` - 添加到购物车
- `getCart(userId)` - 获取购物车
- `removeFromCart(userId, productId)` - 从购物车移除
- `clearCart(userId)` - 清空购物车

### 系统管理
- `initSystem()` - 初始化系统
- `getSystemStatus()` - 获取系统状态
- `shutdown()` - 关闭系统

## 性能特点

- **高并发**: 支持多线程并发访问
- **内存优化**: RAII自动内存管理
- **连接复用**: 智能数据库连接池
- **快速响应**: 原生C++性能优势
- **低延迟**: 直接数据库访问，无ORM开销

## 日志系统

支持多级别日志记录：
- DEBUG: 详细调试信息
- INFO: 一般信息
- WARN: 警告信息
- ERROR: 错误信息

## 错误处理

统一错误码系统：
- 200: 成功
- 1001: 一般错误
- 1002: 验证错误
- 1003: 数据库错误
- 1004: 权限错误

## 开发指南

### 添加新服务
1. 继承`BaseService`基类
2. 实现`getServiceName()`虚函数
3. 在`EmshopServiceManager`中注册服务
4. 添加对应的JNI接口方法

### 数据库操作
```cpp
// 使用BaseService的executeQuery方法
json result = executeQuery("SELECT * FROM users WHERE id = ?", params);
if (result["success"].get<bool>()) {
    // 处理结果
    json data = result["data"];
}
```

### 线程安全
```cpp
// 使用互斥锁保护共享资源
std::lock_guard<std::mutex> lock(resource_mutex_);
// 安全的资源访问
```

## 版本历史

### v2.0.0 (2025-08-31)
- 完全重写为面向对象架构
- 采用C++17标准和Java 21
- 实现设计模式和最佳实践
- 添加完整的错误处理和日志系统
- 优化数据库连接池性能

### v1.0.0 (历史版本)
- 基础JNI实现
- 简单的数据库操作

## 许可证

版权所有 © 2025 JLU Emshop Team
保留所有权利。

## 贡献

欢迎提交问题和改进建议！

---
*本文档生成于 2025年8月31日*
