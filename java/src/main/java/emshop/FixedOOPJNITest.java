package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * 修复后的原始OOP JNI测试类
 * 测试完整的emshop_native_impl_oop.cpp功能
 */
public class FixedOOPJNITest {
    
    // 加载修复后的原始JNI库
    static {
        try {
            System.loadLibrary("emshop_native_oop_fixed");
            System.out.println("成功加载修复后的原始JNI库: emshop_native_oop_fixed");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("加载JNI库失败: " + e.getMessage());
            throw e;
        }
    }
    
    // 原始JNI方法声明（来自EmshopNativeInterface）
    public static native String login(String username, String password);
    public static native String register(String username, String password, String phone);
    
    private static ObjectMapper objectMapper = new ObjectMapper();
    
    public static void main(String[] args) {
        System.out.println("=== 修复后的原始OOP JNI测试开始 ===");
        
        try {
            // 1. 测试用户注册
            System.out.println("\n1. 测试用户注册...");
            testUserRegistration();
            
            // 2. 测试用户登录
            System.out.println("\n2. 测试用户登录...");
            testUserLogin();
            
            // 3. 测试异常情况
            System.out.println("\n3. 测试异常情况...");
            testErrorCases();
            
        } catch (Exception e) {
            System.err.println("测试过程中发生异常: " + e.getMessage());
            e.printStackTrace();
        }
        
        System.out.println("\n=== 修复后的原始OOP JNI测试完成 ===");
    }
    
    /**
     * 测试用户注册功能
     */
    private static void testUserRegistration() {
        try {
            String username = "fixed_testuser_" + System.currentTimeMillis();
            String password = "testpass123";
            String phone = "13900139000";
            
            System.out.println("注册用户: " + username);
            String result = register(username, password, phone);
            
            System.out.println("注册响应: " + result);
            
            // 解析JSON响应
            JsonNode jsonNode = objectMapper.readTree(result);
            boolean success = jsonNode.get("success").asBoolean();
            String message = jsonNode.get("message").asText();
            
            System.out.println("注册结果: " + (success ? "成功" : "失败"));
            System.out.println("响应消息: " + message);
            
            if (success && jsonNode.has("data")) {
                JsonNode data = jsonNode.get("data");
                if (data.has("user_id")) {
                    System.out.println("用户ID: " + data.get("user_id").asText());
                }
            }
            
        } catch (Exception e) {
            System.err.println("用户注册测试失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * 测试用户登录功能
     */
    private static void testUserLogin() {
        try {
            // 先注册一个测试用户
            String username = "login_testuser_" + System.currentTimeMillis();
            String password = "loginpass123";
            String phone = "13800138888";
            
            System.out.println("先注册用户: " + username);
            String registerResult = register(username, password, phone);
            System.out.println("注册结果: " + registerResult);
            
            // 然后测试登录
            System.out.println("测试登录用户: " + username);
            String loginResult = login(username, password);
            
            System.out.println("登录响应: " + loginResult);
            
            // 解析JSON响应
            JsonNode jsonNode = objectMapper.readTree(loginResult);
            boolean success = jsonNode.get("success").asBoolean();
            String message = jsonNode.get("message").asText();
            
            System.out.println("登录结果: " + (success ? "成功" : "失败"));
            System.out.println("响应消息: " + message);
            
        } catch (Exception e) {
            System.err.println("用户登录测试失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    /**
     * 测试异常情况
     */
    private static void testErrorCases() {
        System.out.println("测试空参数和错误情况...");
        
        try {
            // 测试空用户名注册
            System.out.println("测试空用户名注册:");
            String result1 = register("", "password", "13800138000");
            System.out.println("结果: " + result1);
            
            // 测试空密码注册
            System.out.println("测试空密码注册:");
            String result2 = register("testuser", "", "13800138000");
            System.out.println("结果: " + result2);
            
            // 测试空用户名登录
            System.out.println("测试空用户名登录:");
            String result3 = login("", "password");
            System.out.println("结果: " + result3);
            
            // 测试不存在的用户登录
            System.out.println("测试不存在的用户登录:");
            String result4 = login("nonexistent_user_999999", "password");
            System.out.println("结果: " + result4);
            
            // 测试重复用户名注册
            System.out.println("测试重复用户名注册:");
            String existingUser = "duplicate_user_test";
            register(existingUser, "password1", "13800138001");
            String duplicateResult = register(existingUser, "password2", "13800138002");
            System.out.println("重复注册结果: " + duplicateResult);
            
        } catch (Exception e) {
            System.err.println("异常情况测试失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
