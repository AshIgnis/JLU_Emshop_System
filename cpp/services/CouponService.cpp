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

// 获取订单可用优惠券列表
json CouponService::getAvailableCouponsForOrder(long user_id, double order_amount) {
    logInfo("获取订单可用优惠券，用户ID: " + std::to_string(user_id) + ", 订单金额: " + std::to_string(order_amount));
    
    if (user_id <= 0 || order_amount < 0) {
        return createErrorResponse("无效的参数", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 查询用户拥有的、未使用的、满足使用条件的优惠券
        std::string sql = "SELECT uc.id, c.coupon_id, c.name, c.code, c.type, c.value, c.min_amount, "
                         "c.max_discount, c.description, c.usage_instructions "
                         "FROM user_coupons uc "
                         "JOIN coupons c ON uc.coupon_id = c.coupon_id "
                         "WHERE uc.user_id = " + std::to_string(user_id) + 
                         " AND uc.status = 'unused' "
                         " AND c.status = 'active' "
                         " AND c.start_time <= NOW() "
                         " AND c.end_time >= NOW() "
                         " AND c.min_amount <= " + std::to_string(order_amount) +
                         " ORDER BY c.value DESC";
        
        json result = executeQuery(sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        // 计算每张优惠券的优惠金额
        json available_coupons = json::array();
        for (auto& coupon : result["data"]) {
            std::string type = coupon["type"].get<std::string>();
            double value = coupon["value"].get<double>();
            double max_discount = coupon.contains("max_discount") && !coupon["max_discount"].is_null() 
                                 ? coupon["max_discount"].get<double>() : 0;
            
            double discount_amount = 0;
            if (type == "discount") {
                // 折扣券: 优惠金额 = 原价 * (1 - 折扣)
                discount_amount = order_amount * (1.0 - value);
                if (max_discount > 0 && discount_amount > max_discount) {
                    discount_amount = max_discount;
                }
            } else if (type == "full_reduction") {
                // 满减券: 直接减免金额
                discount_amount = value;
            }
            
            double final_amount = order_amount - discount_amount;
            if (final_amount < 0) final_amount = 0;
            
            coupon["discount_amount"] = discount_amount;
            coupon["final_amount"] = final_amount;
            coupon["order_amount"] = order_amount;
            
            available_coupons.push_back(coupon);
        }
        
        json response_data;
        response_data["coupons"] = available_coupons;
        response_data["total_count"] = available_coupons.size();
        
        return createSuccessResponse(response_data, "获取订单可用优惠券成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取订单可用优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 计算优惠券折扣金额
json CouponService::calculateCouponDiscount(const std::string& coupon_code, double order_amount) {
    logInfo("计算优惠券折扣，优惠券代码: " + coupon_code + ", 订单金额: " + std::to_string(order_amount));
    
    if (coupon_code.empty() || order_amount < 0) {
        return createErrorResponse("无效的参数", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 查询优惠券信息
        std::string sql = "SELECT coupon_id, name, type, value, min_amount, max_discount, description "
                         "FROM coupons WHERE code = '" + escapeSQLString(coupon_code) + "' "
                         " AND status = 'active' AND start_time <= NOW() AND end_time >= NOW()";
        
        json result = executeQuery(sql);
        
        if (!result["success"].get<bool>() || result["data"].empty()) {
            return createErrorResponse("优惠券不存在或已失效", Constants::VALIDATION_ERROR_CODE);
        }
        
        auto coupon = result["data"][0];
        std::string type = coupon["type"].get<std::string>();
        double value = coupon["value"].get<double>();
        double min_amount = coupon["min_amount"].get<double>();
        double max_discount = coupon.contains("max_discount") && !coupon["max_discount"].is_null() 
                             ? coupon["max_discount"].get<double>() : 0;
        
        // 检查最低订单金额
        if (order_amount < min_amount) {
            return createErrorResponse("订单金额不满足优惠券使用条件(最低¥" + std::to_string(min_amount) + ")", 
                                     Constants::VALIDATION_ERROR_CODE);
        }
        
        // 计算优惠金额
        double discount_amount = 0;
        std::string discount_description;
        
        if (type == "discount") {
            // 折扣券
            discount_amount = order_amount * (1.0 - value);
            if (max_discount > 0 && discount_amount > max_discount) {
                discount_amount = max_discount;
                discount_description = std::to_string((int)(value * 10)) + "折(最高减¥" + std::to_string(max_discount) + ")";
            } else {
                discount_description = std::to_string((int)(value * 10)) + "折";
            }
        } else if (type == "full_reduction") {
            // 满减券
            discount_amount = value;
            discount_description = "满¥" + std::to_string(min_amount) + "减¥" + std::to_string(value);
        }
        
        double final_amount = order_amount - discount_amount;
        if (final_amount < 0) final_amount = 0;
        
        json response_data;
        response_data["coupon_code"] = coupon_code;
        response_data["coupon_name"] = coupon["name"];
        response_data["coupon_type"] = type;
        response_data["order_amount"] = order_amount;
        response_data["discount_amount"] = discount_amount;
        response_data["final_amount"] = final_amount;
        response_data["discount_description"] = discount_description;
        response_data["min_order_amount"] = min_amount;
        
        return createSuccessResponse(response_data, "计算优惠券折扣成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("计算优惠券折扣异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 创建优惠券活动(管理员功能)
json CouponService::createCouponActivity(const std::string& name, const std::string& coupon_code,
                                         const std::string& type, double discount_value, 
                                         double min_order_amount, int total_quantity,
                                         const std::string& start_date, const std::string& end_date,
                                         long template_id) {
    logInfo("创建优惠券活动，名称: " + name + ", 代码: " + coupon_code);
    
    std::lock_guard<std::mutex> lock(coupon_mutex_);
    
    if (name.empty() || coupon_code.empty() || type.empty()) {
        return createErrorResponse("优惠券名称、代码和类型不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 映射类型: discount -> percentage, full_reduction -> fixed_amount
    std::string db_type = type;
    if (type == "discount") {
        db_type = "percentage";
    } else if (type == "full_reduction") {
        db_type = "fixed_amount";
    } else if (type != "percentage" && type != "fixed_amount" && type != "free_shipping") {
        return createErrorResponse("优惠券类型只能是discount/percentage或full_reduction/fixed_amount或free_shipping", 
                                  Constants::VALIDATION_ERROR_CODE);
    }
    
    if (total_quantity <= 0) {
        return createErrorResponse("发行数量必须大于0", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 检查优惠券代码是否已存在
        std::string check_sql = "SELECT coupon_id FROM coupons WHERE code = '" + escapeSQLString(coupon_code) + "'";
        json check_result = executeQuery(check_sql);
        
        if (check_result["success"].get<bool>() && !check_result["data"].empty()) {
            return createErrorResponse("优惠券代码已存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 获取模板信息
        std::string description = "";
        
        if (template_id > 0) {
            std::string template_sql = "SELECT name, description FROM coupon_templates "
                                       "WHERE template_id = " + std::to_string(template_id);
            json template_result = executeQuery(template_sql);
            
            if (template_result["success"].get<bool>() && !template_result["data"].empty()) {
                if (template_result["data"][0].contains("description") && 
                    !template_result["data"][0]["description"].is_null()) {
                    description = template_result["data"][0]["description"].get<std::string>();
                }
            }
        }
        
        // 创建优惠券 (只使用数据库中存在的字段)
        std::string insert_sql = "INSERT INTO coupons (name, code, type, value, min_amount, total_quantity, "
                                "used_quantity, start_time, end_time, status, created_at";
        
        // 如果有template_id和description字段,添加它们
        if (template_id > 0) {
            insert_sql += ", template_id";
        }
        if (!description.empty()) {
            insert_sql += ", description";
        }
        
        insert_sql += ") VALUES ('" + escapeSQLString(name) + "', '" + escapeSQLString(coupon_code) + "', '" + 
                     escapeSQLString(db_type) + "', " + std::to_string(discount_value) + ", " + 
                     std::to_string(min_order_amount) + ", " + std::to_string(total_quantity) + ", 0, '" + 
                     escapeSQLString(start_date) + "', '" + escapeSQLString(end_date) + "', 'active', NOW()";
        
        if (template_id > 0) {
            insert_sql += ", " + std::to_string(template_id);
        }
        if (!description.empty()) {
            insert_sql += ", '" + escapeSQLString(description) + "'";
        }
        
        insert_sql += ")";
        
        json result = executeQuery(insert_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }

        if (!result.contains("data") || result["data"].is_null() ||
            !result["data"].contains("insert_id") || result["data"]["insert_id"].is_null()) {
            return createErrorResponse("创建优惠券活动失败: 未返回插入ID", Constants::DATABASE_ERROR_CODE);
        }

        long long coupon_id = 0;
        if (result["data"]["insert_id"].is_number_integer()) {
            coupon_id = result["data"]["insert_id"].get<long long>();
        } else if (result["data"]["insert_id"].is_string()) {
            coupon_id = std::stoll(result["data"]["insert_id"].get<std::string>());
        } else {
            return createErrorResponse("创建优惠券活动失败: 插入ID格式无效", Constants::DATABASE_ERROR_CODE);
        }

        json response_data;
        response_data["coupon_id"] = coupon_id;
        response_data["name"] = name;
        response_data["code"] = coupon_code;
        response_data["type"] = db_type;
        response_data["value"] = discount_value;
        response_data["min_amount"] = min_order_amount;
        response_data["total_quantity"] = total_quantity;
        
        logInfo("优惠券活动创建成功，优惠券ID: " + std::to_string(coupon_id));
        return createSuccessResponse(response_data, "优惠券活动创建成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("创建优惠券活动异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 获取优惠券模板列表
json CouponService::getCouponTemplates() {
    logInfo("获取优惠券模板列表");
    
    try {
        // 修正: 使用正确的列名 template_type 而不是 type
        std::string sql = "SELECT template_id, name, template_type, description, "
                         "icon_url, is_active, created_at "
                         "FROM coupon_templates WHERE is_active = TRUE "
                         "ORDER BY template_id";
        
        json result = executeQuery(sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        // 重新格式化数据,将template_type映射为前端期望的字段
        json templates = json::array();
        for (const auto& row : result["data"]) {
            json template_obj;
            // 确保template_id是整数类型
            if (row["template_id"].is_number()) {
                template_obj["template_id"] = row["template_id"].get<long long>();
            } else {
                template_obj["template_id"] = 0LL;
            }
            
            template_obj["template_name"] = row["name"].is_string() ? row["name"].get<std::string>() : "";
            
            // 映射 template_type 到前端期望的 discount_type
            std::string template_type = row["template_type"].is_string() ? row["template_type"].get<std::string>() : "";
            if (template_type == "full_reduction") {
                template_obj["discount_type"] = "fixed";  // 固定金额
                template_obj["discount_value"] = 20.0;     // 示例: 减20元
            } else if (template_type == "discount") {
                template_obj["discount_type"] = "percentage"; // 百分比
                template_obj["discount_value"] = 10.0;        // 示例: 10% off
            } else if (template_type == "free_shipping") {
                template_obj["discount_type"] = "fixed";
                template_obj["discount_value"] = 0.0;  // 包邮券
            } else {
                template_obj["discount_type"] = "fixed";
                template_obj["discount_value"] = 0.0;
            }
            
            template_obj["description"] = row.contains("description") && row["description"].is_string() ? 
                                          row["description"].get<std::string>() : "";
            
            // 确保is_active转换为布尔值 - 处理数据库返回的0/1
            bool is_active = true;
            if (row.contains("is_active") && !row["is_active"].is_null()) {
                if (row["is_active"].is_boolean()) {
                    is_active = row["is_active"].get<bool>();
                } else if (row["is_active"].is_number()) {
                    is_active = (row["is_active"].get<int>() != 0);
                }
            }
            template_obj["status"] = is_active ? "active" : "inactive";
            template_obj["valid_until"] = "";  // 模板没有有效期
            template_obj["min_order_amount"] = 0.0;  // 默认值
            template_obj["max_discount_amount"] = 0.0;  // 默认值
            
            templates.push_back(template_obj);
        }
        
        json response_data;
        response_data["templates"] = templates;
        response_data["total_count"] = templates.size();
        
        return createSuccessResponse(templates, "获取优惠券模板列表成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取优惠券模板列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 批量分配优惠券给用户
json CouponService::distributeCouponsToUsers(const std::string& coupon_code, const json& user_ids) {
    logInfo("批量分配优惠券，优惠券代码: " + coupon_code + ", 用户数: " + std::to_string(user_ids.size()));
    
    std::lock_guard<std::mutex> lock(coupon_mutex_);
    
    if (coupon_code.empty() || !user_ids.is_array() || user_ids.empty()) {
        return createErrorResponse("无效的参数", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 查询优惠券ID
        std::string query_sql = "SELECT coupon_id, total_quantity, used_quantity FROM coupons "
                               "WHERE code = '" + escapeSQLString(coupon_code) + "' AND status = 'active'";
        json query_result = executeQuery(query_sql);
        
        if (!query_result["success"].get<bool>() || query_result["data"].empty()) {
            return createErrorResponse("优惠券不存在或已失效", Constants::VALIDATION_ERROR_CODE);
        }
        
        long coupon_id = query_result["data"][0]["coupon_id"].get<long>();
        int total_quantity = query_result["data"][0]["total_quantity"].get<int>();
        int used_quantity = query_result["data"][0]["used_quantity"].get<int>();
        
        // 检查库存
        int available_quantity = total_quantity - used_quantity;
        if (available_quantity < static_cast<int>(user_ids.size())) {
            return createErrorResponse("优惠券库存不足", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 批量插入用户优惠券记录
        int success_count = 0;
        int failed_count = 0;
        
        for (const auto& user_id : user_ids) {
            if (!user_id.is_number_integer()) continue;
            
            long uid = user_id.get<long>();
            
            // 检查用户是否已拥有该优惠券
            std::string check_sql = "SELECT id FROM user_coupons WHERE user_id = " + std::to_string(uid) + 
                                   " AND coupon_id = " + std::to_string(coupon_id);
            json check_result = executeQuery(check_sql);
            
            if (check_result["success"].get<bool>() && !check_result["data"].empty()) {
                failed_count++;
                continue; // 用户已拥有,跳过
            }
            
            // 插入用户优惠券记录
            std::string insert_sql = "INSERT INTO user_coupons (user_id, coupon_id, status, received_at) "
                                    "VALUES (" + std::to_string(uid) + ", " + std::to_string(coupon_id) + ", 'unused', NOW())";
            json insert_result = executeQuery(insert_sql);
            
            if (insert_result["success"].get<bool>()) {
                success_count++;
            } else {
                failed_count++;
            }
        }
        
        // 更新优惠券已使用数量
        if (success_count > 0) {
            std::string update_sql = "UPDATE coupons SET used_quantity = used_quantity + " + std::to_string(success_count) + 
                                    " WHERE coupon_id = " + std::to_string(coupon_id);
            executeQuery(update_sql);
        }
        
        json response_data;
        response_data["coupon_code"] = coupon_code;
        response_data["total_users"] = user_ids.size();
        response_data["success_count"] = success_count;
        response_data["failed_count"] = failed_count;
        
        logInfo("批量分配优惠券完成，成功: " + std::to_string(success_count) + ", 失败: " + std::to_string(failed_count));
        return createSuccessResponse(response_data, "批量分配优惠券完成");
        
    } catch (const std::exception& e) {
        return createErrorResponse("批量分配优惠券异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}
