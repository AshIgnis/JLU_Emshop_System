/**
 * @file ReviewService.cpp
 * @brief 商品评论服务类实现
 * @date 2025-01-13
 */

#include "ReviewService.h"

ReviewService::ReviewService() : BaseService() {
    logInfo("商品评论服务初始化完成");
}

std::string ReviewService::getServiceName() const {
    return "ReviewService";
}

json ReviewService::addProductReview(long user_id, long product_id, long order_id, int rating, 
                     const std::string& content, bool is_anonymous) {
    logInfo("添加商品评论，用户ID: " + std::to_string(user_id) + ", 商品ID: " + std::to_string(product_id));
    
    std::lock_guard<std::mutex> lock(review_mutex_);
    
    if (user_id <= 0 || product_id <= 0 || rating < 1 || rating > 5) {
        return createErrorResponse("参数无效，评分必须在1-5之间", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 检查用户是否已评论过该商品
        std::string check_sql = "SELECT review_id FROM product_reviews WHERE user_id = " +
                               std::to_string(user_id) + " AND product_id = " + std::to_string(product_id);
        json check_result = executeQuery(check_sql);
        
        if (check_result["success"].get<bool>() && !check_result["data"].empty()) {
            return createErrorResponse("您已经评论过该商品", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 添加评论
        std::string sql = "INSERT INTO product_reviews (product_id, user_id, order_id, rating, "
                         "content, is_anonymous, status) VALUES (" +
                         std::to_string(product_id) + ", " + std::to_string(user_id) + ", " +
                         (order_id > 0 ? std::to_string(order_id) : "NULL") + ", " +
                         std::to_string(rating) + ", '" + content + "', " +
                         (is_anonymous ? "1" : "0") + ", 'pending')";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            long review_id = result["data"]["insert_id"].get<long>();
            
            // 更新商品评分统计
            updateProductRating(product_id);
            
            json response_data;
            response_data["review_id"] = review_id;
            response_data["product_id"] = product_id;
            response_data["user_id"] = user_id;
            response_data["rating"] = rating;
            response_data["status"] = "pending";
            
            logInfo("评论添加成功，评论ID: " + std::to_string(review_id));
            return createSuccessResponse(response_data, "评论提交成功，等待审核");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("添加评论异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json ReviewService::getProductReviews(long product_id, int page, int page_size, const std::string& sort_by) {
    logInfo("获取商品评论列表，商品ID: " + std::to_string(product_id));
    
    if (product_id <= 0) {
        return createErrorResponse("无效的商品ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        int offset = (page - 1) * page_size;
        std::string order_clause = "created_at DESC";
        
        if (sort_by == "rating_high") {
            order_clause = "rating DESC, created_at DESC";
        } else if (sort_by == "rating_low") {
            order_clause = "rating ASC, created_at DESC";
        }
        
        std::string sql = "SELECT r.review_id, r.user_id, r.rating, r.content, r.is_anonymous, "
                         "r.created_at, u.username FROM product_reviews r "
                         "LEFT JOIN users u ON r.user_id = u.user_id "
                         "WHERE r.product_id = " + std::to_string(product_id) + 
                         " AND r.status = 'approved' ORDER BY " + order_clause +
                         " LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            // 获取总数
            std::string count_sql = "SELECT COUNT(*) as total FROM product_reviews WHERE product_id = " +
                                   std::to_string(product_id) + " AND status = 'approved'";
            json count_result = executeQuery(count_sql);
            
            json response_data;
            response_data["product_id"] = product_id;
            response_data["reviews"] = result["data"];
            response_data["page"] = page;
            response_data["page_size"] = page_size;
            response_data["total_count"] = count_result["success"].get<bool>() ? 
                                          count_result["data"][0]["total"].get<int>() : 0;
            
            return createSuccessResponse(response_data, "获取评论列表成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("获取评论列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json ReviewService::getUserReviews(long user_id, int page, int page_size) {
    logInfo("获取用户评论列表，用户ID: " + std::to_string(user_id) + ", 页码: " + std::to_string(page));
    
    std::lock_guard<std::mutex> lock(review_mutex_);
    
    if (user_id <= 0 || page <= 0 || page_size <= 0) {
        return createErrorResponse("参数无效", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        int offset = (page - 1) * page_size;
        
        // 获取用户评论列表
        std::string sql = "SELECT pr.review_id, pr.product_id, pr.order_id, pr.rating, pr.content, "
                         "pr.is_anonymous, pr.status, pr.created_at, pr.updated_at, "
                         "p.name as product_name, p.main_image as product_image "
                         "FROM product_reviews pr "
                         "LEFT JOIN products p ON pr.product_id = p.product_id "
                         "WHERE pr.user_id = " + std::to_string(user_id) + " "
                         "ORDER BY pr.created_at DESC "
                         "LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);
        
        json result = executeQuery(sql);
        
        if (result["success"].get<bool>()) {
            // 获取总数
            std::string count_sql = "SELECT COUNT(*) as total FROM product_reviews WHERE user_id = " + 
                                   std::to_string(user_id);
            json count_result = executeQuery(count_sql);
            
            json response_data;
            response_data["user_id"] = user_id;
            response_data["reviews"] = result["data"];
            response_data["page"] = page;
            response_data["page_size"] = page_size;
            response_data["total_count"] = count_result["success"].get<bool>() ? 
                                          count_result["data"][0]["total"].get<int>() : 0;
            
            return createSuccessResponse(response_data, "获取用户评论列表成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("获取用户评论列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json ReviewService::reviewProductReview(long review_id, const std::string& status, const std::string& admin_note) {
    logInfo("审核评论，评论ID: " + std::to_string(review_id) + ", 状态: " + status);
    
    std::lock_guard<std::mutex> lock(review_mutex_);
    
    if (review_id <= 0 || (status != "approved" && status != "rejected")) {
        return createErrorResponse("参数无效", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::string sql = "UPDATE product_reviews SET status = '" + status + "', updated_at = NOW() "
                         "WHERE review_id = " + std::to_string(review_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            // 如果审核通过，更新商品评分
            if (status == "approved") {
                std::string product_sql = "SELECT product_id FROM product_reviews WHERE review_id = " +
                                         std::to_string(review_id);
                json product_result = executeQuery(product_sql);
                
                if (product_result["success"].get<bool>() && !product_result["data"].empty()) {
                    long product_id = product_result["data"][0]["product_id"].get<long>();
                    updateProductRating(product_id);
                }
            }
            
            json response_data;
            response_data["review_id"] = review_id;
            response_data["status"] = status;
            
            logInfo("评论审核完成，评论ID: " + std::to_string(review_id));
            return createSuccessResponse(response_data, "评论审核完成");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("审核评论异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

void ReviewService::updateProductRating(long product_id) {
    try {
        std::string sql = "UPDATE products SET "
                         "rating = (SELECT AVG(rating) FROM product_reviews WHERE product_id = " +
                         std::to_string(product_id) + " AND status = 'approved'), "
                         "review_count = (SELECT COUNT(*) FROM product_reviews WHERE product_id = " +
                         std::to_string(product_id) + " AND status = 'approved') "
                         "WHERE product_id = " + std::to_string(product_id);
        executeQuery(sql);
    } catch (const std::exception& e) {
        logError("更新商品评分失败: " + std::string(e.what()));
    }
}
