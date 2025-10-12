package emshop;

/**
 * 统一错误码枚举
 * 与ERROR_CODES.md文档保持同步
 * 
 * @author JLU Emshop Team
 * @version 1.0.0
 * @date 2025-10-12
 */
public enum ErrorCode {
    
    // ==================== 通用模块 (00) ====================
    
    // 系统级错误 (00x)
    SUCCESS("S00000", "操作成功", "Success"),
    DB_CONNECTION_ERROR("E00001", "数据库连接失败", "Database connection failed"),
    DB_QUERY_ERROR("E00002", "数据库查询失败", "Database query error"),
    CONFIG_LOAD_ERROR("E00003", "配置加载失败", "Config file load error"),
    JNI_ERROR("E00004", "系统内部错误", "JNI invocation error"),
    OUT_OF_MEMORY("E00005", "系统资源不足", "Out of memory"),
    
    // 验证错误 (01x)
    MISSING_PARAMETER("E00101", "缺少必要参数", "Missing required parameter"),
    INVALID_JSON("E00102", "请求格式错误", "Invalid JSON format"),
    MISSING_ACTION("E00103", "缺少操作类型", "Missing action field"),
    INVALID_ACTION("E00104", "不支持的操作", "Invalid action value"),
    INVALID_PARAMETER_TYPE("E00105", "参数类型不正确", "Invalid parameter type"),
    
    // 未知错误 (09x)
    UNKNOWN_ERROR("E00901", "系统异常,请联系管理员", "Unknown error"),
    
    // ==================== 用户认证模块 (10) ====================
    
    // 验证错误 (11x)
    USERNAME_REQUIRED("E10101", "请输入用户名", "Username is required"),
    PASSWORD_REQUIRED("E10102", "请输入密码", "Password is required"),
    INVALID_EMAIL_FORMAT("E10103", "邮箱格式不正确", "Invalid email format"),
    INVALID_PHONE_FORMAT("E10104", "手机号格式不正确", "Invalid phone number"),
    INVALID_USERNAME_LENGTH("E10105", "用户名需3-20个字符", "Username length invalid"),
    INVALID_PASSWORD_LENGTH("E10106", "密码需6-20个字符", "Password length invalid"),
    
    // 业务错误 (12x)
    INVALID_CREDENTIALS("E10201", "用户名或密码错误", "Invalid username or password"),
    USERNAME_EXISTS("E10202", "该用户名已被注册", "Username already exists"),
    EMAIL_EXISTS("E10203", "该邮箱已被注册", "Email already registered"),
    PHONE_EXISTS("E10204", "该手机号已被注册", "Phone already registered"),
    
    // 权限错误 (13x)
    TOKEN_MISSING("E10301", "请先登录", "Token is missing"),
    TOKEN_INVALID("E10302", "登录已失效,请重新登录", "Invalid token"),
    TOKEN_EXPIRED("E10303", "登录已过期,请重新登录", "Token expired"),
    PERMISSION_DENIED("E10304", "您没有权限执行此操作", "Permission denied"),
    
    // 资源错误 (14x)
    USER_NOT_FOUND("E10401", "用户不存在", "User not found"),
    
    // ==================== 商品管理模块 (20) ====================
    
    // 验证错误 (21x)
    PRODUCT_ID_REQUIRED("E20101", "请提供商品ID", "Product ID is required"),
    PRODUCT_NAME_REQUIRED("E20102", "请输入商品名称", "Product name is required"),
    INVALID_PRODUCT_PRICE("E20103", "商品价格必须大于0", "Invalid product price"),
    INVALID_STOCK_QUANTITY("E20104", "库存数量不能为负", "Invalid stock quantity"),
    INVALID_CATEGORY_ID("E20105", "无效的分类ID", "Invalid category ID"),
    
    // 业务错误 (22x)
    INSUFFICIENT_STOCK("E20201", "商品库存不足", "Insufficient stock"),
    PRODUCT_UNAVAILABLE("E20202", "商品已下架,无法购买", "Product is unavailable"),
    PRODUCT_EXISTS("E20203", "商品已存在", "Product already exists"),
    
    // 资源错误 (24x)
    PRODUCT_NOT_FOUND("E20401", "商品不存在", "Product not found"),
    CATEGORY_NOT_FOUND("E20402", "分类不存在", "Category not found"),
    
    // ==================== 购物车模块 (30) ====================
    
    // 验证错误 (31x)
    CART_PRODUCT_ID_REQUIRED("E30101", "请选择商品", "Product ID is required"),
    INVALID_QUANTITY("E30102", "商品数量必须大于0", "Invalid quantity"),
    INVALID_CART_ITEM_ID("E30103", "购物车项不存在", "Invalid cart item ID"),
    
    // 业务错误 (32x)
    CART_EMPTY("E30201", "购物车为空", "Cart is empty"),
    CART_INSUFFICIENT_STOCK("E30202", "商品库存不足,请调整数量", "Insufficient stock for cart items"),
    PRODUCT_ALREADY_IN_CART("E30203", "商品已在购物车中", "Product already in cart"),
    
    // 资源错误 (34x)
    CART_ITEM_NOT_FOUND("E30401", "购物车项不存在", "Cart item not found"),
    
    // ==================== 订单管理模块 (40) ====================
    
    // 验证错误 (41x)
    ORDER_ID_REQUIRED("E40101", "请提供订单ID", "Order ID is required"),
    SHIPPING_ADDRESS_REQUIRED("E40102", "请填写收货地址", "Shipping address is required"),
    INVALID_ORDER_AMOUNT("E40103", "订单金额计算错误", "Invalid order amount"),
    ORDER_ITEMS_EMPTY("E40104", "订单明细不能为空", "Order items cannot be empty"),
    
    // 业务错误 (42x)
    ORDER_INSUFFICIENT_STOCK("E40201", "部分商品库存不足", "Insufficient stock for some items"),
    INVALID_ORDER_STATUS("E40202", "当前订单状态不允许此操作", "Invalid order status"),
    ORDER_ALREADY_PAID("E40203", "订单已支付,无法取消", "Order already paid"),
    ORDER_ALREADY_SHIPPED("E40204", "订单已发货,无法取消", "Order already shipped"),
    ORDER_CANNOT_REFUND("E40205", "订单状态不允许退款", "Order cannot be refunded"),
    
    // 权限错误 (43x)
    NO_PERMISSION_FOR_ORDER("E40301", "您无权操作此订单", "No permission for this order"),
    
    // 资源错误 (44x)
    ORDER_NOT_FOUND("E40401", "订单不存在", "Order not found"),
    ORDER_ITEMS_NOT_FOUND("E40402", "订单明细不存在", "Order items not found"),
    
    // ==================== 支付系统模块 (50) ====================
    
    // 验证错误 (51x)
    INVALID_PAYMENT_METHOD("E50101", "请选择支付方式", "Invalid payment method"),
    PAYMENT_AMOUNT_MISMATCH("E50102", "支付金额与订单金额不符", "Payment amount mismatch"),
    
    // 业务错误 (52x)
    PAYMENT_FAILED("E50201", "支付失败,请重试", "Payment failed"),
    INSUFFICIENT_BALANCE("E50202", "账户余额不足", "Insufficient balance"),
    PAYMENT_TIMEOUT("E50203", "支付超时,请重新下单", "Payment timeout"),
    DUPLICATE_PAYMENT("E50204", "订单已支付", "Duplicate payment"),
    
    // 系统错误 (53x)
    PAYMENT_GATEWAY_ERROR("E50301", "支付系统异常", "Payment gateway error"),
    REFUND_FAILED("E50302", "退款处理失败", "Refund failed"),
    
    // ==================== 优惠券模块 (60) ====================
    
    // 验证错误 (61x)
    COUPON_CODE_REQUIRED("E60101", "请输入优惠券代码", "Coupon code is required"),
    
    // 业务错误 (62x)
    COUPON_UNAVAILABLE("E60201", "优惠券不可用", "Coupon is unavailable"),
    COUPON_EXPIRED("E60202", "优惠券已过期", "Coupon expired"),
    COUPON_OUT_OF_STOCK("E60203", "优惠券已领完", "Coupon out of stock"),
    COUPON_REQUIREMENT_NOT_MET("E60204", "订单金额不满足优惠券使用条件", "Order amount does not meet coupon requirement"),
    COUPON_ALREADY_CLAIMED("E60205", "您已领取过此优惠券", "Coupon already claimed"),
    COUPON_ALREADY_USED("E60206", "优惠券已使用", "Coupon already used"),
    
    // 资源错误 (64x)
    COUPON_NOT_FOUND("E60401", "优惠券不存在", "Coupon not found"),
    
    // ==================== 促销活动模块 (70) ====================
    
    // 验证错误 (71x)
    INVALID_PROMOTION_ID("E70101", "促销活动ID无效", "Invalid promotion ID"),
    
    // 业务错误 (72x)
    PROMOTION_NOT_STARTED("E70201", "促销活动尚未开始", "Promotion not started"),
    PROMOTION_ENDED("E70202", "促销活动已结束", "Promotion ended"),
    PROMOTION_CONFLICT("E70203", "该商品已参与其他促销", "Promotion conflict"),
    
    // 资源错误 (74x)
    PROMOTION_NOT_FOUND("E70401", "促销活动不存在", "Promotion not found"),
    
    // ==================== 地址管理模块 (80) ====================
    
    // 验证错误 (81x)
    ADDRESS_ID_REQUIRED("E80101", "请提供地址ID", "Address ID is required"),
    RECEIVER_NAME_REQUIRED("E80102", "请填写收货人姓名", "Receiver name is required"),
    PHONE_NUMBER_REQUIRED("E80103", "请填写手机号", "Phone number is required"),
    DETAILED_ADDRESS_REQUIRED("E80104", "请填写详细地址", "Detailed address is required"),
    
    // 业务错误 (82x)
    ADDRESS_LIMIT_EXCEEDED("E80201", "最多只能添加10个地址", "Address limit exceeded"),
    
    // 资源错误 (84x)
    ADDRESS_NOT_FOUND("E80401", "地址不存在", "Address not found"),
    
    // ==================== 系统管理模块 (90) ====================
    
    // 权限错误 (93x)
    ADMIN_PERMISSION_REQUIRED("E90301", "需要管理员权限", "Admin permission required"),
    OPERATION_DENIED("E90302", "操作被拒绝", "Operation denied"),
    
    // 系统错误 (90x)
    LOG_WRITE_ERROR("E90001", "日志记录失败", "Log write error"),
    AUDIT_LOG_ERROR("E90002", "审计记录失败", "Audit log error");
    
    // ==================== 枚举属性 ====================
    
    private final String code;
    private final String messageZh;
    private final String messageEn;
    
    /**
     * 构造函数
     * @param code 错误码
     * @param messageZh 中文消息
     * @param messageEn 英文消息
     */
    ErrorCode(String code, String messageZh, String messageEn) {
        this.code = code;
        this.messageZh = messageZh;
        this.messageEn = messageEn;
    }
    
    // ==================== Getters ====================
    
    public String getCode() {
        return code;
    }
    
    public String getMessageZh() {
        return messageZh;
    }
    
    public String getMessageEn() {
        return messageEn;
    }
    
    /**
     * 获取本地化消息(默认中文)
     */
    public String getMessage() {
        return messageZh;
    }
    
    /**
     * 获取指定语言的消息
     * @param locale 语言("zh"或"en")
     */
    public String getMessage(String locale) {
        if ("en".equalsIgnoreCase(locale)) {
            return messageEn;
        }
        return messageZh;
    }
    
    // ==================== 静态工具方法 ====================
    
    /**
     * 根据错误码字符串查找ErrorCode枚举
     * @param code 错误码
     * @return ErrorCode枚举,未找到返回null
     */
    public static ErrorCode fromCode(String code) {
        if (code == null || code.isEmpty()) {
            return null;
        }
        for (ErrorCode errorCode : ErrorCode.values()) {
            if (errorCode.code.equals(code)) {
                return errorCode;
            }
        }
        return null;
    }
    
    /**
     * 根据错误码获取中文消息
     * @param code 错误码
     * @return 中文消息,未找到返回错误码本身
     */
    public static String getMessageZh(String code) {
        ErrorCode errorCode = fromCode(code);
        return errorCode != null ? errorCode.messageZh : code;
    }
    
    /**
     * 根据错误码获取英文消息
     * @param code 错误码
     * @return 英文消息,未找到返回错误码本身
     */
    public static String getMessageEn(String code) {
        ErrorCode errorCode = fromCode(code);
        return errorCode != null ? errorCode.messageEn : code;
    }
    
    /**
     * 检查是否为成功响应码
     * @param code 响应码
     * @return true=成功, false=错误
     */
    public static boolean isSuccess(String code) {
        return SUCCESS.code.equals(code);
    }
    
    /**
     * 检查是否为系统级错误
     * @param code 错误码
     * @return true=系统错误, false=业务错误
     */
    public static boolean isSystemError(String code) {
        if (code == null || code.length() < 5) {
            return false;
        }
        // 检查模块代码是否为00,或错误类型是否为0
        return code.substring(1, 3).equals("00") || code.charAt(3) == '0';
    }
    
    /**
     * 获取错误码所属模块
     * @param code 错误码
     * @return 模块名称
     */
    public static String getModule(String code) {
        if (code == null || code.length() < 3) {
            return "未知模块";
        }
        String moduleCode = code.substring(1, 3);
        switch (moduleCode) {
            case "00": return "通用模块";
            case "10": return "用户认证";
            case "20": return "商品管理";
            case "30": return "购物车";
            case "40": return "订单管理";
            case "50": return "支付系统";
            case "60": return "优惠券";
            case "70": return "促销活动";
            case "80": return "地址管理";
            case "90": return "系统管理";
            default: return "未知模块";
        }
    }
    
    /**
     * 获取错误类型描述
     * @param code 错误码
     * @return 错误类型
     */
    public static String getErrorType(String code) {
        if (code == null || code.length() < 4) {
            return "未知类型";
        }
        char typeCode = code.charAt(3);
        switch (typeCode) {
            case '0': return "系统级错误";
            case '1': return "验证错误";
            case '2': return "业务错误";
            case '3': return "权限错误";
            case '4': return "资源错误";
            case '9': return "未知错误";
            default: return "其他错误";
        }
    }
    
    @Override
    public String toString() {
        return String.format("ErrorCode{code='%s', messageZh='%s', messageEn='%s'}", 
                           code, messageZh, messageEn);
    }
}
