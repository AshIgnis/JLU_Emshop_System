package emshop;

/**
 * JNI集成测试类
 * 用于测试Java与C++的JNI连接是否正常
 */
public class JNITest {
    
    public static void main(String[] args) {
        System.out.println("=== JLU Emshop System JNI 集成测试 ===\n");
        
        testNativeInterfaceLoading();
        testBasicFunctions();
        testDatabaseOperations();
        
        System.out.println("\n=== 测试完成 ===");
    }
    
    /**
     * 测试JNI库加载
     */
    private static void testNativeInterfaceLoading() {
        System.out.println("1. 测试JNI库加载...");
        
        try {
            // 尝试加载JNI库
            System.out.println("   - 尝试加载emshop native库...");
            
            // 通过调用一个简单的方法来测试库是否正确加载
            String result = EmshopNativeInterface.getServerStatus();
            System.out.println("   ✓ JNI库加载成功");
            System.out.println("   - 服务器状态: " + result);
            
        } catch (UnsatisfiedLinkError e) {
            System.out.println("   ✗ JNI库未找到或加载失败: " + e.getMessage());
            System.out.println("   - 将使用Mock数据进行测试");
            System.out.println("   - 请确保编译了C++库并放在正确的位置");
        } catch (Exception e) {
            System.out.println("   - JNI调用异常: " + e.getMessage());
        }
    }
    
    /**
     * 测试基本功能
     */
    private static void testBasicFunctions() {
        System.out.println("\n2. 测试基本功能...");
        
        // 测试用户注册
        System.out.println("   - 测试用户注册...");
        try {
            String registerResult = EmshopNativeInterface.register("testuser", "123456", "13800138000");
            System.out.println("     注册结果: " + registerResult);
        } catch (UnsatisfiedLinkError e) {
            System.out.println("     JNI库未加载，无法测试注册功能");
        }
        
        // 测试用户登录
        System.out.println("   - 测试用户登录...");
        try {
            String loginResult = EmshopNativeInterface.login("testuser", "123456");
            System.out.println("     登录结果: " + loginResult);
        } catch (UnsatisfiedLinkError e) {
            System.out.println("     JNI库未加载，无法测试登录功能");
        }
        
        // 测试获取商品列表
        System.out.println("   - 测试获取商品列表...");
        try {
            String productsResult = EmshopNativeInterface.getProductList("电子产品", 1, 5);
            System.out.println("     商品列表: " + productsResult);
        } catch (UnsatisfiedLinkError e) {
            System.out.println("     JNI库未加载，无法测试商品列表功能");
        }
    }    /**
     * 测试数据库操作
     */
    private static void testDatabaseOperations() {
        System.out.println("\n3. 测试数据库操作...");
        
        // 测试数据库连接
        System.out.println("   - 测试数据库连接...");
        try {
            String dbSchemaResult = EmshopNativeInterface.getDatabaseSchema();
            System.out.println("     数据库模式: " + (dbSchemaResult.length() > 100 ? 
                dbSchemaResult.substring(0, 100) + "..." : dbSchemaResult));
        } catch (UnsatisfiedLinkError e) {
            System.out.println("     Mock数据库模式: {\"tables\": [\"users\", \"products\", \"orders\"]}");
        }
        
        // 测试查询执行
        System.out.println("   - 测试SELECT查询...");
        try {
            String queryResult = EmshopNativeInterface.executeSelectQuery(
                "SELECT COUNT(*) as count FROM users", 
                "{}"
            );
            System.out.println("     查询结果: " + queryResult);
        } catch (UnsatisfiedLinkError e) {
            System.out.println("     Mock查询结果: {\"success\": true, \"data\": [{\"count\": \"2\"}]}");
        }
    }
    
    /**
     * 性能测试
     */
    public static void performanceTest() {
        System.out.println("\n=== JNI性能测试 ===");
        
        int iterations = 1000;
        long startTime = System.currentTimeMillis();
        
        for (int i = 0; i < iterations; i++) {
            try {
                EmshopNativeInterface.getProductList("all", 1, 10);
            } catch (UnsatisfiedLinkError e) {
                System.out.println("JNI库未加载，无法进行性能测试");
                return;
            }
        }
        
        long endTime = System.currentTimeMillis();
        long duration = endTime - startTime;
        
        System.out.println("执行 " + iterations + " 次getProductList调用");
        System.out.println("总耗时: " + duration + " ms");
        System.out.println("平均耗时: " + (duration / (double)iterations) + " ms/call");
        System.out.println("吞吐量: " + (iterations * 1000 / duration) + " calls/sec");
    }
}
