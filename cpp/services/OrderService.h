/**
 * @file OrderService.h
 * @brief 订单服务类定义
 * @date 2025-01-13
 */

#ifndef ORDER_SERVICE_H
#define ORDER_SERVICE_H

#include <string>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>
#include <unordered_map>

// 前向声明
class BaseService;
using json = nlohmann::json;

/**
 * @class OrderService
 * @brief 订单服务类 - 管理订单的创建、查询、支付、发货和取消
 */
class OrderService : public BaseService {
private:
    std::mutex order_mutex_; ///< 订单操作互斥锁

    // 辅助方法 - 列名映射
    const std::string& getOrderIdColumnName() const;
    const std::string& getOrderNoColumnName() const;
    const std::string& getOrderCreatedAtColumnName() const;
    const std::string& getOrderUpdatedAtColumnName() const;
    const std::string& getUsersPrimaryKeyColumn() const;
    bool orderHasColumn(const std::string& column) const;

    /**
     * @brief 生成唯一订单号
     * @return 订单号字符串(格式: EM+时间戳+毫秒)
     */
    std::string generateOrderNo();

    /**
     * @brief 生成交易ID
     * @return 交易ID字符串
     */
    std::string generateTransactionId();

    /**
     * @brief 验证状态转换是否合法
     * @param from_status 原状态
     * @param to_status 目标状态
     * @return 是否允许此转换
     */
    bool isValidStatusTransition(const std::string& from_status, const std::string& to_status);

    /**
     * @brief 获取当前时间戳
     * @return 格式化的时间字符串
     */
    std::string getCurrentTimestamp();

public:
    /**
     * @brief 构造函数
     */
    OrderService();

    /**
     * @brief 获取服务名称
     * @return 服务名称字符串
     */
    std::string getServiceName() const override;

    /**
     * @brief 从购物车创建订单
     * @param user_id 用户ID
     * @param address_id 收货地址ID
     * @param coupon_code 优惠券代码(可选)
     * @param remark 订单备注(可选)
     * @return JSON响应 包含订单ID、订单号、金额等信息
     * @note 会扣减库存、清空购物车、应用优惠券折扣
     */
    json createOrderFromCart(long user_id, long address_id, const std::string& coupon_code, const std::string& remark);

    /**
     * @brief 直接购买创建订单(不依赖购物车)
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @param quantity 购买数量
     * @param address_id 收货地址ID
     * @param coupon_code 优惠券代码(可选)
     * @param remark 订单备注(可选)
     * @return JSON响应 包含订单信息
     * @note 适用于"立即购买"场景
     */
    json createOrderDirect(long user_id, long product_id, int quantity, long address_id, const std::string& coupon_code, const std::string& remark);

    /**
     * @brief 获取用户订单列表
     * @param user_id 用户ID
     * @return JSON响应 包含用户所有订单
     */
    json getUserOrders(long user_id);

    /**
     * @brief 获取订单详情
     * @param order_id 订单ID
     * @return JSON响应 包含订单基本信息和订单明细
     */
    json getOrderDetail(long order_id);

    /**
     * @brief 删除订单(仅允许已取消的订单)
     * @param order_id 订单ID
     * @return JSON响应 删除结果
     */
    json deleteOrder(long order_id);

    /**
     * @brief 支付订单
     * @param order_id 订单ID
     * @param payment_method 支付方式(如alipay,wechat等)
     * @return JSON响应 支付结果和交易ID
     * @note 模拟支付流程,实际项目需对接第三方支付接口
     */
    json payOrder(long order_id, const std::string& payment_method);

    /**
     * @brief 发货订单(管理员功能)
     * @param order_id 订单ID
     * @param tracking_number 快递单号
     * @param shipping_method 配送方式
     * @return JSON响应 发货结果
     * @note 只有已支付订单才能发货
     */
    json shipOrder(long order_id, const std::string& tracking_number, const std::string& shipping_method);

    /**
     * @brief 确认收货(用户功能)
     * @param order_id 订单ID
     * @return JSON响应 确认结果
     * @note 将订单状态改为completed
     */
    json confirmDelivery(long order_id);

    /**
     * @brief 申请退款(用户功能) - 改进版
     * @param order_id 订单ID
     * @param user_id 用户ID
     * @param reason 退款原因
     * @return JSON响应 退款申请结果,订单状态变为refunding
     * @note 创建退款申请记录,等待管理员审核
     */
    json requestRefund(long order_id, long user_id, const std::string& reason);
    
    /**
     * @brief 审核退款申请(管理员功能)
     * @param refund_id 退款申请ID
     * @param admin_id 管理员ID
     * @param approve 是否批准(true=批准, false=拒绝)
     * @param admin_reply 管理员回复
     * @return JSON响应 审核结果
     * @note 批准后退款并返还库存,拒绝后恢复订单状态
     */
    json approveRefund(long refund_id, long admin_id, bool approve, const std::string& admin_reply);
    
    /**
     * @brief 获取退款申请列表(管理员功能)
     * @param status 状态筛选(all=全部, pending=待审核, approved=已批准, rejected=已拒绝)
     * @param page 页码
     * @param page_size 每页数量
     * @return JSON响应 退款申请列表
     */
    json getRefundRequests(const std::string& status, int page, int page_size);
    
    /**
     * @brief 获取用户的退款申请
     * @param user_id 用户ID
     * @return JSON响应 用户的退款申请列表
     */
    json getUserRefundRequests(long user_id);

    /**
     * @brief 取消订单
     * @param order_id 订单ID
     * @param reason 取消原因
     * @return JSON响应 取消结果
     * @note 只有pending或confirmed状态的订单可取消
     */
    json cancelOrder(long order_id, const std::string& reason);

    /**
     * @brief 更新订单状态(管理员功能)
     * @param order_id 订单ID
     * @param new_status 新状态
     * @return JSON响应 更新结果
     * @note 会验证状态转换的合法性
     */
    json updateOrderStatus(long order_id, const std::string& new_status);

    /**
     * @brief 按状态获取订单列表
     * @param user_id 用户ID
     * @param status 订单状态(all表示全部)
     * @return JSON响应 符合条件的订单列表
     */
    json getOrdersByStatus(long user_id, const std::string& status);

    /**
     * @brief 获取所有订单(管理员功能,带分页)
     * @param status 订单状态筛选(all表示全部)
     * @param page 页码(从1开始)
     * @param page_size 每页数量
     * @param start_date 开始日期(可选,格式:YYYY-MM-DD)
     * @param end_date 结束日期(可选,格式:YYYY-MM-DD)
     * @return JSON响应 包含订单列表和分页信息
     */
    json getAllOrders(const std::string& status, int page, int page_size, 
                     const std::string& start_date, const std::string& end_date);

    /**
     * @brief 创建用户通知
     * @param user_id 用户ID
     * @param type 通知类型(order_status, refund, promotion, system)
     * @param title 通知标题
     * @param content 通知内容
     * @param related_id 关联ID(如订单ID、退款ID)
     * @return JSON响应 创建结果
     */
    json createNotification(long user_id, const std::string& type, 
                           const std::string& title, const std::string& content, 
                           long related_id = 0);
    
    /**
     * @brief 获取用户通知列表
     * @param user_id 用户ID
     * @param unread_only 是否只获取未读通知
     * @return JSON响应 通知列表
     */
    json getNotifications(long user_id, bool unread_only);
    
    /**
     * @brief 标记通知为已读
     * @param notification_id 通知ID
     * @param user_id 用户ID
     * @return JSON响应 操作结果
     */
    json markNotificationRead(long notification_id, long user_id);
    
    /**
     * @brief 记录库存变动
     * @param product_id 商品ID
     * @param change_qty 变动数量(正数=增加,负数=减少)
     * @param reason 变动原因(order_created, order_canceled, refund, manual_adjust, etc.)
     * @param related_type 关联类型(order, refund, adjustment)
     * @param related_id 关联ID
     * @param operator_id 操作员ID(0表示系统操作)
     * @return 成功返回true
     */
    bool logStockChange(long product_id, int change_qty, const std::string& reason,
                       const std::string& related_type, long related_id, long operator_id);

    /**
     * @brief 订单物流跟踪
     * @param order_id 订单ID
     * @return JSON响应 包含物流跟踪信息
     */
    json trackOrder(long order_id);
};

#endif // ORDER_SERVICE_H
