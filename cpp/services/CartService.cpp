#include "CartService.h"
#include <mysql.h>

// ====================================================================
// CartService 实现
// ====================================================================

CartService::CartService() : BaseService() {
    logInfo("购物车服务初始化完成");
}

std::string CartService::getServiceName() const {
    return "CartService";
}

// 检查购物车项是否存在
bool CartService::isCartItemExists(long user_id, long product_id) const {
    std::string sql = "SELECT COUNT(*) as count FROM cart WHERE user_id = " + 
                     std::to_string(user_id) + " AND product_id = " + std::to_string(product_id);
    
    json result = const_cast<CartService*>(this)->executeQuery(sql);
    if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
        return result["data"][0]["count"].get<long>() > 0;
    }
    return false;
}

// 添加商品到购物车
json CartService::addToCart(long user_id, long product_id, int quantity) {
    logInfo("添加商品到购物车，用户ID: " + std::to_string(user_id) + 
           ", 商品ID: " + std::to_string(product_id) + ", 数量: " + std::to_string(quantity));
    
    std::lock_guard<std::mutex> lock(cart_mutex_);
    
    if (user_id <= 0 || product_id <= 0) {
        return createErrorResponse("无效的用户ID或商品ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    if (quantity <= 0 || quantity > Constants::MAX_PRODUCT_QUANTITY) {
        return createErrorResponse("商品数量必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 检查商品是否存在且库存充足
        std::string product_sql = "SELECT stock_quantity as stock, name, status FROM products WHERE product_id = " + 
                                 std::to_string(product_id) + " FOR UPDATE";  // 加锁防止并发问题
        json product_result = executeQuery(product_sql);
        
        if (!product_result["success"].get<bool>()) {
            return product_result;
        }
        
        if (product_result["data"].is_array() && product_result["data"].empty()) {
            return createErrorResponse("商品不存在或已下架", Constants::VALIDATION_ERROR_CODE);
        }
        
        const auto& product = product_result["data"][0];
        std::string status = product.contains("status") && product["status"].is_string() ? 
                            product["status"].get<std::string>() : "inactive";
        
        if (status != "active") {
            return createErrorResponse("商品已下架，无法添加到购物车", Constants::VALIDATION_ERROR_CODE);
        }
        
        int available_stock = product["stock"].get<int>();
        std::string product_name = product.contains("name") && product["name"].is_string() ? 
                                  product["name"].get<std::string>() : "未知商品";
        
        // 明确提示库存为0的情况
        if (available_stock == 0) {
            return createErrorResponse("很抱歉，「" + product_name + "」已售罄，当前库存为 0", 
                                     Constants::VALIDATION_ERROR_CODE);
        }
        
        if (available_stock < quantity) {
            return createErrorResponse("「" + product_name + "」库存不足，您需要 " + std::to_string(quantity) + 
                                     " 件，但仅剩 " + std::to_string(available_stock) + " 件", 
                                     Constants::VALIDATION_ERROR_CODE);
        }        
        // ========== 检查商品限购规则 ==========
        // 获取购物车中该商品的现有数量
        int cart_quantity = 0;
        if (isCartItemExists(user_id, product_id)) {
            std::string cart_qty_sql = "SELECT quantity FROM cart WHERE user_id = " + 
                                       std::to_string(user_id) + " AND product_id = " + 
                                       std::to_string(product_id);
            json cart_qty_result = executeQuery(cart_qty_sql);
            if (cart_qty_result["success"].get<bool>() && !cart_qty_result["data"].empty()) {
                cart_quantity = cart_qty_result["data"][0]["quantity"].get<int>();
            }
        }
        
        // 总需求数量 = 购物车现有数量 + 本次添加数量
        int total_requested = cart_quantity + quantity;
        
        bool limit_violation = false;
        int limit_purchased_count = 0;
        int limit_limit_count = 0;
        std::string limit_period_value;
        {
            ConnectionGuard limit_conn(db_pool_);
            if (!limit_conn.isValid()) {
                logError("限购检查失败: 数据库连接不可用");
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            std::string limit_check_sql = "CALL check_user_purchase_limit(" +
                                         std::to_string(user_id) + ", " +
                                         std::to_string(product_id) + ", " +
                                         std::to_string(total_requested) + ", " +
                                         "@can_purchase, @purchased_count, @limit_count, @limit_period)";

            json limit_call_result = executeQueryWithConnection(limit_conn.get(), limit_check_sql);
            if (!limit_call_result["success"].get<bool>()) {
                logError("限购检查存储过程执行失败: " + limit_call_result["message"].get<std::string>());
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            while (mysql_more_results(limit_conn.get())) {
                mysql_next_result(limit_conn.get());
                MYSQL_RES* extra = mysql_store_result(limit_conn.get());
                if (extra) {
                    mysql_free_result(extra);
                }
            }

            std::string result_sql = "SELECT @can_purchase AS can_purchase, "
                                    "@purchased_count AS purchased_count, "
                                    "@limit_count AS limit_count, "
                                    "@limit_period AS limit_period";

            json limit_result = executeQueryWithConnection(limit_conn.get(), result_sql);
            if (!limit_result["success"].get<bool>() || limit_result["data"].empty()) {
                logError("限购检查结果读取失败: " + limit_result["message"].get<std::string>());
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            const auto& limit_data = limit_result["data"][0];
            bool can_purchase = true;
            if (limit_data.contains("can_purchase") && !limit_data["can_purchase"].is_null()) {
                can_purchase = limit_data["can_purchase"].get<int>() != 0;
            }

            if (limit_data.contains("purchased_count") && !limit_data["purchased_count"].is_null()) {
                limit_purchased_count = limit_data["purchased_count"].get<int>();
            }
            if (limit_data.contains("limit_count") && !limit_data["limit_count"].is_null()) {
                limit_limit_count = limit_data["limit_count"].get<int>();
            }
            if (limit_data.contains("limit_period") && !limit_data["limit_period"].is_null()) {
                limit_period_value = limit_data["limit_period"].get<std::string>();
            }

            logDebug("限购检查(addToCart): user=" + std::to_string(user_id) +
                     ", product=" + std::to_string(product_id) +
                     ", requested=" + std::to_string(total_requested) +
                     ", purchased=" + std::to_string(limit_purchased_count) +
                     ", limit=" + std::to_string(limit_limit_count) +
                     ", canPurchase=" + std::string(can_purchase ? "true" : "false"));

            if (!can_purchase) {
                limit_violation = true;
            }
        }

        if (limit_violation || (limit_limit_count > 0 && (total_requested + limit_purchased_count) > limit_limit_count)) {
            std::string period_text;
            if (limit_period_value == "daily") period_text = "今日";
            else if (limit_period_value == "weekly") period_text = "本周";
            else if (limit_period_value == "monthly") period_text = "本月";
            else period_text = "总计";

            return createErrorResponse(
                "「" + product_name + "」限购 " + std::to_string(limit_limit_count) + " 件/" + period_text +
                "，您" + period_text + "已购买 " + std::to_string(limit_purchased_count) + " 件" +
                (cart_quantity > 0 ? "，购物车中已有 " + std::to_string(cart_quantity) + " 件" : "") +
                "，本次添加 " + std::to_string(quantity) + " 件将超出限购",
                Constants::PURCHASE_LIMIT_EXCEEDED
            );
        }
        
        // 检查购物车中是否已有该商品
        if (isCartItemExists(user_id, product_id)) {
            // 更新数量
            std::string update_sql = "UPDATE cart SET quantity = quantity + " + 
                                    std::to_string(quantity) + ", updated_at = NOW() "
                                    "WHERE user_id = " + std::to_string(user_id) + 
                                    " AND product_id = " + std::to_string(product_id);
            
            json result = executeQuery(update_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["product_id"] = product_id;
                response_data["quantity_added"] = quantity;
                response_data["action"] = "updated";
                
                return createSuccessResponse(response_data, "购物车商品数量已更新");
            } else {
                return result;
            }
        } else {
            std::string insert_sql = "INSERT INTO cart (user_id, product_id, quantity, created_at, updated_at) "
                                    "VALUES (" + std::to_string(user_id) + ", " + std::to_string(product_id) + 
                                    ", " + std::to_string(quantity) + ", NOW(), NOW())";
            
            json result = executeQuery(insert_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["product_id"] = product_id;
                response_data["quantity"] = quantity;
                response_data["cart_item_id"] = result["data"]["insert_id"];
                response_data["action"] = "added";
                
                return createSuccessResponse(response_data, "商品已添加到购物车");
            } else {
                return result;
            }
        }
        
    } catch (const std::exception& e) {
        std::string error_msg = "添加商品到购物车异常: " + std::string(e.what());
        logError(error_msg);
        return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
    }
}

// 获取购物车内容
json CartService::getCart(long user_id) {
    logDebug("获取购物车内容，用户ID: " + std::to_string(user_id));
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::string sql = "SELECT c.cart_id as cart_item_id, c.product_id, c.quantity, c.selected, c.created_at, "
                         "p.name, p.price, p.stock_quantity as stock, p.category_id as category, "
                         "(c.quantity * p.price) as subtotal "
                         "FROM cart c "
                         "JOIN products p ON c.product_id = p.product_id "
                         "WHERE c.user_id = " + std::to_string(user_id) + 
                         " AND p.status = 'active' "
                         "ORDER BY c.created_at DESC";
        
        json result = executeQuery(sql);
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        json items = result["data"];
        double total_amount = 0.0;
        int total_items = 0;
        
        // 计算总金额和总数量
        for (const auto& item : items) {
            total_amount += item["subtotal"].get<double>();
            total_items += item["quantity"].get<int>();
        }
        
        json response_data;
        response_data["user_id"] = user_id;
        response_data["items"] = items;
        response_data["total_items"] = total_items;
        response_data["total_amount"] = total_amount;
        response_data["item_count"] = items.size();
        
        return createSuccessResponse(response_data);
        
    } catch (const std::exception& e) {
        std::string error_msg = "获取购物车内容异常: " + std::string(e.what());
        logError(error_msg);
        return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
    }
}

// 从购物车移除商品
json CartService::removeFromCart(long user_id, long product_id) {
    logInfo("从购物车移除商品，用户ID: " + std::to_string(user_id) + 
           ", 商品ID: " + std::to_string(product_id));
    
    std::lock_guard<std::mutex> lock(cart_mutex_);
    
    if (user_id <= 0 || product_id <= 0) {
        return createErrorResponse("无效的用户ID或商品ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    std::string sql = "DELETE FROM cart WHERE user_id = " + std::to_string(user_id) + 
                     " AND product_id = " + std::to_string(product_id);
    
    json result = executeQuery(sql);
    if (result["success"].get<bool>()) {
        long affected_rows = result["data"]["affected_rows"].get<long>();
        if (affected_rows > 0) {
            logInfo("商品已从购物车移除，用户ID: " + std::to_string(user_id) + 
                   ", 商品ID: " + std::to_string(product_id));
            return createSuccessResponse(json::object(), "商品已从购物车移除");
        } else {
            return createErrorResponse("购物车中没有该商品", Constants::VALIDATION_ERROR_CODE);
        }
    }
    
    return result;
}

// 更新购物车商品数量
json CartService::updateCartItemQuantity(long user_id, long product_id, int quantity) {
    logInfo("更新购物车商品数量，用户ID: " + std::to_string(user_id) + 
           ", 商品ID: " + std::to_string(product_id) + ", 新数量: " + std::to_string(quantity));
    
    std::lock_guard<std::mutex> lock(cart_mutex_);
    
    if (user_id <= 0 || product_id <= 0) {
        return createErrorResponse("无效的用户ID或商品ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    if (quantity <= 0) {
        return createErrorResponse("商品数量必须大于0", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 检查购物车中是否存在该商品
        if (!isCartItemExists(user_id, product_id)) {
            return createErrorResponse("购物车中没有该商品", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 检查商品库存和限购
        std::string product_sql = "SELECT stock_quantity as stock, name, status FROM products WHERE product_id = " + 
                                 std::to_string(product_id);
        json product_result = executeQuery(product_sql);
        
        if (!product_result["success"].get<bool>() || product_result["data"].empty()) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        const auto& product = product_result["data"][0];
        std::string status = product.contains("status") && product["status"].is_string() ? 
                            product["status"].get<std::string>() : "inactive";
        std::string product_name = product.contains("name") && product["name"].is_string() ? 
                                  product["name"].get<std::string>() : "未知商品";
        
        if (status != "active") {
            return createErrorResponse("商品已下架，无法更新数量", Constants::VALIDATION_ERROR_CODE);
        }
        
        int available_stock = product["stock"].get<int>();
        if (available_stock < quantity) {
            return createErrorResponse("「" + product_name + "」库存不足，您需要 " + std::to_string(quantity) + 
                                     " 件，但仅剩 " + std::to_string(available_stock) + " 件", 
                                     Constants::VALIDATION_ERROR_CODE);
        }
        
        // ========== 检查商品限购规则 ==========
        bool limit_violation = false;
        int limit_purchased_count = 0;
        int limit_limit_count = 0;
        std::string limit_period_value;
        {
            ConnectionGuard limit_conn(db_pool_);
            if (!limit_conn.isValid()) {
                logError("限购检查失败: 数据库连接不可用");
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            std::string limit_check_sql = "CALL check_user_purchase_limit(" +
                                         std::to_string(user_id) + ", " +
                                         std::to_string(product_id) + ", " +
                                         std::to_string(quantity) + ", " +
                                         "@can_purchase, @purchased_count, @limit_count, @limit_period)";

            json limit_call_result = executeQueryWithConnection(limit_conn.get(), limit_check_sql);
            if (!limit_call_result["success"].get<bool>()) {
                logError("限购检查存储过程执行失败: " + limit_call_result["message"].get<std::string>());
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            while (mysql_more_results(limit_conn.get())) {
                mysql_next_result(limit_conn.get());
                MYSQL_RES* extra = mysql_store_result(limit_conn.get());
                if (extra) {
                    mysql_free_result(extra);
                }
            }

            std::string result_sql = "SELECT @can_purchase AS can_purchase, "
                                    "@purchased_count AS purchased_count, "
                                    "@limit_count AS limit_count, "
                                    "@limit_period AS limit_period";

            json limit_result = executeQueryWithConnection(limit_conn.get(), result_sql);
            if (!limit_result["success"].get<bool>() || limit_result["data"].empty()) {
                logError("限购检查结果读取失败: " + limit_result["message"].get<std::string>());
                return createErrorResponse("限购检查失败，请稍后再试", Constants::DATABASE_ERROR_CODE);
            }

            const auto& limit_data = limit_result["data"][0];
            bool can_purchase = true;
            if (limit_data.contains("can_purchase") && !limit_data["can_purchase"].is_null()) {
                can_purchase = limit_data["can_purchase"].get<int>() != 0;
            }

            if (limit_data.contains("purchased_count") && !limit_data["purchased_count"].is_null()) {
                limit_purchased_count = limit_data["purchased_count"].get<int>();
            }
            if (limit_data.contains("limit_count") && !limit_data["limit_count"].is_null()) {
                limit_limit_count = limit_data["limit_count"].get<int>();
            }
            if (limit_data.contains("limit_period") && !limit_data["limit_period"].is_null()) {
                limit_period_value = limit_data["limit_period"].get<std::string>();
            }

            logDebug("限购检查(update): user=" + std::to_string(user_id) +
                     ", product=" + std::to_string(product_id) +
                     ", requested=" + std::to_string(quantity) +
                     ", purchased=" + std::to_string(limit_purchased_count) +
                     ", limit=" + std::to_string(limit_limit_count) +
                     ", canPurchase=" + std::string(can_purchase ? "true" : "false"));

            if (!can_purchase) {
                limit_violation = true;
            }
        }

        if (limit_violation || (limit_limit_count > 0 && (quantity + limit_purchased_count) > limit_limit_count)) {
            std::string period_text;
            if (limit_period_value == "daily") period_text = "今日";
            else if (limit_period_value == "weekly") period_text = "本周";
            else if (limit_period_value == "monthly") period_text = "本月";
            else period_text = "总计";

            return createErrorResponse(
                "「" + product_name + "」限购 " + std::to_string(limit_limit_count) + " 件/" + period_text +
                "，您" + period_text + "已购买 " + std::to_string(limit_purchased_count) + " 件" +
                "，更新为 " + std::to_string(quantity) + " 件将超出限购",
                Constants::PURCHASE_LIMIT_EXCEEDED
            );
        }
        
        // 更新商品数量
        std::string sql = "UPDATE cart SET quantity = " + std::to_string(quantity) + 
                         ", updated_at = NOW() WHERE user_id = " + std::to_string(user_id) + 
                         " AND product_id = " + std::to_string(product_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            long affected_rows = result["data"]["affected_rows"].get<long>();
            if (affected_rows > 0) {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["product_id"] = product_id;
                response_data["new_quantity"] = quantity;
                
                logInfo("购物车商品数量更新成功，用户ID: " + std::to_string(user_id) + 
                       ", 商品ID: " + std::to_string(product_id) + ", 新数量: " + std::to_string(quantity));
                return createSuccessResponse(response_data, "购物车商品数量更新成功");
            } else {
                return createErrorResponse("更新失败，购物车中没有该商品", Constants::VALIDATION_ERROR_CODE);
            }
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("更新购物车数量异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 更新购物车条目选中状态；product_id = -1 表示全选/全不选
json CartService::updateCartSelected(long user_id, long product_id, bool selected) {
    logInfo(std::string("更新购物车选中状态，用户ID: ") + std::to_string(user_id) +
            ", 商品ID: " + std::to_string(product_id) + ", 选中: " + (selected ? "true" : "false"));
    std::lock_guard<std::mutex> lock(cart_mutex_);
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    try {
        std::string sql;
        if (product_id <= 0) {
            sql = "UPDATE cart SET selected = " + std::string(selected ? "1" : "0") +
                  ", updated_at = NOW() WHERE user_id = " + std::to_string(user_id);
        } else {
            sql = "UPDATE cart SET selected = " + std::string(selected ? "1" : "0") +
                  ", updated_at = NOW() WHERE user_id = " + std::to_string(user_id) +
                  " AND product_id = " + std::to_string(product_id);
        }
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json data;
            data["user_id"] = user_id;
            if (product_id > 0) data["product_id"] = product_id;
            data["selected"] = selected;
            return createSuccessResponse(data, "购物车选中状态已更新");
        }
        return result;
    } catch (const std::exception &e) {
        return createErrorResponse("更新购物车选中状态异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 清空购物车
json CartService::clearCart(long user_id) {
    logInfo("清空购物车，用户ID: " + std::to_string(user_id));
    
    std::lock_guard<std::mutex> lock(cart_mutex_);
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    std::string sql = "DELETE FROM cart WHERE user_id = " + std::to_string(user_id);
    
    json result = executeQuery(sql);
    if (result["success"].get<bool>()) {
        long affected_rows = result["data"]["affected_rows"].get<long>();
        
        json response_data;
        response_data["user_id"] = user_id;
        response_data["removed_items"] = affected_rows;
        
        logInfo("购物车已清空，用户ID: " + std::to_string(user_id) + 
               ", 移除商品数: " + std::to_string(affected_rows));
        return createSuccessResponse(response_data, "购物车已清空");
    }
    
    return result;
}
