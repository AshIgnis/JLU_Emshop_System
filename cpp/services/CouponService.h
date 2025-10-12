/**
 * @file CouponService.h
 * @brief 优惠券服务类定义
 * @date 2025-01-13
 */

#ifndef COUPON_SERVICE_H
#define COUPON_SERVICE_H

#include <string>
#include <mutex>

// 前向声明
class BaseService;
using json = nlohmann::json;

/**
 * @class CouponService
 * @brief 优惠券服务类 - 管理优惠券的领取、分配和使用
 */
class CouponService : public BaseService {
private:
    std::mutex coupon_mutex_; ///< 优惠券操作互斥锁

public:
    /**
     * @brief 构造函数
     */
    CouponService();

    /**
     * @brief 获取服务名称
     * @return 服务名称字符串
     */
    std::string getServiceName() const override;

    /**
     * @brief 获取可用优惠券列表
     * @return JSON响应 包含所有活动且可领取的优惠券列表
     * @note 返回未过期且未领完的优惠券
     */
    json getAvailableCoupons();

    /**
     * @brief 获取用户优惠券
     * @param user_id 用户ID
     * @return JSON响应 包含用户已领取的所有优惠券及其状态
     */
    json getUserCoupons(long user_id);

    /**
     * @brief 管理员分配优惠券给用户
     * @param user_id 用户ID
     * @param coupon_identifier_raw 优惠券标识(可以是ID、代码或名称)
     * @return JSON响应 分配结果
     * @note 支持多种标识符格式: 纯数字ID、优惠券代码、"名称(代码)"格式等
     */
    json assignCouponToUser(long user_id, const std::string& coupon_identifier_raw);

    /**
     * @brief 用户领取优惠券
     * @param user_id 用户ID
     * @param coupon_code 优惠券代码
     * @return JSON响应 领取结果
     * @note 检查库存、个人限制,领取成功后更新used_quantity
     */
    json claimCoupon(long user_id, const std::string& coupon_code);

    /**
     * @brief 使用优惠券
     * @param user_id 用户ID
     * @param order_id 订单ID
     * @param coupon_code 优惠券代码
     * @return JSON响应 使用结果
     * @note 检查优惠券状态,标记为已使用并关联订单
     */
    json useCoupon(long user_id, long order_id, const std::string& coupon_code);
};

#endif // COUPON_SERVICE_H
