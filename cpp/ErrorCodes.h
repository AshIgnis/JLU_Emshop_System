/**
 * @file ErrorCodes.h
 * @brief 统一错误码常量定义
 * @date 2025-01-13
 * 
 * 定义系统中所有错误码常量,与ERROR_CODES.md文档保持同步
 * 错误码格式: EXXXYY (XX=模块, Y=类型, YY=序号)
 */

#ifndef EMSHOP_ERROR_CODES_H
#define EMSHOP_ERROR_CODES_H

#include <string>

namespace emshop {

/**
 * @class ErrorCodes
 * @brief 错误码常量类
 */
class ErrorCodes {
public:
    // ==================== 通用模块 (00) ====================
    
    // 系统级错误 (00x)
    static constexpr const char* SUCCESS = "S00000";
    static constexpr const char* DB_CONNECTION_ERROR = "E00001";
    static constexpr const char* DB_QUERY_ERROR = "E00002";
    static constexpr const char* CONFIG_LOAD_ERROR = "E00003";
    static constexpr const char* JNI_ERROR = "E00004";
    static constexpr const char* OUT_OF_MEMORY = "E00005";
    
    // 验证错误 (01x)
    static constexpr const char* MISSING_PARAMETER = "E00101";
    static constexpr const char* INVALID_JSON = "E00102";
    static constexpr const char* MISSING_ACTION = "E00103";
    static constexpr const char* INVALID_ACTION = "E00104";
    static constexpr const char* INVALID_PARAMETER_TYPE = "E00105";
    
    // 未知错误 (09x)
    static constexpr const char* UNKNOWN_ERROR = "E00901";
    
    // ==================== 用户认证模块 (10) ====================
    
    // 验证错误 (11x)
    static constexpr const char* USERNAME_REQUIRED = "E10101";
    static constexpr const char* PASSWORD_REQUIRED = "E10102";
    static constexpr const char* INVALID_EMAIL_FORMAT = "E10103";
    static constexpr const char* INVALID_PHONE_FORMAT = "E10104";
    static constexpr const char* INVALID_USERNAME_LENGTH = "E10105";
    static constexpr const char* INVALID_PASSWORD_LENGTH = "E10106";
    
    // 业务错误 (12x)
    static constexpr const char* INVALID_CREDENTIALS = "E10201";
    static constexpr const char* USERNAME_EXISTS = "E10202";
    static constexpr const char* EMAIL_EXISTS = "E10203";
    static constexpr const char* PHONE_EXISTS = "E10204";
    
    // 权限错误 (13x)
    static constexpr const char* TOKEN_MISSING = "E10301";
    static constexpr const char* TOKEN_INVALID = "E10302";
    static constexpr const char* TOKEN_EXPIRED = "E10303";
    static constexpr const char* PERMISSION_DENIED = "E10304";
    
    // 资源错误 (14x)
    static constexpr const char* USER_NOT_FOUND = "E10401";
    
    // ==================== 商品管理模块 (20) ====================
    
    // 验证错误 (21x)
    static constexpr const char* PRODUCT_ID_REQUIRED = "E20101";
    static constexpr const char* PRODUCT_NAME_REQUIRED = "E20102";
    static constexpr const char* INVALID_PRODUCT_PRICE = "E20103";
    static constexpr const char* INVALID_STOCK_QUANTITY = "E20104";
    static constexpr const char* INVALID_CATEGORY_ID = "E20105";
    
    // 业务错误 (22x)
    static constexpr const char* INSUFFICIENT_STOCK = "E20201";
    static constexpr const char* PRODUCT_UNAVAILABLE = "E20202";
    static constexpr const char* PRODUCT_EXISTS = "E20203";
    
    // 资源错误 (24x)
    static constexpr const char* PRODUCT_NOT_FOUND = "E20401";
    static constexpr const char* CATEGORY_NOT_FOUND = "E20402";
    
    // ==================== 购物车模块 (30) ====================
    
    // 验证错误 (31x)
    static constexpr const char* CART_PRODUCT_ID_REQUIRED = "E30101";
    static constexpr const char* INVALID_QUANTITY = "E30102";
    static constexpr const char* INVALID_CART_ITEM_ID = "E30103";
    
    // 业务错误 (32x)
    static constexpr const char* CART_EMPTY = "E30201";
    static constexpr const char* CART_INSUFFICIENT_STOCK = "E30202";
    static constexpr const char* PRODUCT_ALREADY_IN_CART = "E30203";
    
    // 资源错误 (34x)
    static constexpr const char* CART_ITEM_NOT_FOUND = "E30401";
    
    // ==================== 订单管理模块 (40) ====================
    
    // 验证错误 (41x)
    static constexpr const char* ORDER_ID_REQUIRED = "E40101";
    static constexpr const char* SHIPPING_ADDRESS_REQUIRED = "E40102";
    static constexpr const char* INVALID_ORDER_AMOUNT = "E40103";
    static constexpr const char* ORDER_ITEMS_EMPTY = "E40104";
    
    // 业务错误 (42x)
    static constexpr const char* ORDER_INSUFFICIENT_STOCK = "E40201";
    static constexpr const char* INVALID_ORDER_STATUS = "E40202";
    static constexpr const char* ORDER_ALREADY_PAID = "E40203";
    static constexpr const char* ORDER_ALREADY_SHIPPED = "E40204";
    static constexpr const char* ORDER_CANNOT_REFUND = "E40205";
    
    // 权限错误 (43x)
    static constexpr const char* NO_PERMISSION_FOR_ORDER = "E40301";
    
    // 资源错误 (44x)
    static constexpr const char* ORDER_NOT_FOUND = "E40401";
    static constexpr const char* ORDER_ITEMS_NOT_FOUND = "E40402";
    
    // ==================== 支付系统模块 (50) ====================
    
    // 验证错误 (51x)
    static constexpr const char* INVALID_PAYMENT_METHOD = "E50101";
    static constexpr const char* PAYMENT_AMOUNT_MISMATCH = "E50102";
    
    // 业务错误 (52x)
    static constexpr const char* PAYMENT_FAILED = "E50201";
    static constexpr const char* INSUFFICIENT_BALANCE = "E50202";
    static constexpr const char* PAYMENT_TIMEOUT = "E50203";
    static constexpr const char* DUPLICATE_PAYMENT = "E50204";
    
    // 系统错误 (53x)
    static constexpr const char* PAYMENT_GATEWAY_ERROR = "E50301";
    static constexpr const char* REFUND_FAILED = "E50302";
    
    // ==================== 优惠券模块 (60) ====================
    
    // 验证错误 (61x)
    static constexpr const char* COUPON_CODE_REQUIRED = "E60101";
    
    // 业务错误 (62x)
    static constexpr const char* COUPON_UNAVAILABLE = "E60201";
    static constexpr const char* COUPON_EXPIRED = "E60202";
    static constexpr const char* COUPON_OUT_OF_STOCK = "E60203";
    static constexpr const char* COUPON_REQUIREMENT_NOT_MET = "E60204";
    static constexpr const char* COUPON_ALREADY_CLAIMED = "E60205";
    static constexpr const char* COUPON_ALREADY_USED = "E60206";
    
    // 资源错误 (64x)
    static constexpr const char* COUPON_NOT_FOUND = "E60401";
    
    // ==================== 促销活动模块 (70) ====================
    
    // 验证错误 (71x)
    static constexpr const char* INVALID_PROMOTION_ID = "E70101";
    
    // 业务错误 (72x)
    static constexpr const char* PROMOTION_NOT_STARTED = "E70201";
    static constexpr const char* PROMOTION_ENDED = "E70202";
    static constexpr const char* PROMOTION_CONFLICT = "E70203";
    
    // 资源错误 (74x)
    static constexpr const char* PROMOTION_NOT_FOUND = "E70401";
    
    // ==================== 地址管理模块 (80) ====================
    
    // 验证错误 (81x)
    static constexpr const char* ADDRESS_ID_REQUIRED = "E80101";
    static constexpr const char* RECEIVER_NAME_REQUIRED = "E80102";
    static constexpr const char* PHONE_NUMBER_REQUIRED = "E80103";
    static constexpr const char* DETAILED_ADDRESS_REQUIRED = "E80104";
    
    // 业务错误 (82x)
    static constexpr const char* ADDRESS_LIMIT_EXCEEDED = "E80201";
    
    // 资源错误 (84x)
    static constexpr const char* ADDRESS_NOT_FOUND = "E80401";
    
    // ==================== 系统管理模块 (90) ====================
    
    // 权限错误 (93x)
    static constexpr const char* ADMIN_PERMISSION_REQUIRED = "E90301";
    static constexpr const char* OPERATION_DENIED = "E90302";
    
    // 系统错误 (90x)
    static constexpr const char* LOG_WRITE_ERROR = "E90001";
    static constexpr const char* AUDIT_LOG_ERROR = "E90002";
    
    /**
     * @brief 获取错误码对应的中文消息
     * @param errorCode 错误码
     * @return 中文错误消息
     */
    static std::string getChineseMessage(const std::string& errorCode);
    
    /**
     * @brief 获取错误码对应的英文消息
     * @param errorCode 错误码
     * @return 英文错误消息
     */
    static std::string getEnglishMessage(const std::string& errorCode);
    
    /**
     * @brief 检查是否为成功响应码
     * @param code 响应码
     * @return true=成功, false=错误
     */
    static bool isSuccess(const std::string& code) {
        return code == SUCCESS;
    }
    
    /**
     * @brief 检查是否为系统级错误
     * @param errorCode 错误码
     * @return true=系统错误, false=业务错误
     */
    static bool isSystemError(const std::string& errorCode) {
        return errorCode.substr(1, 2) == "00" || errorCode.substr(3, 1) == "0";
    }
    
private:
    ErrorCodes() = delete; // 禁止实例化
};

} // namespace emshop

#endif // EMSHOP_ERROR_CODES_H
