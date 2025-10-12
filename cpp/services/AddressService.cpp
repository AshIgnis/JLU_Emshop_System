#include "AddressService.h"

// 构造函数实现
AddressService::AddressService() : BaseService() {
    logInfo("用户地址服务初始化完成");
}

// 获取服务名称
std::string AddressService::getServiceName() const {
    return "AddressService";
}

// 添加用户地址
json AddressService::addUserAddress(long user_id, const std::string& receiver_name, const std::string& receiver_phone,
                   const std::string& province, const std::string& city, const std::string& district,
                   const std::string& detail_address, const std::string& postal_code, bool is_default) {
    logInfo("添加用户地址，用户ID: " + std::to_string(user_id));
    
    std::lock_guard<std::mutex> lock(address_mutex_);
    
    if (user_id <= 0 || receiver_name.empty() || receiver_phone.empty()) {
        return createErrorResponse("用户ID、收货人姓名和电话不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 如果设置为默认地址，先取消其他默认地址
        if (is_default) {
            std::string update_sql = "UPDATE user_addresses SET is_default = 0 WHERE user_id = " + 
                                   std::to_string(user_id);
            executeQuery(update_sql);
        }
        
        // 插入新地址 - 使用安全的字符串转义
        std::string escaped_receiver_name = escapeSQLString(receiver_name);
        std::string escaped_receiver_phone = escapeSQLString(receiver_phone);
        std::string escaped_province = escapeSQLString(province);
        std::string escaped_city = escapeSQLString(city);
        std::string escaped_district = escapeSQLString(district);
        std::string escaped_detail_address = escapeSQLString(detail_address);
        std::string escaped_postal_code = escapeSQLString(postal_code);
        
        std::string sql = "INSERT INTO user_addresses (user_id, receiver_name, receiver_phone, "
                         "province, city, district, detail_address, postal_code, is_default) VALUES (" +
                         std::to_string(user_id) + ", '" + escaped_receiver_name + "', '" + 
                         escaped_receiver_phone + "', '" + escaped_province + "', '" + 
                         escaped_city + "', '" + escaped_district + "', '" + 
                         escaped_detail_address + "', '" + escaped_postal_code + "', " + 
                         (is_default ? "1" : "0") + ")";
        
        logDebug("执行SQL: " + sql);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["address_id"] = result["data"]["insert_id"].get<long>();
            response_data["user_id"] = user_id;
            response_data["is_default"] = is_default;
            
            logInfo("地址添加成功，地址ID: " + std::to_string(response_data["address_id"].get<long>()));
            return createSuccessResponse(response_data, "地址添加成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("添加地址异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 获取用户地址列表
json AddressService::getUserAddresses(long user_id) {
    logInfo("获取用户地址列表，用户ID: " + std::to_string(user_id));
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::string sql = "SELECT address_id, user_id, receiver_name, receiver_phone, "
                         "province, city, district, detail_address, postal_code, is_default, "
                         "created_at, updated_at FROM user_addresses WHERE user_id = " +
                         std::to_string(user_id) + " ORDER BY is_default DESC, created_at DESC";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["addresses"] = result["data"];
            response_data["total_count"] = result["data"].size();
            
            return createSuccessResponse(response_data, "获取地址列表成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("获取地址列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 更新用户地址
json AddressService::updateUserAddress(long address_id, const std::string& receiver_name, const std::string& receiver_phone,
                      const std::string& province, const std::string& city, const std::string& district,
                      const std::string& detail_address, const std::string& postal_code) {
    logInfo("更新用户地址，地址ID: " + std::to_string(address_id));
    
    std::lock_guard<std::mutex> lock(address_mutex_);
    
    if (address_id <= 0) {
        return createErrorResponse("无效的地址ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 获取地址的用户ID
        std::string check_sql = "SELECT user_id, is_default FROM user_addresses WHERE address_id = " + 
                               std::to_string(address_id);
        json check_result = executeQuery(check_sql);
        
        if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
            return createErrorResponse("地址不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        long user_id = check_result["data"][0]["user_id"].get<long>();
        bool is_default = check_result["data"][0]["is_default"].get<bool>();
        
        // 更新地址信息 - 使用安全的字符串转义
        std::string escaped_receiver_name = escapeSQLString(receiver_name);
        std::string escaped_receiver_phone = escapeSQLString(receiver_phone);
        std::string escaped_province = escapeSQLString(province);
        std::string escaped_city = escapeSQLString(city);
        std::string escaped_district = escapeSQLString(district);
        std::string escaped_detail_address = escapeSQLString(detail_address);
        std::string escaped_postal_code = escapeSQLString(postal_code);
        
        std::string sql = "UPDATE user_addresses SET receiver_name = '" + escaped_receiver_name + 
                         "', receiver_phone = '" + escaped_receiver_phone + "', province = '" + escaped_province +
                         "', city = '" + escaped_city + "', district = '" + escaped_district + 
                         "', detail_address = '" + escaped_detail_address + "', postal_code = '" + escaped_postal_code +
                         "', updated_at = NOW() WHERE address_id = " + std::to_string(address_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["address_id"] = address_id;
            response_data["updated"] = true;
            
            logInfo("地址更新成功，地址ID: " + std::to_string(address_id));
            return createSuccessResponse(response_data, "地址更新成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("更新地址异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 删除用户地址
json AddressService::deleteUserAddress(long address_id) {
    logInfo("删除用户地址，地址ID: " + std::to_string(address_id));
    
    std::lock_guard<std::mutex> lock(address_mutex_);
    
    if (address_id <= 0) {
        return createErrorResponse("无效的地址ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::string sql = "DELETE FROM user_addresses WHERE address_id = " + std::to_string(address_id);
        json result = executeQuery(sql);
        
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["address_id"] = address_id;
            response_data["deleted"] = true;
            
            logInfo("地址删除成功，地址ID: " + std::to_string(address_id));
            return createSuccessResponse(response_data, "地址删除成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("删除地址异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 设置默认地址
json AddressService::setDefaultAddress(long user_id, long address_id) {
    logInfo("设置默认地址，用户ID: " + std::to_string(user_id) + ", 地址ID: " + std::to_string(address_id));
    
    std::lock_guard<std::mutex> lock(address_mutex_);
    
    try {
        // 验证地址属于该用户
        std::string check_sql = "SELECT address_id FROM user_addresses WHERE address_id = " +
                               std::to_string(address_id) + " AND user_id = " + std::to_string(user_id);
        json check_result = executeQuery(check_sql);
        
        if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
            return createErrorResponse("地址不存在或不属于该用户", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 取消其他默认地址
        std::string update_sql1 = "UPDATE user_addresses SET is_default = 0 WHERE user_id = " + 
                                 std::to_string(user_id);
        executeQuery(update_sql1);
        
        // 设置新的默认地址
        std::string update_sql2 = "UPDATE user_addresses SET is_default = 1 WHERE address_id = " + 
                                 std::to_string(address_id);
        json result = executeQuery(update_sql2);
        
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["address_id"] = address_id;
            response_data["is_default"] = true;
            
            logInfo("默认地址设置成功");
            return createSuccessResponse(response_data, "默认地址设置成功");
        }
        
        return result;
    } catch (const std::exception& e) {
        return createErrorResponse("设置默认地址异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}
