package emshop;

import org.junit.jupiter.api.*;
import static org.assertj.core.api.Assertions.*;

/**
 * ErrorCode枚举测试类
 */
@DisplayName("ErrorCode枚举测试")
public class ErrorCodeTest {
    
    @Test
    @DisplayName("测试错误码基本属性")
    void testErrorCodeProperties() {
        ErrorCode error = ErrorCode.INVALID_CREDENTIALS;
        
        assertThat(error.getCode()).isEqualTo("E10201");
        assertThat(error.getMessageZh()).isEqualTo("用户名或密码错误");
        assertThat(error.getMessageEn()).isEqualTo("Invalid username or password");
        assertThat(error.getMessage()).isEqualTo(error.getMessageZh());
    }
    
    @Test
    @DisplayName("测试根据错误码查找枚举")
    void testFromCode() {
        ErrorCode error = ErrorCode.fromCode("E20201");
        
        assertThat(error).isNotNull();
        assertThat(error).isEqualTo(ErrorCode.INSUFFICIENT_STOCK);
        assertThat(error.getMessageZh()).isEqualTo("商品库存不足");
    }
    
    @Test
    @DisplayName("测试错误码不存在")
    void testFromCodeNotFound() {
        ErrorCode error = ErrorCode.fromCode("E99999");
        assertThat(error).isNull();
    }
    
    @Test
    @DisplayName("测试获取本地化消息")
    void testGetMessage() {
        ErrorCode error = ErrorCode.TOKEN_EXPIRED;
        
        assertThat(error.getMessage("zh")).isEqualTo("登录已过期,请重新登录");
        assertThat(error.getMessage("en")).isEqualTo("Token expired");
        assertThat(error.getMessage("unknown")).isEqualTo(error.getMessageZh());
    }
    
    @Test
    @DisplayName("测试成功响应码判断")
    void testIsSuccess() {
        assertThat(ErrorCode.isSuccess("S00000")).isTrue();
        assertThat(ErrorCode.isSuccess("E10201")).isFalse();
    }
    
    @Test
    @DisplayName("测试系统级错误判断")
    void testIsSystemError() {
        // 通用模块的系统级错误
        assertThat(ErrorCode.isSystemError("E00001")).isTrue();
        assertThat(ErrorCode.isSystemError("E00002")).isTrue();
        
        // 其他模块的业务错误
        assertThat(ErrorCode.isSystemError("E10201")).isFalse();
        assertThat(ErrorCode.isSystemError("E20201")).isFalse();
        
        // 系统管理模块的系统错误
        assertThat(ErrorCode.isSystemError("E90001")).isTrue();
    }
    
    @Test
    @DisplayName("测试获取模块名称")
    void testGetModule() {
        assertThat(ErrorCode.getModule("E00001")).isEqualTo("通用模块");
        assertThat(ErrorCode.getModule("E10201")).isEqualTo("用户认证");
        assertThat(ErrorCode.getModule("E20201")).isEqualTo("商品管理");
        assertThat(ErrorCode.getModule("E30201")).isEqualTo("购物车");
        assertThat(ErrorCode.getModule("E40201")).isEqualTo("订单管理");
        assertThat(ErrorCode.getModule("E50201")).isEqualTo("支付系统");
        assertThat(ErrorCode.getModule("E60201")).isEqualTo("优惠券");
        assertThat(ErrorCode.getModule("E70201")).isEqualTo("促销活动");
        assertThat(ErrorCode.getModule("E80201")).isEqualTo("地址管理");
        assertThat(ErrorCode.getModule("E90301")).isEqualTo("系统管理");
    }
    
    @Test
    @DisplayName("测试获取错误类型")
    void testGetErrorType() {
        assertThat(ErrorCode.getErrorType("E00001")).isEqualTo("系统级错误");
        assertThat(ErrorCode.getErrorType("E10101")).isEqualTo("验证错误");
        assertThat(ErrorCode.getErrorType("E20201")).isEqualTo("业务错误");
        assertThat(ErrorCode.getErrorType("E10301")).isEqualTo("权限错误");
        assertThat(ErrorCode.getErrorType("E20401")).isEqualTo("资源错误");
        assertThat(ErrorCode.getErrorType("E00901")).isEqualTo("未知错误");
    }
    
    @Test
    @DisplayName("测试静态方法获取消息")
    void testStaticGetMessage() {
        String zhMessage = ErrorCode.getMessageZh("E10201");
        String enMessage = ErrorCode.getMessageEn("E10201");
        
        assertThat(zhMessage).isEqualTo("用户名或密码错误");
        assertThat(enMessage).isEqualTo("Invalid username or password");
        
        // 测试不存在的错误码
        assertThat(ErrorCode.getMessageZh("E99999")).isEqualTo("E99999");
    }
    
    @Test
    @DisplayName("测试所有错误码格式正确")
    void testAllErrorCodesFormat() {
        for (ErrorCode error : ErrorCode.values()) {
            String code = error.getCode();
            
            // 检查错误码长度
            assertThat(code).hasSize(6);
            
            // 检查第一个字符是S或E
            assertThat(code.charAt(0)).isIn('S', 'E');
            
            // 检查后5位都是数字
            for (int i = 1; i < 6; i++) {
                assertThat(Character.isDigit(code.charAt(i)))
                    .as("错误码 %s 的第%d位不是数字", code, i + 1)
                    .isTrue();
            }
            
            // 检查消息不为空
            assertThat(error.getMessageZh()).isNotEmpty();
            assertThat(error.getMessageEn()).isNotEmpty();
        }
    }
    
    @Test
    @DisplayName("测试toString方法")
    void testToString() {
        ErrorCode error = ErrorCode.INSUFFICIENT_STOCK;
        String str = error.toString();
        
        assertThat(str).contains("E20201");
        assertThat(str).contains("商品库存不足");
        assertThat(str).contains("Insufficient stock");
    }
    
    @Test
    @DisplayName("测试错误码唯一性")
    void testErrorCodeUniqueness() {
        ErrorCode[] values = ErrorCode.values();
        
        for (int i = 0; i < values.length; i++) {
            for (int j = i + 1; j < values.length; j++) {
                assertThat(values[i].getCode())
                    .as("错误码重复: %s 和 %s", values[i].name(), values[j].name())
                    .isNotEqualTo(values[j].getCode());
            }
        }
    }
    
    @Test
    @DisplayName("测试错误码数量")
    void testErrorCodeCount() {
        ErrorCode[] values = ErrorCode.values();
        
        // 应该有87个错误码(包括SUCCESS)
        assertThat(values.length).isEqualTo(87);
        
        System.out.println("总共定义了 " + values.length + " 个错误码");
    }
}
