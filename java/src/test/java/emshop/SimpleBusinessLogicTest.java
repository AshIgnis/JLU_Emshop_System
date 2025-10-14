package emshop;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * 简单业务逻辑测试 - v1.1.0
 * 直接测试新增的本地方法
 */
public class SimpleBusinessLogicTest {
    
    private static final ObjectMapper mapper = new ObjectMapper();
    
    public static void main(String[] args) {
        System.out.println("========================================");
        System.out.println("业务逻辑改进测试 - v1.1.0");
        System.out.println("========================================\n");
        
        // 加载 native 库
        try {
            System.loadLibrary("emshop_native_oop");
            System.out.println("✅ Native library loaded successfully\n");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("❌ Failed to load native library: " + e.getMessage());
            return;
        }
        
        // 运行所有测试
        testGetCouponTemplates();
        testCalculateCouponDiscount();
        testGetAvailableCouponsForOrder();
        testGetNotifications();
        testGetRefundRequests();
        testGetUserRefundRequests();
        
        System.out.println("\n========================================");
        System.out.println("所有测试完成!");
        System.out.println("========================================");
    }
    
    private static void testGetCouponTemplates() {
        System.out.println("[测试 1] 获取优惠券模板列表");
        
        try {
            String result = EmshopNativeInterface.getCouponTemplates();
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                int count = json.get("data").size();
                System.out.println("✅ 成功 - 获取到 " + count + " 个优惠券模板\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
    
    private static void testCalculateCouponDiscount() {
        System.out.println("[测试 2] 计算优惠券折扣");
        
        try {
            String result = EmshopNativeInterface.calculateCouponDiscount("NEWUSER2025", 100.0);
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                JsonNode data = json.get("data");
                double discount = data.get("discount_amount").asDouble();
                System.out.println("✅ 成功 - 折扣金额: " + discount + "\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
    
    private static void testGetAvailableCouponsForOrder() {
        System.out.println("[测试 3] 获取订单可用优惠券");
        
        try {
            String result = EmshopNativeInterface.getAvailableCouponsForOrder(1, 200.0);
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                int count = json.get("data").size();
                System.out.println("✅ 成功 - 找到 " + count + " 个可用优惠券\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
    
    private static void testGetNotifications() {
        System.out.println("[测试 4] 获取用户通知");
        
        try {
            String result = EmshopNativeInterface.getNotifications(1, false);
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                int count = json.get("data").size();
                System.out.println("✅ 成功 - 用户有 " + count + " 条通知\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
    
    private static void testGetRefundRequests() {
        System.out.println("[测试 5] 获取所有退款申请(管理员)");
        
        try {
            String result = EmshopNativeInterface.getRefundRequests("pending", 1, 10);
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                JsonNode data = json.get("data");
                int total = data.get("total").asInt();
                System.out.println("✅ 成功 - 共有 " + total + " 个待处理退款申请\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
    
    private static void testGetUserRefundRequests() {
        System.out.println("[测试 6] 获取用户退款申请列表");
        
        try {
            String result = EmshopNativeInterface.getUserRefundRequests(1);
            System.out.println("Response: " + result);
            
            JsonNode json = mapper.readTree(result);
            int code = json.get("code").asInt();
            
            if (code == 200) {
                int count = json.get("data").size();
                System.out.println("✅ 成功 - 用户有 " + count + " 个退款申请\n");
            } else {
                System.out.println("⚠️  返回码: " + code + ", 消息: " + json.get("message").asText() + "\n");
            }
        } catch (Exception e) {
            System.err.println("❌ 测试失败: " + e.getMessage() + "\n");
        }
    }
}
