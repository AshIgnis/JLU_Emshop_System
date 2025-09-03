package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * 简化的OOP JNI测试类
 * 直接测试修复后的原始实现
 */
public class SimpleOOPTest {
    
    // 加载修复后的原始JNI库
    static {
        try {
            System.loadLibrary("emshop_native_oop_fixed");
            System.out.println("成功加载修复后的JNI库: emshop_native_oop_fixed");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("加载JNI库失败: " + e.getMessage());
            throw e;
        }
    }
    
    // 只声明我们要测试的方法
    public static native String register(String username, String password, String phone);
    public static native String login(String username, String password);
    
    private static ObjectMapper objectMapper = new ObjectMapper();
    
    public static void main(String[] args) {
        System.out.println("=== 修复后的OOP JNI简单测试 ===\n");
        
        try {
            testUserRegistration();
            testUserLogin();
            System.out.println("\n=== 所有测试完成！===");
        } catch (Exception e) {
            System.err.println("测试出现异常: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    private static void testUserRegistration() {
        System.out.println("1. 测试用户注册...");
        
        try {
            String username = "fixed_user_" + System.currentTimeMillis();
            String password = "testpass123";
            String phone = "13800138000";
            
            System.out.println("注册用户: " + username);
            String result = register(username, password, phone);
            
            JsonNode jsonResult = objectMapper.readTree(result);
            boolean success = jsonResult.get("success").asBoolean();
            String message = jsonResult.get("message").asText();
            
            System.out.println("注册结果: " + (success ? "成功" : "失败"));
            System.out.println("消息: " + message);
            
            if (jsonResult.has("userId")) {
                long userId = jsonResult.get("userId").asLong();
                System.out.println("用户ID: " + userId);
            }
            
        } catch (Exception e) {
            System.err.println("注册测试失败: " + e.getMessage());
            throw new RuntimeException(e);
        }
        System.out.println();
    }
    
    private static void testUserLogin() {
        System.out.println("2. 测试用户登录...");
        
        try {
            String username = "admin";  // 使用已存在的用户
            String password = "admin123";
            
            System.out.println("登录用户: " + username);
            String result = login(username, password);
            
            JsonNode jsonResult = objectMapper.readTree(result);
            boolean success = jsonResult.get("success").asBoolean();
            String message = jsonResult.get("message").asText();
            
            System.out.println("登录结果: " + (success ? "成功" : "失败"));
            System.out.println("消息: " + message);
            
            if (jsonResult.has("userId")) {
                long userId = jsonResult.get("userId").asLong();
                System.out.println("用户ID: " + userId);
            }
            
        } catch (Exception e) {
            System.err.println("登录测试失败: " + e.getMessage());
            e.printStackTrace();
        }
        System.out.println();
    }
}
