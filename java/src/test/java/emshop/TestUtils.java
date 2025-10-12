package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;

/**
 * 测试工具类
 * 提供通用测试辅助功能
 */
public class TestUtils {
    
    private static final ObjectMapper objectMapper = new ObjectMapper();
    
    /**
     * 从测试资源加载JSON配置
     */
    public static JsonNode loadTestConfig() throws Exception {
        try (InputStream is = TestUtils.class.getClassLoader()
                .getResourceAsStream("test-config.json")) {
            if (is == null) {
                throw new IllegalStateException("test-config.json not found in test resources");
            }
            return objectMapper.readTree(is);
        }
    }
    
    /**
     * 加载测试SQL脚本
     */
    public static String loadSqlScript(String scriptName) throws Exception {
        try (InputStream is = TestUtils.class.getClassLoader()
                .getResourceAsStream("sql/" + scriptName)) {
            if (is == null) {
                throw new IllegalStateException("SQL script not found: " + scriptName);
            }
            return new String(is.readAllBytes(), StandardCharsets.UTF_8);
        }
    }
    
    /**
     * 生成随机用户名
     */
    public static String randomUsername() {
        return "test_user_" + System.currentTimeMillis() + "_" + 
               (int)(Math.random() * 10000);
    }
    
    /**
     * 生成随机邮箱
     */
    public static String randomEmail() {
        return "test_" + System.currentTimeMillis() + "@example.com";
    }
    
    /**
     * 生成随机手机号
     */
    public static String randomPhone() {
        return "138" + String.format("%08d", (int)(Math.random() * 100000000));
    }
    
    /**
     * 等待指定毫秒数
     */
    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
    
    /**
     * 解析JSON字符串
     */
    public static JsonNode parseJson(String json) throws Exception {
        return objectMapper.readTree(json);
    }
    
    /**
     * 对象转JSON字符串
     */
    public static String toJson(Object obj) throws Exception {
        return objectMapper.writeValueAsString(obj);
    }
}
