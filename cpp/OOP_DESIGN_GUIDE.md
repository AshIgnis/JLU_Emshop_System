# JLU Emshop System - 面向对象重构说明

## 项目概述

本项目基于面向对象设计原则对原有的 C++ JNI 实现进行了全面重构，采用现代 C++17 标准，提高了代码的可维护性、可复用性和扩展性。

## 面向对象设计原则

### 1. 单一职责原则 (SRP)

每个类只负责一个明确的功能领域：

- `DatabaseConfig`: 仅负责数据库配置管理  
- `DatabaseConnectionPool`: 仅负责数据库连接池管理  
- `UserService`: 仅负责用户相关业务逻辑
- `ProductService`: 仅负责商品相关业务逻辑
- `CartService`: 仅负责购物车相关业务逻辑
- `OrderService`: 仅负责订单相关业务逻辑

### 2. 开放封闭原则 (OCP)

- 通过继承 `BaseService` 类，新的服务类可以扩展功能而无需修改现有代码
- 数据库配置可以通过配置类扩展新的参数，而不影响现有功能

### 3. 里氏替换原则 (LSP)

- 所有服务类继承自 `BaseService`，可以互相替换使用基础功能
- 保持了父类和子类的行为一致性

### 4. 接口隔离原则 (ISP)

- 每个服务类只暴露与其职责相关的接口
- JNI 接口按功能模块分组，避免客户端依赖不必要的接口

### 5. 依赖倒置原则 (DIP)

- 高层模块（业务逻辑）不依赖低层模块（数据库操作）的具体实现
- 通过抽象的数据库连接池接口实现解耦

## 设计模式应用

### 1. 单例模式 (Singleton)

```cpp
class DatabaseConfig {
public:
    static DatabaseConfig& getInstance() {
        static DatabaseConfig instance;
        return instance;
    }
private:
    DatabaseConfig() = default;
    DatabaseConfig(const DatabaseConfig&) = delete;
    DatabaseConfig& operator=(const DatabaseConfig&) = delete;
};
```

**应用场景**：

- `DatabaseConfig`: 全局数据库配置管理
- `DatabaseConnectionPool`: 全局连接池管理
- `EmshopServiceManager`: 全局服务管理器

**优势**：

- 确保全局只有一个实例
- 线程安全的懒加载初始化
- 避免资源重复创建

### 2. 工厂模式 (Factory)

```cpp
class EmshopServiceManager {
public:
    UserService& getUserService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *user_service_;
    }
    // 其他服务的工厂方法...
};
```

**应用场景**：

- 服务类的创建和管理
- 统一的服务获取接口

**优势**：

- 封装对象创建逻辑
- 便于管理对象生命周期
- 支持延迟初始化

### 3. 模板方法模式 (Template Method)

```cpp
class BaseService {
protected:
    json createSuccessResponse(const json& data = json::object(), 
                              const std::string& message = "操作成功");
    json createErrorResponse(const std::string& message, int error_code = 1001);
    void logInfo(const std::string& message);
    void logError(const std::string& message);
};
```

**应用场景**：

- 统一的响应格式创建
- 标准化的日志记录
- 通用的异常处理

**优势**：

- 定义算法骨架，子类实现具体步骤
- 提高代码复用性
- 保持接口一致性

### 4. 对象池模式 (Object Pool)

```cpp
class DatabaseConnectionPool {
private:
    std::queue<MYSQL*> available_connections_;
    std::mutex pool_mutex_;
    std::condition_variable connection_available_;
    
public:
    MYSQL* getConnection();
    void returnConnection(MYSQL* conn);
};
```

**应用场景**：

- 数据库连接复用
- 减少连接创建开销

**优势**：

- 提高性能，减少资源创建销毁开销
- 控制资源使用量
- 线程安全的资源管理

## 类层次结构

```
BaseService
├── UserService
├── ProductService  
├── CartService
└── OrderService

DatabaseConfig (Singleton)
DatabaseConnectionPool (Singleton)
EmshopServiceManager (Singleton)
```

## 关键特性

### 1. 线程安全

- 使用 `std::mutex` 保护共享资源
- `std::condition_variable` 实现线程同步
- 原子操作保证数据一致性

### 2. 资源管理

- RAII 原则管理数据库连接
- 智能指针自动内存管理
- 异常安全的资源释放

### 3. 错误处理

- 统一的异常处理机制
- 结构化的错误响应格式
- 详细的日志记录系统

### 4. 性能优化

- 连接池减少连接创建开销
- 预编译语句提高查询性能
- 批量操作减少数据库交互

## 可扩展性

### 1. 新增服务类

只需继承 `BaseService` 并在 `EmshopServiceManager` 中注册：

```cpp
class PaymentService : public BaseService {
public:
    json processPayment(long orderId, const json& paymentInfo) {
        // 支付处理逻辑
        return createSuccessResponse(json::object(), "支付成功");
    }
};
```

### 2. 新增数据源

可以通过继承或组合扩展不同的数据源支持：

```cpp
class RedisConnectionPool {
    // Redis 连接池实现
};

class CacheService : public BaseService {
private:
    RedisConnectionPool& redis_pool_;
public:
    json getCache(const std::string& key);
    json setCache(const std::string& key, const std::string& value);
};
```

### 3. 新增业务逻辑

通过组合模式扩展复杂业务逻辑：

```cpp
class OrderProcessingService {
private:
    OrderService& order_service_;
    PaymentService& payment_service_;
    InventoryService& inventory_service_;
    
public:
    json processCompleteOrder(long userId, const json& orderInfo) {
        // 组合多个服务完成复杂业务流程
    }
};
```

## 性能考虑

### 1. 内存管理

- 使用 `std::unique_ptr` 和 `std::shared_ptr` 自动管理内存
- 避免内存泄漏和野指针问题
- 合理的对象生命周期管理

### 2. 并发性能

- 读写锁分离提高并发读取性能
- 无锁编程技术在适当场景下使用
- 线程池管理减少线程创建开销

### 3. 数据库性能

- 连接池复用减少连接建立时间
- 预编译语句减少SQL解析开销
- 批量操作减少网络往返

## 测试策略

### 1. 单元测试

```cpp
class UserServiceTest {
public:
    void testLogin() {
        UserService service;
        json result = service.login("testuser", "testpass");
        assert(result["success"].get<bool>());
    }
};
```

### 2. 集成测试

- 测试服务间交互
- 验证数据库事务一致性
- 检查并发场景下的正确性

### 3. 性能测试

- 连接池性能基准测试
- 高并发场景压力测试
- 内存使用情况监控

## 部署和运维

### 1. 配置管理

- 数据库连接参数可配置
- 日志级别可调整
- 连接池大小可优化

### 2. 监控指标

- 连接池使用率
- API 调用成功率
- 响应时间统计
- 错误率统计

### 3. 故障恢复

- 数据库连接自动重连
- 服务降级机制
- 优雅的错误处理

## 总结

通过面向对象的重构，本项目实现了：

1. **高内聚低耦合**：每个类职责明确，类间依赖最小化
2. **可维护性提升**：代码结构清晰，易于理解和修改
3. **可扩展性增强**：新功能可以通过继承和组合轻松添加
4. **可测试性改善**：模块化设计便于单元测试和集成测试
5. **性能优化**：合理的资源管理和缓存策略
6. **线程安全**：多线程环境下的数据一致性保证

这种设计为后续的功能扩展和系统维护奠定了良好的基础，符合企业级应用开发的最佳实践。
