# JLU Emshop System - 快速启动指南

> 📅 更新日期: 2025年10月12日  
> 🎯 目标: 帮助开发者快速搭建和运行完整的Emshop系统

---

## 📋 目录

1. [系统概述](#系统概述)
2. [环境要求](#环境要求)
3. [安装步骤](#安装步骤)
4. [配置说明](#配置说明)
5. [启动服务](#启动服务)
6. [测试验证](#测试验证)
7. [常见问题](#常见问题)

---

## 🎯 系统概述

JLU Emshop System 是一个完整的电商交易系统,采用C/S架构:

- **后端**: C++ JNI + Java Netty服务器
- **前端**: Qt 6.x C++桌面客户端  
- **数据库**: MySQL 8.0+
- **通信**: TCP/IP + JSON协议

**架构图**:
```
┌─────────────┐         ┌──────────────────┐         ┌──────────┐
│ Qt客户端     │ <----->  │ Java Netty服务器  │ <----->  │ MySQL DB │
│ (GUI)       │   TCP    │ + C++ JNI核心     │   JDBC   │          │
└─────────────┘         └──────────────────┘         └──────────┘
```

---

## 💻 环境要求

### 1. 操作系统
- ✅ Windows 10/11 (推荐)
- ✅ Linux (Ubuntu 20.04+)
- ✅ macOS (实验性支持)

### 2. 开发工具

| 工具 | 版本要求 | 用途 | 下载链接 |
|------|----------|------|----------|
| **JDK** | 21+ | Java开发和运行 | [Oracle JDK](https://www.oracle.com/java/technologies/downloads/) |
| **Qt** | 6.5+ | Qt客户端开发 | [Qt官网](https://www.qt.io/download) |
| **MySQL** | 8.0+ | 数据库服务 | [MySQL官网](https://dev.mysql.com/downloads/mysql/) |
| **MinGW-w64** | 最新版 | C++编译器(Windows) | [MinGW-w64](https://www.mingw-w64.org/) |
| **CMake** | 3.20+ | 构建工具 | [CMake官网](https://cmake.org/download/) |
| **Git** | 最新版 | 版本控制 | [Git官网](https://git-scm.com/) |
| **Maven** | 3.8+ | Java项目管理 | [Maven官网](https://maven.apache.org/) |

### 3. 第三方库

| 库 | 版本 | 用途 |
|-----|------|------|
| **MySQL Connector/C** | 8.0+ | C++连接MySQL |
| **Netty** | 4.1.x | Java异步网络框架 |
| **nlohmann/json** | 3.x | C++ JSON处理 |

---

## 📦 安装步骤

### Step 1: 克隆项目

```bash
git clone https://github.com/AshIgnis/JLU_Emshop_System.git
cd JLU_Emshop_System
```

### Step 2: 安装MySQL

#### Windows:
1. 下载MySQL 8.0+ Windows安装包
2. 运行安装程序,记住设置的root密码
3. 确保MySQL服务已启动

```powershell
# 检查MySQL服务状态
Get-Service MySQL80

# 启动MySQL服务(如果未运行)
Start-Service MySQL80
```

#### Linux:
```bash
sudo apt update
sudo apt install mysql-server
sudo systemctl start mysql
sudo systemctl enable mysql

# 设置root密码
sudo mysql_secure_installation
```

### Step 3: 初始化数据库

```bash
# 进入数据库脚本目录
cd cpp

# 连接MySQL(Windows PowerShell)
mysql -u root -p

# 或者使用批处理脚本(Windows)
.\init_emshop_database.bat
```

在MySQL命令行中执行:
```sql
-- 创建数据库和表结构
SOURCE emshop_database_init.sql;

-- 初始化测试数据(可选)
SOURCE init_database.sql;

-- 创建审计表(新增)
SOURCE create_audit_tables.sql;

-- 初始化优惠券数据(可选)
SOURCE init_coupons.sql;

-- 验证数据库
SOURCE verify_database_connection.sql;

-- 查看所有表
USE emshop;
SHOW TABLES;
```

### Step 4: 配置系统

```bash
# 返回项目根目录
cd ..

# 复制配置文件模板
cp config.example.json config.json

# 编辑配置文件,填入实际的数据库密码
notepad config.json  # Windows
# 或
nano config.json     # Linux/Mac
```

在`config.json`中修改:
```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "emshop",
    "user": "root",
    "password": "YOUR_ACTUAL_PASSWORD",  // ← 修改这里
    "charset": "utf8mb4"
  },
  ...
}
```

💡 **安全提示**: `config.json`已在`.gitignore`中,不会被提交到Git。

### Step 5: 编译C++ JNI库

#### Windows:
```powershell
cd cpp
.\build_oop_jni.bat
```

编译成功后会生成`emshop_native_oop.dll`文件。

#### Linux:
```bash
cd cpp
g++ -std=c++17 -shared -fPIC \
    -I$JAVA_HOME/include \
    -I$JAVA_HOME/include/linux \
    -I/usr/include/mysql \
    -L/usr/lib/x86_64-linux-gnu \
    -lmysqlclient \
    -o emshop_native_oop.so \
    emshop_native_impl_oop.cpp
```

### Step 6: 编译Java服务端

```bash
cd ../java
mvn clean package
```

编译成功后会生成`target/emshop-1.0-SNAPSHOT.jar`。

### Step 7: 编译Qt客户端

#### 使用Qt Creator:
1. 打开Qt Creator
2. 打开项目: `qtclient/CMakeLists.txt`
3. 配置Kit (选择MinGW 64-bit或MSVC)
4. 点击"构建" → "构建项目"

#### 使用命令行:
```bash
cd qtclient
mkdir build
cd build

# Windows
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/6.9.1/mingw_64 ..
mingw32-make -j4

# Linux
cmake ..
make -j4
```

---

## 🔧 配置说明

### 配置方式

系统支持两种配置方式(环境变量优先级更高):

1. **配置文件**: `config.json`
2. **环境变量**: 详见[CONFIG_GUIDE.md](CONFIG_GUIDE.md)

### 核心配置项

```json
{
  "database": {
    "host": "localhost",      // 数据库地址
    "port": 3306,            // 数据库端口
    "name": "emshop",        // 数据库名
    "user": "root",          // 数据库用户名
    "password": "******"     // 数据库密码 ⚠️
  },
  "server": {
    "port": 8888,            // Netty服务器端口
    "max_connections": 100   // 最大连接数
  }
}
```

详细配置说明请参考: [CONFIG_GUIDE.md](CONFIG_GUIDE.md)

---

## 🚀 启动服务

### 启动顺序

必须按以下顺序启动服务:

```
MySQL数据库 → Java Netty服务器 → Qt客户端
```

### 1. 启动MySQL服务

#### Windows:
```powershell
# 检查服务状态
Get-Service MySQL80

# 启动服务
Start-Service MySQL80
```

#### Linux:
```bash
sudo systemctl status mysql
sudo systemctl start mysql
```

### 2. 启动Java Netty服务器

```bash
cd java

# 方式1: 使用Maven运行
mvn exec:java -Dexec.mainClass="emshop.EmshopNettyServer"

# 方式2: 使用jar包运行
java -cp target/emshop-1.0-SNAPSHOT.jar emshop.EmshopNettyServer

# 方式3: 使用管理员控制台
java -cp target/emshop-1.0-SNAPSHOT.jar emshop.AdminConsole
```

看到以下输出表示服务器启动成功:
```
============================================
  JLU Emshop Netty 服务器
============================================
[INFO] 正在初始化C++ JNI库...
[INFO] JNI库初始化成功
[INFO] Netty服务器启动成功，监听端口: 8888
[INFO] 服务器已准备就绪，等待客户端连接...
```

### 3. 启动Qt客户端

#### Windows:
```powershell
cd qtclient\build
.\emshop_qtclient.exe
```

#### Linux:
```bash
cd qtclient/build
./emshop_qtclient
```

#### 使用Qt Creator:
1. 打开项目
2. 点击"运行"按钮 (Ctrl+R)

---

## ✅ 测试验证

### 1. 测试数据库连接

```sql
mysql -u root -p
USE emshop;

-- 查看用户表
SELECT user_id, username, role FROM users LIMIT 5;

-- 查看商品表
SELECT product_id, name, price, stock_quantity FROM products LIMIT 5;
```

### 2. 测试服务器连接

使用测试客户端:
```bash
cd java
java -cp target/emshop-1.0-SNAPSHOT.jar emshop.JNITestClient
```

或使用用户控制台:
```bash
java -cp target/emshop-1.0-SNAPSHOT.jar emshop.UserConsole
```

### 3. 测试Qt客户端功能

1. **登录测试**
   - 使用测试账号: `admin` / `admin123`
   - 或: `test_user` / `test123`

2. **商品浏览**
   - 切换到"商品"标签页
   - 查看商品列表和详情

3. **购物车功能**
   - 添加商品到购物车
   - 修改数量
   - 结算测试

4. **订单管理**
   - 查看订单列表
   - 查看订单详情

### 4. 功能清单

| 功能模块 | 功能点 | 状态 |
|---------|--------|------|
| **用户管理** | 注册、登录、登出 | ✅ |
| **商品管理** | 列表、搜索、详情 | ✅ |
| **购物车** | 添加、删除、修改、结算 | ✅ |
| **订单管理** | 创建、查询、状态管理 | ✅ |
| **优惠券** | 查询、使用 | ✅ |
| **库存管理** | 实时更新、并发控制 | ✅ |
| **支付功能** | 模拟支付 | ✅ |
| **退款功能** | 申请退款、库存返还 | 🔄 进行中 |
| **管理后台** | 商品管理、订单管理 | ✅ |

---

## ❓ 常见问题

### Q1: 编译JNI库失败

**问题**: `fatal error: jni.h: No such file or directory`

**解决**:
1. 确认JDK已正确安装
2. 检查`build_oop_jni.bat`中的JDK路径是否正确
3. 设置`JAVA_HOME`环境变量

```powershell
# Windows
$env:JAVA_HOME="C:\Program Files\Java\jdk-21"

# Linux
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk
```

### Q2: 连接MySQL失败

**问题**: `Access denied for user 'root'@'localhost'`

**解决**:
1. 确认MySQL服务已启动
2. 检查`config.json`中的用户名和密码
3. 重置MySQL密码:

```sql
ALTER USER 'root'@'localhost' IDENTIFIED BY 'new_password';
FLUSH PRIVILEGES;
```

### Q3: Netty服务器启动失败

**问题**: `Address already in use: bind`

**解决**:
1. 端口8888被占用,修改`config.json`中的端口
2. 或关闭占用端口的程序:

```powershell
# Windows - 查找占用端口的程序
netstat -ano | findstr :8888
taskkill /PID <PID> /F

# Linux
lsof -i :8888
kill -9 <PID>
```

### Q4: Qt客户端无法连接服务器

**问题**: 客户端显示"连接失败"

**解决**:
1. 确认Netty服务器已启动
2. 检查防火墙设置
3. 确认客户端连接地址为`localhost:8888`

### Q5: 数据库表不存在

**问题**: `Table 'emshop.users' doesn't exist`

**解决**:
```bash
cd cpp
mysql -u root -p < emshop_database_init.sql
```

### Q6: JNI库加载失败

**问题**: `java.lang.UnsatisfiedLinkError`

**解决**:
1. 确认`emshop_native_oop.dll`已编译
2. 确认DLL文件在正确的路径
3. 确认`libmysql.dll`在PATH中

```powershell
# Windows - 将DLL复制到Java目录
copy cpp\emshop_native_oop.dll java\src\main\java\emshop\
copy cpp\libmysql.dll java\src\main\java\emshop\
```

---

## 📚 相关文档

- [配置管理指南](CONFIG_GUIDE.md) - 详细的配置说明
- [TODO清单](TODO.md) - 待完善功能列表
- [README](readme.md) - 项目概述和技术栈
- [JNI设计文档](cpp/README_OOP_JNI.md) - C++ JNI实现说明

---

## 🔄 开发流程

### 日常开发

```bash
# 1. 拉取最新代码
git pull

# 2. 修改代码后重新编译
cd cpp && build_oop_jni.bat
cd ../java && mvn clean package
cd ../qtclient/build && mingw32-make

# 3. 重启服务测试
# 先停止正在运行的服务器(Ctrl+C)
# 然后重新启动
```

### 数据库变更

```bash
# 1. 修改SQL脚本
cd cpp
notepad emshop_database_init.sql

# 2. 重新初始化数据库
mysql -u root -p < emshop_database_init.sql
```

### 调试技巧

1. **查看日志**: 
   - C++日志: `logs/emshop.log`
   - Java日志: 控制台输出

2. **数据库调试**:
   ```sql
   -- 查看最近的订单
   SELECT * FROM orders ORDER BY created_at DESC LIMIT 10;
   
   -- 查看库存变动
   SELECT * FROM stock_change_audit ORDER BY created_at DESC LIMIT 20;
   ```

3. **网络调试**: 使用Wireshark抓包分析TCP通信

---

## 💡 最佳实践

1. **开发环境**:
   - 使用`config.json`存储配置
   - 定期提交代码(但不提交`config.json`)

2. **生产环境**:
   - 使用环境变量代替配置文件
   - 启用日志记录
   - 定期备份数据库

3. **性能优化**:
   - 调整数据库连接池大小
   - 使用索引优化查询
   - 监控内存使用

---

## 🆘 获取帮助

遇到问题? 可以通过以下方式获取帮助:

1. 查看[常见问题](#常见问题)章节
2. 检查日志文件: `logs/emshop.log`
3. 提交Issue: [GitHub Issues](https://github.com/AshIgnis/JLU_Emshop_System/issues)
4. 查看相关文档

---

## 📝 更新日志

- **2025-10-12**: 添加配置管理系统,创建快速启动指南
- **2025-09-11**: 完成核心功能开发,系统完成度75%
- **2025-08-31**: 完成面向对象JNI重构

---

**祝开发顺利! 🎉**

**维护者**: JLU Emshop Team  
**联系方式**: [GitHub Repository](https://github.com/AshIgnis/JLU_Emshop_System)
