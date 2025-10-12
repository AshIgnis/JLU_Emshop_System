# 测试框架使用指南

## 概述

本项目已配置完整的JUnit 5测试框架,包含以下测试工具:

- **JUnit 5 (Jupiter)**: 现代化的Java测试框架
- **Mockito**: Mock对象创建和验证
- **AssertJ**: 流式断言库,提供更好的可读性
- **Testcontainers**: Docker容器化测试(数据库测试)
- **Logback**: 测试日志记录

---

## 快速开始

### 运行所有测试

```bash
cd java
mvn test
```

### 运行特定测试类

```bash
mvn test -Dtest=EmshopNativeInterfaceTest
mvn test -Dtest=StockManagementTest
```

### 运行特定测试方法

```bash
mvn test -Dtest=EmshopNativeInterfaceTest#testUserRegistration
```

### 跳过测试构建

```bash
mvn clean package -DskipTests
```

---

## 测试类说明

### 1. EmshopNativeInterfaceTest

**位置**: `src/test/java/emshop/EmshopNativeInterfaceTest.java`

**功能**: 测试JNI接口的基础功能

**测试用例**:
- ✅ 用户注册功能
- ✅ 用户登录功能
- ✅ 错误密码登录
- ✅ 查询商品列表
- ✅ 无效请求格式处理
- ✅ 缺少action字段处理
- ✅ 并发请求处理

**前置条件**:
- JNI库 `emshop_native_oop.dll` 已编译
- 数据库已初始化并运行
- 配置文件 `config.json` 正确设置

### 2. StockManagementTest

**位置**: `src/test/java/emshop/StockManagementTest.java`

**功能**: 测试P0-2任务实现的库存管理和审计功能

**测试用例**:
- ✅ 订单创建时库存扣减及审计记录
- ✅ 取消订单时库存返还及审计记录
- ✅ 申请退款时库存返还及审计记录
- ✅ 库存不足时订单创建失败

**验证点**:
- 库存数量变化正确
- `stock_changes` 数组返回完整信息
- 审计表 `stock_change_audit` 记录写入
- 审计表 `order_status_audit` 记录写入
- 事务回滚机制正常

---

## 测试配置

### logback-test.xml

测试日志配置文件,位于 `src/test/resources/logback-test.xml`

**日志级别**:
- 测试代码: DEBUG
- Netty: INFO
- Testcontainers: INFO

**输出位置**:
- 控制台: 彩色输出
- 文件: `logs/test.log` (滚动策略: 10MB/文件, 保留7天)

### test-config.json

测试专用配置,位于 `src/test/resources/test-config.json`

```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "name": "emshop_test",
    "username": "root",
    "password": "test_password"
  },
  "test": {
    "use_testcontainers": true,
    "mock_external_services": true,
    "timeout_seconds": 30
  }
}
```

---

## 测试工具类

### TestUtils

**位置**: `src/test/java/emshop/TestUtils.java`

**提供的工具方法**:
- `loadTestConfig()`: 加载测试配置
- `randomUsername()`: 生成随机用户名
- `randomEmail()`: 生成随机邮箱
- `randomPhone()`: 生成随机手机号
- `parseJson(String)`: 解析JSON字符串
- `toJson(Object)`: 对象转JSON
- `sleep(long)`: 线程等待

**使用示例**:
```java
String username = TestUtils.randomUsername();
String email = TestUtils.randomEmail();
var json = TestUtils.parseJson(response);
```

---

## 断言风格

推荐使用AssertJ流式断言,提高可读性:

```java
// 基础断言
assertThat(response).isNotNull();
assertThat(response).contains("\"success\":true");

// 数值断言
assertThat(stockQuantity).isGreaterThan(0);
assertThat(stockQuantity).isEqualTo(expectedStock);

// 集合断言
assertThat(products).isNotEmpty();
assertThat(products).hasSize(10);

// JSON断言
assertThat(json.has("data")).isTrue();
assertThat(json.get("success").asBoolean()).isTrue();
```

---

## 测试顺序控制

使用 `@Order` 注解控制测试执行顺序:

```java
@TestMethodOrder(MethodOrderer.OrderAnnotation.class)
public class MyTest {
    
    @Test
    @Order(1)
    void firstTest() { }
    
    @Test
    @Order(2)
    void secondTest() { }
}
```

---

## 测试前置/后置

```java
@BeforeAll
static void setupClass() {
    // 整个测试类执行前运行一次
    System.loadLibrary("emshop_native_oop");
}

@BeforeEach
void setup() {
    // 每个测试方法执行前运行
    nativeInterface = new EmshopNativeInterface();
}

@AfterEach
void teardown() {
    // 每个测试方法执行后运行
    // 清理测试数据
}

@AfterAll
static void cleanup() {
    // 整个测试类执行后运行一次
}
```

---

## 测试数据管理

### 原则
1. 每个测试使用独立的测试数据
2. 使用随机数据避免冲突 (`TestUtils.randomUsername()`)
3. 测试后清理临时数据(在 `@AfterEach` 中)
4. 不依赖数据库初始数据的具体内容

### 示例
```java
@Test
void testUserRegistration() {
    String username = TestUtils.randomUsername(); // test_user_1234567890_5678
    String email = TestUtils.randomEmail();       // test_1234567890@example.com
    
    // 注册测试...
}
```

---

## 并发测试

测试并发场景:

```java
@Test
void testConcurrentRequests() throws InterruptedException {
    int threadCount = 10;
    Thread[] threads = new Thread[threadCount];
    
    for (int i = 0; i < threadCount; i++) {
        threads[i] = new Thread(() -> {
            String response = nativeInterface.handleRequest(request);
            assertThat(response).contains("\"success\":true");
        });
        threads[i].start();
    }
    
    for (Thread thread : threads) {
        thread.join();
    }
}
```

---

## Mock外部依赖

使用Mockito模拟外部依赖:

```java
@ExtendWith(MockitoExtension.class)
public class ServiceTest {
    
    @Mock
    private PaymentGateway paymentGateway;
    
    @InjectMocks
    private OrderService orderService;
    
    @Test
    void testPayment() {
        when(paymentGateway.charge(anyDouble()))
            .thenReturn(PaymentResult.success());
        
        // 测试逻辑...
        
        verify(paymentGateway, times(1)).charge(100.0);
    }
}
```

---

## 参数化测试

测试多个输入场景:

```java
@ParameterizedTest
@ValueSource(strings = {"", "a", "ab", "123"})
void testInvalidUsername(String username) {
    String request = String.format("{\"action\":\"register\",\"username\":\"%s\"}", username);
    String response = nativeInterface.handleRequest(request);
    assertThat(response).contains("\"success\":false");
}

@ParameterizedTest
@CsvSource({
    "1, 10, 10",
    "2, 20, 40",
    "5, 15, 75"
})
void testPriceCalculation(int quantity, double price, double expected) {
    double total = quantity * price;
    assertThat(total).isEqualTo(expected);
}
```

---

## 异常测试

测试异常情况:

```java
@Test
void testNullRequest() {
    assertThrows(NullPointerException.class, () -> {
        nativeInterface.handleRequest(null);
    });
}

@Test
void testInvalidJson() {
    String response = nativeInterface.handleRequest("{invalid}");
    assertThat(response).contains("\"success\":false");
}
```

---

## 测试覆盖率

生成测试覆盖率报告(需要添加JaCoCo插件):

```bash
mvn clean test jacoco:report
```

查看报告: `target/site/jacoco/index.html`

---

## 常见问题

### 1. JNI库加载失败

**错误**: `UnsatisfiedLinkError: no emshop_native_oop in java.library.path`

**解决方案**:
- 确保 `emshop_native_oop.dll` 已编译
- 将DLL路径添加到 `java.library.path`:
  ```bash
  mvn test -Djava.library.path="D:\path\to\dll"
  ```
- 或复制DLL到 `java/src/main/java/emshop/` 目录

### 2. 数据库连接失败

**错误**: `Communications link failure`

**解决方案**:
- 确保MySQL服务运行中
- 检查 `config.json` 数据库配置
- 运行数据库初始化脚本:
  ```bash
  cd cpp
  init_emshop_database.bat
  ```

### 3. 测试超时

**错误**: `TimeoutException`

**解决方案**:
- 增加测试超时时间:
  ```java
  @Test
  @Timeout(value = 30, unit = TimeUnit.SECONDS)
  void testLongOperation() { }
  ```

### 4. 端口冲突

**错误**: `Address already in use: bind`

**解决方案**:
- 使用不同的测试端口 (test-config.json中设置为18080)
- 或关闭占用端口的进程

---

## 最佳实践

### 1. 测试命名

```java
// Good
@Test
@DisplayName("测试用户注册成功")
void testUserRegistration() { }

// Better
@Test
@DisplayName("测试使用有效邮箱和密码注册用户成功")
void testUserRegistrationWithValidEmailAndPassword() { }
```

### 2. 断言消息

```java
// Good
assertThat(stock).isGreaterThan(0);

// Better  
assertThat(stock)
    .as("商品库存应大于0")
    .isGreaterThan(0);
```

### 3. 测试隔离

每个测试应独立运行,不依赖其他测试:

```java
@Test
void test1() {
    // 创建自己的测试数据
    String username = TestUtils.randomUsername();
    // ...
}

@Test
void test2() {
    // 不依赖test1的数据
    String username = TestUtils.randomUsername();
    // ...
}
```

### 4. Given-When-Then模式

```java
@Test
void testOrderCreation() {
    // Given: 准备测试数据
    long userId = createTestUser();
    addProductToCart(userId, productId, 2);
    
    // When: 执行操作
    String response = createOrder(userId);
    
    // Then: 验证结果
    assertThat(response).contains("\"success\":true");
    assertThat(getStock(productId)).isEqualTo(originalStock - 2);
}
```

---

## 待扩展测试

根据TODO.md,以下功能需要添加测试:

### P0级别
- [x] 库存管理和审计 (StockManagementTest)
- [ ] 配置加载和安全性
- [ ] 事务回滚机制

### P1级别
- [ ] 优惠券应用逻辑
- [ ] 订单状态转换
- [ ] 权限验证

### P2级别
- [ ] 促销策略计算
- [ ] 物流信息更新
- [ ] 地址管理

### P3级别
- [ ] 性能压力测试
- [ ] 并发库存扣减
- [ ] 缓存一致性

---

## 持续集成

在GitHub Actions中自动运行测试:

```yaml
name: Java CI

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Set up JDK 21
        uses: actions/setup-java@v3
        with:
          java-version: '21'
      - name: Run tests
        run: cd java && mvn test
```

---

**最后更新**: 2025-01-13  
**维护者**: JLU Emshop 开发团队
