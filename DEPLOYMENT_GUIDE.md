# JLU Emshop System - 部署与答辩指南

> 版本: 1.0  
> 更新日期: 2025年10月13日  
> 用途: 项目答辩与快速部署

---

## 📋 快速部署检查清单

- [ ] Java 21 已安装并配置环境变量
- [ ] Maven 3.6+ 已安装
- [ ] g++ (MinGW 64位) 已安装
- [ ] MySQL 8.0+ 已安装并运行
- [ ] 数据库已初始化
- [ ] DLL 文件已编译
- [ ] Java 测试通过

---

## 🚀 一键部署步骤（Windows）

### 步骤 1: 环境准备

1. **检查 Java 版本**
```powershell
java -version  # 应显示 Java 21
```

2. **检查 Maven**
```powershell
mvn -version  # 应显示 Maven 3.6+
```

3. **检查 g++**
```powershell
g++ --version  # 应显示 MinGW-w64 gcc
```

4. **检查 MySQL**
```powershell
mysql --version  # 应显示 MySQL 8.0+
```

### 步骤 2: 配置文件设置

1. **复制配置文件模板**
```powershell
cd d:\codehome\jlu\JLU_Emshop_System
copy config.example.json config.json
```

2. **编辑 config.json**
```json
{
  "database": {
    "host": "127.0.0.1",
    "port": 3306,
    "name": "emshop",
    "user": "root",
    "password": "YOUR_PASSWORD",  // ← 改为你的密码
    "charset": "utf8mb4"
  }
}
```

### 步骤 3: 数据库初始化

1. **登录 MySQL**
```powershell
mysql -u root -p
```

2. **创建数据库**
```sql
CREATE DATABASE IF NOT EXISTS emshop CHARACTER SET utf8mb4;
USE emshop;
SOURCE d:/codehome/jlu/JLU_Emshop_System/cpp/emshop_database_init.sql;
```

3. **验证数据库**
```sql
SHOW TABLES;  -- 应显示多个表
SELECT COUNT(*) FROM products;  -- 应有测试数据
```

### 步骤 4: 一键构建与测试

```powershell
cd d:\codehome\jlu\JLU_Emshop_System
.\build_and_test.bat
```

**预期输出**:
```
============================================
  JLU Emshop系统 - 一键构建与测试
============================================
[1/4] 检查构建环境...
✓ Java环境正常
✓ Maven构建工具正常
✓ g++编译器正常
[2/4] 编译C++ JNI库...
✅ 编译成功！
[3/4] 部署JNI库到Java项目...
✓ JNI库部署成功
[4/4] 运行Java单元测试...
Tests run: 13, Failures: 0, Errors: 0, Skipped: 0
✅ 所有测试通过！
```

### 步骤 5: 启动服务

```powershell
cd java
mvn exec:java -Dexec.mainClass="emshop.EmshopNettyServer" -Dexec.args="8080"
```

**成功标志**:
```
Native library 'emshop_native_oop' loaded successfully
服务器启动成功，监听端口: 8080
```

---

## 🐛 常见问题与解决方案

### 问题 1: DLL 找不到

**错误**: `java.lang.UnsatisfiedLinkError: no emshop_native_oop in java.library.path`

**解决方案**:
```powershell
# 方案1: 复制DLL到Java项目
copy cpp\emshop_native_oop.dll java\src\main\resources\lib\
copy cpp\libmysql.dll java\src\main\resources\lib\

# 方案2: 添加到系统PATH
set PATH=%PATH%;d:\codehome\jlu\JLU_Emshop_System\cpp
```

### 问题 2: 数据库连接失败

**错误**: `Can't connect to MySQL server on 'localhost'`

**检查项**:
1. MySQL 服务是否运行？
```powershell
net start MySQL80  # 启动MySQL服务
```

2. config.json 中密码是否正确？

3. 数据库是否已创建？
```sql
SHOW DATABASES LIKE 'emshop';
```

### 问题 3: 编译失败

**错误**: `error: 'xxx' was not declared in this scope`

**解决方案**:
1. 确认 g++ 版本支持 C++17
2. 检查头文件路径是否正确
3. 重新运行 `build_oop_jni.bat`

### 问题 4: 测试失败

**错误**: `Tests run: X, Failures: Y`

**解决方案**:
1. 确认数据库中有测试数据
2. 检查 config.json 配置
3. 查看 `java/target/surefire-reports/*.txt` 详细错误

---

## 🎯 答辩演示脚本

### 演示场景 1: 完整下单流程（3分钟）

1. **用户注册与登录**
```json
POST /register
{
  "username": "demo_user",
  "password": "Demo123456",
  "phone": "13800138000"
}

POST /login
{
  "username": "demo_user",
  "password": "Demo123456"
}
```

2. **浏览商品**
```json
GET /products?page=1&pageSize=10
```

3. **添加到购物车**
```json
POST /cart/add
{
  "user_id": 1,
  "product_id": 1,
  "quantity": 2
}
```

4. **创建订单**
```json
POST /order/create
{
  "user_id": 1,
  "address_id": 1,
  "coupon_code": "",
  "remark": "尽快发货"
}
```

5. **支付订单**
```json
POST /order/pay
{
  "order_id": 1,
  "payment_method": "alipay"
}
```

**预期结果**: 订单状态从 `pending` → `paid`，库存自动扣减

---

### 演示场景 2: 订单取消与库存返还（2分钟）

1. **查看订单详情**
```json
GET /order/detail?order_id=2
```

2. **取消订单**
```json
POST /order/cancel
{
  "user_id": 1,
  "order_id": 2
}
```

3. **验证库存返还**
```json
GET /product/detail?product_id=1
// 库存应增加取消订单的数量
```

**预期结果**: 订单状态变为 `cancelled`，库存自动返还

---

### 演示场景 3: 并发安全性（2分钟）

**演示内容**:
- 多个用户同时购买最后1件商品
- 只有1个订单成功，其他订单提示库存不足

**关键代码展示**:
```cpp
// OrderService.cpp - 使用事务和FOR UPDATE锁
executeQuery("BEGIN");
json stock_result = executeQuery("SELECT ... FOR UPDATE");
// ... 库存校验和扣减
executeQuery("COMMIT");
```

---

## 📊 项目亮点展示

### 1. 代码质量

- **模块化设计**: 7个核心服务类，主文件从9629行→6069行（减少37%）
- **事务保护**: 订单创建、取消、退款均有完整事务
- **并发控制**: 使用 `FOR UPDATE` 和条件更新防止超卖

### 2. 测试覆盖

- **单元测试**: 13个测试用例，覆盖核心业务流程
- **集成测试**: 订单全生命周期测试
- **自动化**: GitHub Actions CI/CD 配置

### 3. 安全性

- **SQL注入防护**: 核心代码使用 `escapeSQLString`
- **输入校验**: ID、空值、数量等多重校验
- **配置分离**: 敏感信息不在源码中

### 4. 可维护性

- **错误码规范**: 90个错误码，覆盖10个模块
- **日志系统**: 完整的日志追踪
- **文档齐全**: 部署指南、审计报告、设计文档

---

## 📝 答辩问答准备

### Q1: 系统架构是什么？

**答**: 
> 系统采用三层架构：
> 1. **表示层**: Qt客户端（C++）
> 2. **业务层**: Netty服务器（Java）+ JNI桥接（C++）
> 3. **数据层**: MySQL数据库
> 
> 核心业务逻辑在C++中实现，通过JNI与Java Netty服务器通信。

### Q2: 如何保证并发安全？

**答**:
> 采用多重并发控制机制：
> 1. **数据库锁**: 库存查询使用 `FOR UPDATE` 行锁
> 2. **条件更新**: `WHERE stock_quantity >= X` 防止超卖
> 3. **互斥锁**: C++ `std::mutex` 保护订单操作
> 4. **事务保护**: 关键业务流程均在事务中

### Q3: 遇到的最大挑战是什么？

**答**:
> 最大挑战是**代码重构与模块化**。原始代码9629行，通过自动化脚本提取了7个服务类，减少了37%的代码量。过程中遇到了方法前缀缺失、重复include等问题，都逐一解决了。

### Q4: 系统的扩展性如何？

**答**:
> 系统具有良好的扩展性：
> 1. **服务层解耦**: 每个服务独立，易于单独优化
> 2. **配置外部化**: 业务参数在配置文件中
> 3. **接口标准化**: 所有响应统一JSON格式
> 4. **日志完善**: 便于问题追踪和性能分析

### Q5: 系统安全性如何保证？

**答**:
> 多层安全防护：
> 1. **SQL注入防护**: 核心代码使用转义函数
> 2. **输入校验**: 多重参数验证
> 3. **密码加密**: 使用salt+hash存储（如适用）
> 4. **配置安全**: 敏感信息外部化，不在版本控制中

---

## 🔧 技术栈说明

| 组件 | 技术 | 版本 |
|-----|------|------|
| 后端语言 | Java + C++ | Java 21, C++17 |
| 网络框架 | Netty | 4.1.100.Final |
| JNI桥接 | JNI 1.8 | - |
| 数据库 | MySQL | 8.0+ |
| 构建工具 | Maven | 3.6+ |
| 编译器 | g++ (MinGW) | 11.0+ |
| 客户端 | Qt | 6.9.1 |
| 测试框架 | JUnit 5 | 5.10.1 |
| JSON处理 | nlohmann/json | 3.11.2 |
| 日志框架 | Logback | 1.2.12 |

---

## 📦 项目文件结构

```
JLU_Emshop_System/
├── build_and_test.bat          # 一键构建与测试脚本
├── config.json                  # 配置文件（不提交git）
├── config.example.json          # 配置模板
├── cpp/
│   ├── services/                # 7个服务类（模块化后）
│   │   ├── OrderService.cpp
│   │   ├── UserService.cpp
│   │   ├── ProductService.cpp
│   │   ├── CartService.cpp
│   │   ├── AddressService.cpp
│   │   ├── CouponService.cpp
│   │   └── ReviewService.cpp
│   ├── emshop_native_impl_oop.cpp  # JNI主文件（6069行）
│   ├── build_oop_jni.bat        # C++编译脚本
│   ├── emshop_native_oop.dll    # 编译产物
│   └── emshop_database_init.sql # 数据库初始化脚本
├── java/
│   ├── src/
│   │   ├── main/java/emshop/    # Java源码
│   │   └── test/java/emshop/    # 测试代码
│   ├── pom.xml                   # Maven配置
│   └── target/                   # 编译产物
├── qtclient/                     # Qt客户端
├── .github/workflows/            # CI/CD配置
│   └── build-and-test.yml
├── TRANSACTION_AUDIT_REPORT.md   # 事务审计报告
├── SECURITY_AUDIT_REPORT.md      # 安全审计报告
└── README.md                     # 项目说明
```

---

## ✅ 答辩前最终检查

### 代码检查
- [ ] 所有代码已提交git
- [ ] DLL文件已编译成功
- [ ] 测试全部通过
- [ ] 配置文件已准备好

### 文档检查
- [ ] README.md 完整
- [ ] 部署指南清晰
- [ ] 审计报告已生成
- [ ] 答辩材料已准备

### 演示准备
- [ ] 数据库已初始化
- [ ] 测试数据已准备
- [ ] 服务可正常启动
- [ ] 演示脚本已测试

---

## 🎉 祝答辩成功！

**准备建议**:
1. 提前一天完整部署一遍，确保无问题
2. 准备U盘备份（代码+编译好的DLL+数据库脚本）
3. 演示前启动服务并验证
4. 准备口头陈述提纲（3-5分钟）

**答辩当天**:
- 提前30分钟到场
- 检查网络、投影仪
- 启动服务并验证
- 保持自信，从容应对

---

**文档版本**: 1.0  
**最后更新**: 2025年10月13日  
**维护人**: JLU Emshop Team
