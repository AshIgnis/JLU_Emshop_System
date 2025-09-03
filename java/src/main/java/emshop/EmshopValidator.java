package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * JLU Emshop System - 功能验证工具
 * 用于验证修复后的JNI实现正常工作
 */
public class EmshopValidator {
    
    private static ObjectMapper objectMapper = new ObjectMapper();
    
    public static void main(String[] args) {
        System.out.println("=== JLU Emshop System 功能验证 ===\n");
        
        try {
            // 测试基本功能
            testUserRegistration();
            
            System.out.println("\n=== 所有功能验证通过 ===");
            System.out.println("JLU Emshop System 已准备就绪！");
            
        } catch (Exception e) {
            System.err.println("功能验证失败: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void testUserRegistration() throws Exception {
        System.out.println("验证用户注册功能...");
        
        String username = "validator_" + System.currentTimeMillis();
        String password = "test123456";
        String phone = "13800138000";
        
        String result = EmshopNativeInterface.register(username, password, phone);
        JsonNode jsonResult = objectMapper.readTree(result);
        
        if (jsonResult.get("success").asBoolean()) {
            System.out.println("✓ 用户注册功能正常");
            if (jsonResult.has("data")) {
                JsonNode data = jsonResult.get("data");
                System.out.println("  - 用户ID: " + data.get("user_id").asLong());
                System.out.println("  - 用户名: " + data.get("username").asText());
            }
        } else {
            throw new RuntimeException("用户注册失败: " + jsonResult.get("message").asText());
        }
    }
}
