# C++ 代码重构计划 - P1-4

## 📋 重构目标

将 `emshop_native_impl_oop.cpp` (9902行) 拆分为模块化结构,提高代码可维护性和可测试性。

## 📊 当前状态分析

### 文件规模
- **总行数**: 9902行
- **JNI导出函数**: 82个
- **核心类**: DatabaseManager, UserService, ProductService, CartService, OrderService等

### 功能分布

通过分析JNI函数,识别出以下功能模块:

| 模块 | JNI函数数量 | 行数估算 | 主要功能 |
|------|------------|---------|---------|
| **系统初始化** | 2 | ~100 | initializeService, getInitializationStatus |
| **用户认证** | 5 | ~800 | login, register, logout, getUserInfo, updateUserInfo |
| **商品管理** | 12 | ~1500 | getProductList, searchProducts, addProduct, updateProduct, deleteProduct等 |
| **购物车** | 7 | ~900 | addToCart, getCart, updateCartItemQuantity, removeFromCart, clearCart等 |
| **库存管理** | 3 | ~400 | checkStock, updateStock, getLowStockProducts |
| **订单管理** | 12 | ~1800 | createOrderFromCart, getOrderList, getOrderDetail, payOrder, cancelOrder等 |
| **地址管理** | 4 | ~300 | addUserAddress, getUserAddresses, deleteUserAddress, setDefaultAddress |
| **优惠券** | 5 | ~600 | getAvailableCoupons, getUserCoupons, assignCoupon, claimCoupon, validateCoupon |
| **评论系统** | 3 | ~400 | addProductReview, getProductReviews, getUserReviews |
| **权限管理** | 4 | ~500 | verifyAdminPermission, getUserRoles, setUserRole, checkUserPermission |
| **促销活动** | 4 | ~600 | getActivePromotions, createPromotion, calculateDiscount, applyCoupon |
| **售后服务** | 3 | ~400 | createAfterSaleRequest, getAfterSaleRequests, processAfterSaleRequest |
| **主题系统** | 3 | ~300 | getAvailableThemes, setUserTheme, getUserTheme |
| **并发控制** | 4 | ~500 | acquireProductLock, releaseProductLock, getProductLockStatus, setProductLimitQuantity |
| **数据分析** | 3 | ~400 | getSalesStatistics, getUserBehaviorAnalysis, getPopularProducts |
| **支付系统** | 3 | ~500 | processPayment, getPaymentStatus, refundPayment |
| **系统监控** | 4 | ~300 | getServerStatus, getSystemLogs, getSystemMetrics, getActiveConnections |
| **数据库操作** | 4 | ~500 | executeDMLQuery, executeSelectQuery, getDatabaseSchema, executeBatch |
| **缓存管理** | 2 | ~100 | clearCache, getCacheStats |
| **管理员功能** | 3 | ~300 | getAllUsers, searchUsers, getSystemStatistics |

---

## 🎯 重构策略

### 方案选择: **渐进式重构**

**原因**:
- 文件过大,一次性重构风险高
- 需要保持现有功能正常运行
- 便于逐步测试和验证

### 重构步骤

#### 阶段1: 准备工作 (当前)
- [x] 分析现有代码结构
- [x] 识别功能模块
- [ ] 创建模块划分方案
- [ ] 备份原始文件

#### 阶段2: 创建服务层头文件
创建以下头文件定义接口:
- `services/UserService.h` - 用户认证和管理
- `services/ProductService.h` - 商品管理
- `services/CartService.h` - 购物车管理
- `services/OrderService.h` - 订单管理
- `services/StockService.h` - 库存管理
- `services/DatabaseManager.h` - 数据库连接管理

#### 阶段3: 创建服务层实现文件
- `services/UserService.cpp`
- `services/ProductService.cpp`
- `services/CartService.cpp`
- `services/OrderService.cpp`
- `services/StockService.cpp`
- `services/DatabaseManager.cpp`

#### 阶段4: 迁移JNI包装层
- 保留主文件 `emshop_native_impl_oop.cpp` 作为JNI入口
- JNI函数只负责参数转换和调用服务层
- 业务逻辑全部移至服务层

#### 阶段5: 测试验证
- 编译验证
- 功能测试
- 性能测试

---

## 📁 目标文件结构

```
cpp/
├── emshop_native_impl_oop.cpp          # JNI入口文件(仅包含JNI函数包装)
├── emshop_native_impl_oop_backup.cpp   # 原始文件备份
├── emshop_EmshopNativeInterface.h      # JNI头文件
├── ConfigLoader.h                       # 配置加载器
├── nlohmann_json.hpp                    # JSON库
├── ErrorCodes.h                         # 错误码定义
├── services/                            # 服务层目录
│   ├── DatabaseManager.h                # 数据库连接管理(头文件)
│   ├── DatabaseManager.cpp              # 数据库连接管理(实现)
│   ├── UserService.h                    # 用户服务(头文件)
│   ├── UserService.cpp                  # 用户服务(实现)
│   ├── ProductService.h                 # 商品服务
│   ├── ProductService.cpp
│   ├── CartService.h                    # 购物车服务
│   ├── CartService.cpp
│   ├── OrderService.h                   # 订单服务
│   ├── OrderService.cpp
│   ├── StockService.h                   # 库存服务
│   ├── StockService.cpp
│   ├── CouponService.h                  # 优惠券服务
│   ├── CouponService.cpp
│   ├── ReviewService.h                  # 评论服务
│   ├── ReviewService.cpp
│   └── PaymentService.h                 # 支付服务
│       └── PaymentService.cpp
└── build_modular.bat                    # 模块化编译脚本
```

---

## 🔧 实现细节

### DatabaseManager (数据库管理)

**职责**:
- 数据库连接池管理
- 连接获取和释放
- 事务管理
- 连接健康检查

**关键类**:
```cpp
class DatabaseManager {
private:
    std::queue<MYSQL*> connectionPool;
    std::mutex poolMutex;
    std::condition_variable poolCondition;
    static DatabaseManager* instance;
    
public:
    static DatabaseManager& getInstance();
    MYSQL* getConnection(int timeout = 30);
    void releaseConnection(MYSQL* conn);
    bool executeQuery(const std::string& query);
    // ... 其他方法
};
```

---

### UserService (用户服务)

**职责**:
- 用户登录/注册/登出
- 用户信息查询和更新
- 用户权限验证
- 地址管理

**关键方法**:
```cpp
class UserService {
public:
    std::string login(const std::string& username, const std::string& password);
    std::string registerUser(const std::string& username, const std::string& password, const std::string& phone);
    std::string getUserInfo(jlong userId);
    std::string updateUserInfo(jlong userId, const std::string& jsonInfo);
    std::string addUserAddress(jlong userId, const std::string& address);
    std::string getUserAddresses(jlong userId);
    // ... 其他方法
};
```

---

### ProductService (商品服务)

**职责**:
- 商品列表查询
- 商品详情查询
- 商品搜索
- 商品CRUD操作(管理员)
- 分类管理

**关键方法**:
```cpp
class ProductService {
public:
    std::string getProductList(const std::string& category, int page, int pageSize);
    std::string getProductDetail(jlong productId);
    std::string searchProducts(const std::string& keyword, int page, int pageSize);
    std::string addProduct(const std::string& jsonProduct);
    std::string updateProduct(jlong productId, const std::string& jsonProduct);
    std::string deleteProduct(jlong productId);
    std::string getCategories();
    // ... 其他方法
};
```

---

### CartService (购物车服务)

**职责**:
- 添加商品到购物车
- 更新购物车数量
- 删除购物车商品
- 清空购物车
- 获取购物车摘要

**关键方法**:
```cpp
class CartService {
public:
    std::string addToCart(jlong userId, jlong productId, int quantity);
    std::string getCart(jlong userId);
    std::string updateCartItemQuantity(jlong userId, jlong productId, int quantity);
    std::string removeFromCart(jlong userId, jlong productId);
    std::string clearCart(jlong userId);
    std::string getCartSummary(jlong userId);
    // ... 其他方法
};
```

---

### OrderService (订单服务)

**职责**:
- 创建订单(从购物车/直接购买)
- 订单查询(列表/详情)
- 订单状态管理
- 订单支付
- 订单取消/删除
- 退款处理

**关键方法**:
```cpp
class OrderService {
public:
    std::string createOrderFromCart(jlong userId, jlong addressId, const std::string& couponCode, const std::string& remark);
    std::string createOrderDirect(jlong userId, jlong productId, int quantity, jlong addressId);
    std::string getOrderList(jlong userId);
    std::string getOrderDetail(jlong orderId);
    std::string payOrder(jlong orderId, const std::string& paymentMethod);
    std::string cancelOrder(jlong userId, jlong orderId);
    std::string deleteOrder(jlong orderId);
    std::string updateOrderStatus(jlong orderId, const std::string& status);
    // ... 其他方法
};
```

---

### StockService (库存服务)

**职责**:
- 库存查询
- 库存更新(扣减/增加/设置)
- 低库存商品查询
- 库存锁定(并发控制)

**关键方法**:
```cpp
class StockService {
public:
    std::string checkStock(jlong productId);
    std::string updateStock(jlong productId, int quantity, const std::string& operation);
    std::string getLowStockProducts(int threshold);
    std::string acquireProductLock(jlong productId, jlong userId, int quantity);
    std::string releaseProductLock(jlong productId, jlong userId);
    // ... 其他方法
};
```

---

## 🚀 迁移示例

### 示例: login函数迁移

#### 迁移前 (emshop_native_impl_oop.cpp)
```cpp
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_login
  (JNIEnv *env, jclass, jstring jusername, jstring jpassword) {
    // 1. 参数转换
    const char* username_cstr = env->GetStringUTFChars(jusername, nullptr);
    std::string username(username_cstr);
    env->ReleaseStringUTFChars(jusername, username_cstr);
    
    // 2. 业务逻辑 (直接在JNI函数中)
    MYSQL* conn = DatabaseManager::getInstance().getConnection(30);
    // ... 200+ 行业务逻辑代码 ...
    
    // 3. 返回结果
    return env->NewStringUTF(result.c_str());
}
```

#### 迁移后

**services/UserService.h**
```cpp
#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <string>
#include "DatabaseManager.h"

class UserService {
public:
    static UserService& getInstance();
    std::string login(const std::string& username, const std::string& password);
    
private:
    UserService() = default;
    static UserService* instance;
};

#endif
```

**services/UserService.cpp**
```cpp
#include "UserService.h"
#include "nlohmann_json.hpp"

UserService* UserService::instance = nullptr;

UserService& UserService::getInstance() {
    if (!instance) {
        instance = new UserService();
    }
    return *instance;
}

std::string UserService::login(const std::string& username, const std::string& password) {
    // 业务逻辑 (从JNI函数迁移过来)
    MYSQL* conn = DatabaseManager::getInstance().getConnection(30);
    // ... 业务逻辑代码 ...
    return result;
}
```

**emshop_native_impl_oop.cpp (JNI包装层)**
```cpp
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_login
  (JNIEnv *env, jclass, jstring jusername, jstring jpassword) {
    try {
        // 1. 参数转换
        const char* username_cstr = env->GetStringUTFChars(jusername, nullptr);
        std::string username(username_cstr);
        env->ReleaseStringUTFChars(jusername, username_cstr);
        
        const char* password_cstr = env->GetStringUTFChars(jpassword, nullptr);
        std::string password(password_cstr);
        env->ReleaseStringUTFChars(jpassword, password_cstr);
        
        // 2. 调用服务层
        std::string result = UserService::getInstance().login(username, password);
        
        // 3. 返回结果
        return env->NewStringUTF(result.c_str());
    } catch (const std::exception& e) {
        // 错误处理
        json error = {{"success", false}, {"message", e.what()}};
        return env->NewStringUTF(error.dump().c_str());
    }
}
```

**优势**:
- JNI函数从200+行减少到20行
- 业务逻辑独立可测试
- 便于复用和维护

---

## 📝 编译脚本更新

### build_modular.bat

```batch
@echo off
echo ========================================
echo  编译模块化JNI库
echo ========================================

set JDK_HOME=C:\Program Files\Java\jdk-21
set MYSQL_HOME=D:\MySQL
set MINGW_HOME=D:\mingw\x86_64-15.2.0-release-win32-seh-ucrt-rt_v13-rev0\mingw64

echo 清理旧文件...
del /Q bin\*.o bin\*.dll 2>nul

echo 编译服务层模块...
"%MINGW_HOME%\bin\g++.exe" -c services\DatabaseManager.cpp -o bin\DatabaseManager.o ^
    -I"%JDK_HOME%\include" -I"%JDK_HOME%\include\win32" -I"%MYSQL_HOME%\include" ^
    -std=c++17 -Wall -fPIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX

"%MINGW_HOME%\bin\g++.exe" -c services\UserService.cpp -o bin\UserService.o ^
    -I"%JDK_HOME%\include" -I"%JDK_HOME%\include\win32" -I"%MYSQL_HOME%\include" ^
    -std=c++17 -Wall -fPIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX

"%MINGW_HOME%\bin\g++.exe" -c services\ProductService.cpp -o bin\ProductService.o ^
    -I"%JDK_HOME%\include" -I"%JDK_HOME%\include\win32" -I"%MYSQL_HOME%\include" ^
    -std=c++17 -Wall -fPIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX

REM ... 其他服务模块 ...

echo 编译JNI入口文件...
"%MINGW_HOME%\bin\g++.exe" -c emshop_native_impl_oop.cpp -o bin\emshop_native_impl_oop.o ^
    -I"%JDK_HOME%\include" -I"%JDK_HOME%\include\win32" -I"%MYSQL_HOME%\include" ^
    -std=c++17 -Wall -fPIC -DWIN32_LEAN_AND_MEAN -DNOMINMAX

echo 链接生成DLL...
"%MINGW_HOME%\bin\g++.exe" -shared -o bin\emshop_native_oop.dll ^
    bin\DatabaseManager.o bin\UserService.o bin\ProductService.o bin\CartService.o bin\OrderService.o ^
    bin\emshop_native_impl_oop.o ^
    -L"%MYSQL_HOME%\lib" -lmysql -Wl,--out-implib,bin\emshop_native_oop.lib

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo  编译成功!
    echo  输出文件: bin\emshop_native_oop.dll
    echo ========================================
) else (
    echo.
    echo ========================================
    echo  编译失败! 错误代码: %ERRORLEVEL%
    echo ========================================
    exit /b %ERRORLEVEL%
)

pause
```

---

## ⚠️ 风险和注意事项

### 潜在风险
1. **编译链接错误**: 多文件编译可能出现符号冲突
2. **内存管理**: 跨模块的对象生命周期管理
3. **线程安全**: 单例模式需要线程安全实现
4. **性能退化**: 函数调用层次增加可能影响性能

### 缓解措施
1. **逐步迁移**: 每次只迁移一个模块,立即测试
2. **保留备份**: 原始文件完整保留
3. **增量编译**: 使用Makefile或CMake管理依赖
4. **性能测试**: 对比重构前后性能指标

---

## 📊 预期收益

### 代码质量
- **可维护性**: ⭐⭐⭐⭐⭐ (单一职责,易于理解)
- **可测试性**: ⭐⭐⭐⭐⭐ (服务层独立可测)
- **可扩展性**: ⭐⭐⭐⭐⭐ (新增功能只需添加服务类)
- **代码复用**: ⭐⭐⭐⭐⭐ (服务层可跨JNI调用复用)

### 文件大小
- **主文件**: 9902行 → ~500行 (95%减少)
- **单个服务文件**: 平均200-400行
- **总代码量**: 略有增加(约+10%,因为增加了接口定义)

### 编译时间
- **全量编译**: 略有增加 (+10-20%)
- **增量编译**: 大幅减少 (-70%,只编译修改的模块)

---

## ✅ 验收标准

### 功能验证
- [ ] 所有82个JNI函数正常工作
- [ ] 通过现有的Java单元测试
- [ ] 通过集成测试

### 性能验证
- [ ] 响应时间不超过原实现的110%
- [ ] 内存占用不超过原实现的120%
- [ ] 并发能力不低于原实现

### 代码质量
- [ ] 通过C++静态分析工具检查
- [ ] 无内存泄漏(Valgrind验证)
- [ ] 符合C++17标准
- [ ] 文档注释完整

---

## 🔄 实施时间表

| 阶段 | 任务 | 预计工时 | 状态 |
|------|------|---------|------|
| 1 | 创建规划文档 | 2小时 | ✅ 完成 |
| 2 | 创建服务层头文件 | 4小时 | ⏳ 进行中 |
| 3 | 迁移DatabaseManager | 3小时 | ⏸️ 待开始 |
| 4 | 迁移UserService | 4小时 | ⏸️ 待开始 |
| 5 | 迁移ProductService | 4小时 | ⏸️ 待开始 |
| 6 | 迁移CartService | 3小时 | ⏸️ 待开始 |
| 7 | 迁移OrderService | 5小时 | ⏸️ 待开始 |
| 8 | 迁移其他服务 | 6小时 | ⏸️ 待开始 |
| 9 | 更新JNI包装层 | 4小时 | ⏸️ 待开始 |
| 10 | 编译测试 | 3小时 | ⏸️ 待开始 |
| 11 | 功能验证 | 4小时 | ⏸️ 待开始 |
| 12 | 性能测试 | 2小时 | ⏸️ 待开始 |
| **总计** | | **44小时** | |

---

## 📚 参考资料

- [cpp/OOP_DESIGN_GUIDE.md](OOP_DESIGN_GUIDE.md) - OOP设计指南
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [JNI Specification](https://docs.oracle.com/en/java/javase/21/docs/specs/jni/)
- [Modern C++ Design Patterns](https://refactoring.guru/design-patterns/cpp)

---

_文档创建时间: 2025年10月12日_  
_最后更新: 2025年10月12日_  
_版本: v1.0_
