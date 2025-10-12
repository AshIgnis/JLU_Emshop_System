/**
 * @file CouponService.cpp
 * @brief 优惠券服务类实现
 * @date 2025-01-13
 */

#include "CouponService.h"

CouponService::CouponService() : BaseService() {
    logInfo("优惠券服务初始化完成");
}

std::string CouponService::getServiceName() const {
    return "CouponService";
}

json CouponService::getAvailableCoupons() {
    logInfo("获取可用优惠券列表");
    
    try {
        std::string sql = "SELECT coupon_id, name, code, type, value, min_amount, max_discount, "
                         "start_time, end_time, total_quantity, used_quantity, per_user_limit "
                         "FROM coupons WHERE status = 'active' AND start_time <= NOW() AND "
                         "end_time >= NOW() AND used_quantity < total_quantity "
                         "ORDER BY value DESC";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["coupons"] = result["data"];
            response_data["total_count"] = result["data"].size();
            
            return createSuccessResponse(response_data, "获取优惠券列表成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("获取优惠券列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json CouponService::getUserCoupons(long user_id) {
    logInfo("获取用户优惠券，用户ID: " + std::to_string(user_id));
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::string sql = "SELECT uc.id, uc.status as user_coupon_status, uc.received_at, uc.used_at, "
                         "c.coupon_id, c.name, c.code, c.type, c.value, c.min_amount, c.max_discount, "
                         "c.start_time, c.end_time FROM user_coupons uc "
                         "JOIN coupons c ON uc.coupon_id = c.coupon_id "
                         "WHERE uc.user_id = " + std::to_string(user_id) + 
                         " ORDER BY uc.received_at DESC";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["user_coupons"] = result["data"];
            response_data["total_count"] = result["data"].size();
            
            return createSuccessResponse(response_data, "获取用户优惠券成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("获取用户优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json CouponService::assignCouponToUser(long user_id, const std::string& coupon_identifier_raw) {
    logInfo("管理员分配优惠券，用户ID: " + std::to_string(user_id) + ", 标识: " + coupon_identifier_raw);

    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }

    std::string identifier = StringUtils::trim(coupon_identifier_raw);
    if (identifier.empty()) {
        return createErrorResponse("优惠券标识不能为空", Constants::VALIDATION_ERROR_CODE);
    }

    try {
        auto stripQuotes = [](std::string value) {
            value = StringUtils::trim(value);
            if (!value.empty() && (value.front() == '"' || value.front() == '\'')) {
                value.erase(value.begin());
            }
            if (!value.empty() && (value.back() == '"' || value.back() == '\'')) {
                value.pop_back();
            }
            return StringUtils::trim(value);
        };

        identifier = stripQuotes(identifier);

        std::string extracted_code;
        std::string name_candidate;
        auto open_paren = identifier.find_last_of('(');
        auto close_paren = identifier.find(')', open_paren == std::string::npos ? 0 : open_paren);
        if (open_paren != std::string::npos && close_paren != std::string::npos && close_paren > open_paren + 1) {
            extracted_code = StringUtils::trim(identifier.substr(open_paren + 1, close_paren - open_paren - 1));
            name_candidate = StringUtils::trim(identifier.substr(0, open_paren));
            extracted_code = stripQuotes(extracted_code);
            name_candidate = stripQuotes(name_candidate);
        }

        const bool identifier_is_numeric = !identifier.empty() &&
            std::all_of(identifier.begin(), identifier.end(), ::isdigit);
        const bool extracted_is_numeric = !extracted_code.empty() &&
            std::all_of(extracted_code.begin(), extracted_code.end(), ::isdigit);

        std::vector<std::string> lookup_conditions;
        auto add_condition = [&lookup_conditions](const std::string& condition) {
            if (condition.empty()) {
                return;
            }
            if (std::find(lookup_conditions.begin(), lookup_conditions.end(), condition) == lookup_conditions.end()) {
                lookup_conditions.push_back(condition);
            }
        };

        if (identifier_is_numeric) {
            add_condition("coupon_id = " + identifier);
        }

        if (!identifier.empty()) {
            add_condition("code = '" + escapeSQLString(identifier) + "'");
            add_condition("name = '" + escapeSQLString(identifier) + "'");
        }

        if (!extracted_code.empty()) {
            add_condition("code = '" + escapeSQLString(extracted_code) + "'");
            add_condition("name = '" + escapeSQLString(extracted_code) + "'");
        }

        if (extracted_is_numeric) {
            add_condition("coupon_id = " + extracted_code);
        }

        if (!name_candidate.empty()) {
            add_condition("name = '" + escapeSQLString(name_candidate) + "'");
        }

        if (lookup_conditions.empty()) {
            return createErrorResponse("优惠券不存在或已失效", Constants::VALIDATION_ERROR_CODE);
        }

        std::string lookup_sql = "SELECT coupon_id, code, name FROM coupons WHERE status = 'active' AND (";
        for (size_t i = 0; i < lookup_conditions.size(); ++i) {
            if (i > 0) {
                lookup_sql += " OR ";
            }
            lookup_sql += lookup_conditions[i];
        }
        lookup_sql += ") LIMIT 1";

        json lookup_result = executeQuery(lookup_sql);
        if (!lookup_result["success"].get<bool>() || lookup_result["data"].empty()) {
            return createErrorResponse("优惠券不存在或已失效", Constants::VALIDATION_ERROR_CODE);
        }

        const json coupon_info = lookup_result["data"][0];
        long coupon_id = 0;
        std::string coupon_code;
        std::string coupon_name;

        if (coupon_info.contains("coupon_id")) {
            if (coupon_info["coupon_id"].is_number_integer()) {
                coupon_id = coupon_info["coupon_id"].get<long>();
            } else if (coupon_info["coupon_id"].is_string()) {
                try {
                    coupon_id = std::stol(coupon_info["coupon_id"].get<std::string>());
                } catch (...) {
                    coupon_id = 0;
                }
            }
        }

        if (coupon_info.contains("code") && coupon_info["code"].is_string()) {
            coupon_code = coupon_info["code"].get<std::string>();
        }
        if (coupon_code.empty()) {
            coupon_code = identifier;
        }

        if (coupon_info.contains("name") && coupon_info["name"].is_string()) {
            coupon_name = coupon_info["name"].get<std::string>();
        }

        std::string existing_sql =
            "SELECT uc.id, uc.status, uc.received_at "
            "FROM user_coupons uc WHERE uc.user_id = " + std::to_string(user_id) +
            " AND uc.coupon_id = " + std::to_string(coupon_id) + " ORDER BY uc.received_at DESC LIMIT 1";

        json existing_result = executeQuery(existing_sql);
        if (existing_result["success"].get<bool>() && !existing_result["data"].empty()) {
            auto record = existing_result["data"][0];
            std::string status = "unused";
            if (record.contains("status") && record["status"].is_string()) {
                status = record["status"].get<std::string>();
            } else if (record.contains("user_coupon_status") && record["user_coupon_status"].is_string()) {
                status = record["user_coupon_status"].get<std::string>();
            }

            if (status != "used") {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["coupon_code"] = coupon_code;
                response_data["coupon_id"] = coupon_id;
                response_data["status"] = status;
                if (!coupon_name.empty()) {
                    response_data["coupon_name"] = coupon_name;
                }
                return createSuccessResponse(response_data, "用户已拥有该优惠券");
            }
        }

        json claim_result = claimCoupon(user_id, coupon_code);
        if (claim_result["success"].get<bool>()) {
            if (claim_result.contains("message")) {
                claim_result["message"] = "优惠券分配成功";
            }
            if (claim_result.contains("data") && claim_result["data"].is_object()) {
                claim_result["data"]["assignment_type"] = "admin";
                claim_result["data"]["coupon_id"] = coupon_id;
                if (!coupon_name.empty()) {
                    claim_result["data"]["coupon_name"] = coupon_name;
                }
            }
        }
        return claim_result;

    } catch (const std::exception& e) {
        return createErrorResponse("分配优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json CouponService::claimCoupon(long user_id, const std::string& coupon_code) {
    logInfo("领取优惠券，用户ID: " + std::to_string(user_id) + ", 优惠券代码: " + coupon_code);
    
    std::lock_guard<std::mutex> lock(coupon_mutex_);
    
    if (user_id <= 0 || coupon_code.empty()) {
        return createErrorResponse("用户ID和优惠券代码不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 检查优惠券是否存在且可领取
        std::string check_sql = "SELECT coupon_id, name, total_quantity, used_quantity, per_user_limit, "
                               "start_time, end_time FROM coupons WHERE code = '" + coupon_code + 
                               "' AND status = 'active'";
        json check_result = executeQuery(check_sql);
        
        if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
            return createErrorResponse("优惠券不存在或已失效", Constants::VALIDATION_ERROR_CODE);
        }
        
        auto coupon = check_result["data"][0];
        long coupon_id = coupon["coupon_id"].get<long>();
        int total_quantity = coupon["total_quantity"].get<int>();
        int used_quantity = coupon["used_quantity"].get<int>();
        int per_user_limit = coupon["per_user_limit"].get<int>();
        
        // 检查库存
        if (used_quantity >= total_quantity) {
            return createErrorResponse("优惠券已领完", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 检查用户领取限制
        std::string user_check_sql = "SELECT COUNT(*) as count FROM user_coupons WHERE user_id = " +
                                    std::to_string(user_id) + " AND coupon_id = " + std::to_string(coupon_id);
        json user_result = executeQuery(user_check_sql);
        
        if (user_result["success"].get<bool>() && !user_result["data"].empty()) {
            int user_count = user_result["data"][0]["count"].get<int>();
            if (user_count >= per_user_limit) {
                return createErrorResponse("已达到个人领取限制", Constants::VALIDATION_ERROR_CODE);
            }
        }
        
        // 领取优惠券
        std::string insert_sql = "INSERT INTO user_coupons (user_id, coupon_id, status) VALUES (" +
                                std::to_string(user_id) + ", " + std::to_string(coupon_id) + ", 'unused')";
        json insert_result = executeQuery(insert_sql);
        
        if (insert_result["success"].get<bool>()) {
            // 更新优惠券使用数量
            std::string update_sql = "UPDATE coupons SET used_quantity = used_quantity + 1 WHERE coupon_id = " +
                                    std::to_string(coupon_id);
            executeQuery(update_sql);
            
            json response_data;
            response_data["user_id"] = user_id;
            response_data["coupon_id"] = coupon_id;
            response_data["coupon_code"] = coupon_code;
            response_data["coupon_name"] = coupon["name"];
            
            logInfo("优惠券领取成功，用户ID: " + std::to_string(user_id));
            return createSuccessResponse(response_data, "优惠券领取成功");
        }
        
        return insert_result;
    } catch (const std::exception& e) {
        return createErrorResponse("领取优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json CouponService::useCoupon(long user_id, long order_id, const std::string& coupon_code) {
    logInfo("使用优惠券，用户ID: " + std::to_string(user_id) + ", 订单ID: " + std::to_string(order_id));
    
    std::lock_guard<std::mutex> lock(coupon_mutex_);
    
    try {
        // 检查用户是否拥有该优惠券
        std::string check_sql = "SELECT uc.id, uc.status, c.coupon_id, c.type, c.value, c.min_amount "
                               "FROM user_coupons uc JOIN coupons c ON uc.coupon_id = c.coupon_id "
                               "WHERE uc.user_id = " + std::to_string(user_id) + 
                               " AND c.code = '" + coupon_code + "' AND uc.status = 'unused'";
        
        json check_result = executeQuery(check_sql);
        if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
            return createErrorResponse("优惠券不可用", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 标记优惠券为已使用
        long user_coupon_id = check_result["data"][0]["id"].get<long>();
        std::string update_sql = "UPDATE user_coupons SET status = 'used', used_at = NOW(), "
                                "order_id = " + std::to_string(order_id) + 
                                " WHERE id = " + std::to_string(user_coupon_id);
        
        json result = executeQuery(update_sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["order_id"] = order_id;
            response_data["coupon_code"] = coupon_code;
            response_data["used"] = true;
            
            logInfo("优惠券使用成功");
            return createSuccessResponse(response_data, "优惠券使用成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("使用优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}
