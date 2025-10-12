package emshop;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * 业务日志工具类
 * 提供统一的业务日志记录接口
 */
public class BusinessLogger {
    
    private static final Logger logger = LoggerFactory.getLogger("emshop.business");
    
    /**
     * 记录用户登录日志
     */
    public static void logLogin(long userId, String username, String ip, boolean success) {
        if (success) {
            logger.info("用户登录成功 | userId={} | username={} | ip={}", 
                       userId, username, ip);
        } else {
            logger.warn("用户登录失败 | username={} | ip={}", username, ip);
        }
    }
    
    /**
     * 记录用户注册日志
     */
    public static void logRegister(long userId, String username, String email, String ip) {
        logger.info("用户注册成功 | userId={} | username={} | email={} | ip={}", 
                   userId, username, email, ip);
    }
    
    /**
     * 记录订单创建日志
     */
    public static void logOrderCreate(long orderId, long userId, String username, 
                                     double totalAmount, double finalAmount) {
        logger.info("订单创建成功 | orderId={} | userId={} | username={} | totalAmount={} | finalAmount={}", 
                   orderId, userId, username, totalAmount, finalAmount);
    }
    
    /**
     * 记录订单取消日志
     */
    public static void logOrderCancel(long orderId, long userId, String username, String reason) {
        logger.info("订单取消 | orderId={} | userId={} | username={} | reason={}", 
                   orderId, userId, username, reason);
    }
    
    /**
     * 记录退款申请日志
     */
    public static void logRefundRequest(long orderId, long userId, String username, String reason) {
        logger.info("退款申请 | orderId={} | userId={} | username={} | reason={}", 
                   orderId, userId, username, reason);
    }
    
    /**
     * 记录库存变动日志
     */
    public static void logStockChange(long productId, String productName, 
                                     int changeQuantity, int stockBefore, int stockAfter, 
                                     String changeType, String reason) {
        logger.info("库存变动 | productId={} | productName={} | changeQuantity={} | " +
                   "stockBefore={} | stockAfter={} | changeType={} | reason={}", 
                   productId, productName, changeQuantity, stockBefore, stockAfter, 
                   changeType, reason);
    }
    
    /**
     * 记录支付日志
     */
    public static void logPayment(long orderId, long userId, String paymentMethod, 
                                  double amount, boolean success) {
        if (success) {
            logger.info("支付成功 | orderId={} | userId={} | paymentMethod={} | amount={}", 
                       orderId, userId, paymentMethod, amount);
        } else {
            logger.warn("支付失败 | orderId={} | userId={} | paymentMethod={} | amount={}", 
                       orderId, userId, paymentMethod, amount);
        }
    }
    
    /**
     * 记录优惠券使用日志
     */
    public static void logCouponUse(long couponId, String couponCode, long userId, 
                                   String username, long orderId, double discountAmount) {
        logger.info("优惠券使用 | couponId={} | couponCode={} | userId={} | username={} | " +
                   "orderId={} | discountAmount={}", 
                   couponId, couponCode, userId, username, orderId, discountAmount);
    }
    
    /**
     * 记录商品查看日志(用于统计热门商品)
     */
    public static void logProductView(long productId, String productName, long userId) {
        logger.info("商品查看 | productId={} | productName={} | userId={}", 
                   productId, productName, userId);
    }
    
    /**
     * 记录购物车操作日志
     */
    public static void logCartOperation(String operation, long userId, long productId, int quantity) {
        logger.info("购物车操作 | operation={} | userId={} | productId={} | quantity={}", 
                   operation, userId, productId, quantity);
    }
    
    /**
     * 记录异常业务日志
     */
    public static void logBusinessError(String operation, String errorCode, String message, 
                                       long userId, String details) {
        logger.error("业务异常 | operation={} | errorCode={} | message={} | userId={} | details={}", 
                    operation, errorCode, message, userId, details);
    }
}
