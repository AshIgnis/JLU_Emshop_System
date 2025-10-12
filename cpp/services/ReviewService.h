/**
 * @file ReviewService.h
 * @brief 商品评论服务类定义
 * @date 2025-01-13
 */

#ifndef REVIEW_SERVICE_H
#define REVIEW_SERVICE_H

#include <string>
#include <mutex>

// 前向声明
class BaseService;
using json = nlohmann::json;

/**
 * @class ReviewService
 * @brief 商品评论服务类 - 管理商品评论的添加、查询和审核
 */
class ReviewService : public BaseService {
private:
    std::mutex review_mutex_; ///< 评论操作互斥锁

    /**
     * @brief 更新商品评分统计
     * @param product_id 商品ID
     * @note 计算该商品所有已审核评论的平均评分,更新products表
     */
    void updateProductRating(long product_id);

public:
    /**
     * @brief 构造函数
     */
    ReviewService();

    /**
     * @brief 获取服务名称
     * @return 服务名称字符串
     */
    std::string getServiceName() const override;

    /**
     * @brief 添加商品评论
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @param order_id 订单ID(可选,0表示无关联订单)
     * @param rating 评分(1-5星)
     * @param content 评论内容
     * @param is_anonymous 是否匿名评论
     * @return JSON响应 包含新评论ID和状态(pending待审核)
     * @note 评论需要管理员审核通过后才会显示,防止重复评论同一商品
     */
    json addProductReview(long user_id, long product_id, long order_id, int rating, 
                         const std::string& content, bool is_anonymous);

    /**
     * @brief 获取商品评论列表
     * @param product_id 商品ID
     * @param page 页码(从1开始)
     * @param page_size 每页数量
     * @param sort_by 排序方式("newest"最新,"rating_high"评分高到低,"rating_low"评分低到高)
     * @return JSON响应 包含已审核的评论列表及分页信息
     * @note 只返回已审核通过的评论,支持多种排序方式
     */
    json getProductReviews(long product_id, int page, int page_size, const std::string& sort_by);

    /**
     * @brief 获取用户评论列表
     * @param user_id 用户ID
     * @param page 页码(从1开始)
     * @param page_size 每页数量
     * @return JSON响应 包含用户的所有评论(含待审核/已拒绝)及分页信息
     * @note 用户可以看到自己的所有评论状态
     */
    json getUserReviews(long user_id, int page, int page_size);

    /**
     * @brief 审核评论(管理员功能)
     * @param review_id 评论ID
     * @param status 审核状态("approved"通过,"rejected"拒绝)
     * @param admin_note 管理员备注(可选)
     * @return JSON响应 审核结果
     * @note 审核通过后会自动更新商品评分统计
     */
    json reviewProductReview(long review_id, const std::string& status, const std::string& admin_note);
};

#endif // REVIEW_SERVICE_H
