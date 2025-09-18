package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * JNI 接口测试客户端
 * 测试与 C++ 实现的数据库接口连接
 */
public class JNITestClient {
    
    private static final ObjectMapper objectMapper = new ObjectMapper();
    
    public static void main(String[] args) {
        System.out.println("JNI Interface Test Client");
        System.out.println("==========================");
        
        try {
            // 测试数据库连接
            testDatabaseConnection();
            
            // 测试登录功能
            testLoginFunction();
            
            // 测试商品查询
            testProductQuery();
            
        } catch (Exception e) {
            System.err.println("Test failed with error: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * 测试数据库连接
     */
    private static void testDatabaseConnection() {
        System.out.println("\n1. Testing Database Connection...");
        try {
            // 尝试调用一个简单的数据库方法
            String result = EmshopNativeInterface.getInitializationStatus();
            System.out.println("Initialization Status: " + result);
            
            JsonNode resultNode = objectMapper.readTree(result);
            // 检查 initialized 字段而不是 success 字段
            if (resultNode.has("initialized") && resultNode.get("initialized").asBoolean()) {
                System.out.println("✓ Database connection successful");
                String dbStatus = resultNode.has("database_pool_status") ? 
                    resultNode.get("database_pool_status").asText() : "未知";
                System.out.println("Database Pool Status: " + dbStatus);
            } else {
                System.out.println("✗ Database connection failed: " + resultNode.get("message").asText());
            }
        } catch (Exception e) {
            System.out.println("✗ Database connection test failed: " + e.getMessage());
        }
    }
    
    /**
     * 测试登录功能
     */
    private static void testLoginFunction() {
        System.out.println("\n2. Testing Login Function...");
        try {
            // 测试用户登录
            String result = EmshopNativeInterface.login("testuser", "testpass");
            System.out.println("Login Result: " + result);
            
            JsonNode resultNode = objectMapper.readTree(result);
            if (resultNode.get("success").asBoolean()) {
                System.out.println("✓ Login function working correctly");
            } else {
                System.out.println("✗ Login failed (expected for test credentials): " + resultNode.get("message").asText());
            }
        } catch (Exception e) {
            System.out.println("✗ Login function test failed: " + e.getMessage());
        }
    }
    
    /**
     * 测试商品查询功能
     */
    private static void testProductQuery() {
        System.out.println("\n3. Testing Product Query...");
        try {
            String result = EmshopNativeInterface.getProductList("all", 1, 5);
            System.out.println("Product Query Result: " + result);
            
            JsonNode resultNode = objectMapper.readTree(result);
            if (resultNode.get("success").asBoolean()) {
                System.out.println("✓ Product query function working correctly");
                JsonNode products = resultNode.get("data").get("products");
                System.out.println("Found " + products.size() + " products");
            } else {
                System.out.println("✗ Product query failed: " + resultNode.get("message").asText());
            }
        } catch (Exception e) {
            System.out.println("✗ Product query test failed: " + e.getMessage());
        }
    }
}