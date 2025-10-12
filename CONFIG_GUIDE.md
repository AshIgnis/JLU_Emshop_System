# JLU Emshop System - 配置管理说明

## 📋 概述

本系统采用灵活的配置管理机制,支持配置文件和环境变量两种方式。环境变量的优先级高于配置文件。

## 🔧 配置方式

### 方式1: 使用配置文件 (推荐用于开发环境)

1. 复制示例配置文件:
```bash
cp config.example.json config.json
```

2. 编辑 `config.json` 填写实际配置值:
```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "emshop",
    "user": "root",
    "password": "YOUR_ACTUAL_PASSWORD",
    "charset": "utf8mb4"
  },
  ...
}
```

3. **重要**: `config.json` 已在 `.gitignore` 中,不会被提交到Git

### 方式2: 使用环境变量 (推荐用于生产环境)

#### Windows PowerShell:
```powershell
$env:DB_HOST="localhost"
$env:DB_PORT="3306"
$env:DB_NAME="emshop"
$env:DB_USER="root"
$env:DB_PASSWORD="your_password"
```

#### Windows CMD:
```cmd
set DB_HOST=localhost
set DB_PORT=3306
set DB_NAME=emshop
set DB_USER=root
set DB_PASSWORD=your_password
```

#### Linux/macOS:
```bash
export DB_HOST=localhost
export DB_PORT=3306
export DB_NAME=emshop
export DB_USER=root
export DB_PASSWORD=your_password
```

或者使用 `.env` 文件:
```bash
# 复制示例文件
cp .env.example .env

# 编辑 .env 文件
nano .env
```

## 📝 配置项说明

### 数据库配置 (database)

| 配置项 | 环境变量 | 类型 | 默认值 | 说明 |
|--------|----------|------|--------|------|
| host | DB_HOST | string | localhost | 数据库服务器地址 |
| port | DB_PORT | int | 3306 | 数据库端口 |
| name | DB_NAME | string | emshop | 数据库名称 |
| user | DB_USER | string | root | 数据库用户名 |
| password | DB_PASSWORD | string | - | 数据库密码 ⚠️必填 |
| charset | DB_CHARSET | string | utf8mb4 | 字符集 |

### 服务器配置 (server)

| 配置项 | 环境变量 | 类型 | 默认值 | 说明 |
|--------|----------|------|--------|------|
| port | SERVER_PORT | int | 8888 | Netty服务器端口 |
| max_connections | SERVER_MAX_CONNECTIONS | int | 100 | 最大并发连接数 |
| timeout_seconds | SERVER_TIMEOUT | int | 30 | 连接超时时间(秒) |

### 日志配置 (logging)

| 配置项 | 环境变量 | 类型 | 默认值 | 说明 |
|--------|----------|------|--------|------|
| level | LOG_LEVEL | string | INFO | 日志级别: DEBUG/INFO/WARN/ERROR |
| file | LOG_FILE | string | logs/emshop.log | 日志文件路径 |
| max_size_mb | LOG_MAX_SIZE_MB | int | 50 | 单个日志文件最大大小(MB) |
| max_files | LOG_MAX_FILES | int | 10 | 保留的日志文件数量 |

### 安全配置 (security)

| 配置项 | 环境变量 | 类型 | 默认值 | 说明 |
|--------|----------|------|--------|------|
| jwt_secret | JWT_SECRET | string | - | JWT密钥 ⚠️生产环境必填 |
| session_timeout_minutes | SESSION_TIMEOUT_MINUTES | int | 120 | 会话超时时间(分钟) |
| password_salt | PASSWORD_SALT | string | - | 密码加密盐值 ⚠️生产环境必填 |

### 业务配置 (business)

| 配置项 | 环境变量 | 类型 | 默认值 | 说明 |
|--------|----------|------|--------|------|
| low_stock_threshold | LOW_STOCK_THRESHOLD | int | 10 | 低库存预警阈值 |
| order_timeout_minutes | ORDER_TIMEOUT_MINUTES | int | 30 | 订单超时时间(分钟) |
| max_cart_items | MAX_CART_ITEMS | int | 99 | 购物车最大商品数量 |

## 🔒 安全建议

### 开发环境
1. 使用 `config.json` 存储配置
2. **不要** 将 `config.json` 提交到Git
3. 定期更换数据库密码

### 生产环境
1. **必须** 使用环境变量或安全的密钥管理服务
2. **禁止** 在代码中硬编码任何敏感信息
3. **禁止** 将 `config.json` 部署到生产服务器
4. 使用强密码并定期轮换
5. 限制配置文件的访问权限 (chmod 600)
6. 使用加密连接 (SSL/TLS)

## 🔄 配置优先级

当同时存在多种配置时,优先级从高到低为:

1. 环境变量
2. config.json 文件  
3. config.example.json 文件
4. 代码中的默认值

## 📖 使用示例

### C++ 代码中使用配置

```cpp
#include "ConfigLoader.h"

// 获取配置
auto& config = ConfigLoader::getInstance();

std::string dbHost = config.getString("database", "host");
int dbPort = config.getInt("database", "port");
bool autoReconnect = config.getBool("database", "auto_reconnect", true);

// 重新加载配置
config.reload();
```

### Java 代码中使用配置

目前Java端使用JNI调用C++的DatabaseConfig,后续可以扩展Java配置加载器。

## 🐛 常见问题

### Q: 为什么连接数据库失败?
A: 检查以下几点:
1. `config.json` 中的密码是否正确
2. MySQL服务是否启动
3. 防火墙是否允许连接
4. 数据库用户是否有足够权限

### Q: 如何查看当前使用的配置?
A: 检查日志文件 `logs/emshop.log`,系统启动时会记录配置信息(密码除外)

### Q: 配置修改后需要重启吗?
A: 是的,当前版本配置在程序启动时加载,修改后需要重启服务。

### Q: 可以在运行时修改配置吗?
A: C++端支持调用 `ConfigLoader::getInstance().reload()` 重新加载配置。

## 📚 相关文档

- [快速启动指南](RUN_GUIDE.md)
- [安全指南](SECURITY_NOTES.md)
- [部署指南](DEPLOYMENT_GUIDE.md)

## 🔔 更新日志

- 2025-10-12: 初始版本,支持配置文件和环境变量
- 2025-10-12: 添加ConfigLoader C++类
- 2025-10-12: 从源码中移除硬编码密码

---

**维护者**: JLU Emshop Team  
**更新日期**: 2025-10-12
