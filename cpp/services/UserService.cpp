#include "UserService.h"
#include <sstream>

// ==================== 私有辅助方法 ====================

const std::string& UserService::getUserIdColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("users", "user_id")) return "user_id";
        if (hasColumn("users", "id")) return "id";
        return "user_id";
    }();
    return column;
}

const std::string& UserService::getUserCreatedAtColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("users", "created_at")) return "created_at";
        if (hasColumn("users", "create_time")) return "create_time";
        return std::string();
    }();
    return column;
}

const std::string& UserService::getUserUpdatedAtColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("users", "updated_at")) return "updated_at";
        if (hasColumn("users", "update_time")) return "update_time";
        return std::string();
    }();
    return column;
}

std::string UserService::qualifyUserColumn(const std::string& alias, const std::string& column_name) const {
    if (column_name.empty()) {
        return "";
    }
    return alias.empty() ? column_name : alias + "." + column_name;
}

std::string UserService::hashPassword(const std::string& password) const {
    std::hash<std::string> hasher;
    size_t hash_value = hasher(password + "emshop_salt_2025");
    return std::to_string(hash_value);
}

std::string UserService::generateToken(long user_id) {
    std::string token = StringUtils::generateRandomString(32) + "_" + std::to_string(user_id);
    
    std::lock_guard<std::mutex> lock(session_mutex_);
    active_sessions_[user_id] = token;
    
    return token;
}

json UserService::validateUserInput(const std::string& username, const std::string& password, 
                      const std::string& phone) const {
    if (username.empty() || username.length() < 3 || username.length() > 50) {
        return createErrorResponse("用户名长度必须在3-50个字符之间", Constants::VALIDATION_ERROR_CODE);
    }
    
    if (password.empty() || password.length() < 6 || password.length() > 100) {
        return createErrorResponse("密码长度必须在6-100个字符之间", Constants::VALIDATION_ERROR_CODE);
    }
    
    if (!phone.empty() && !StringUtils::isValidPhone(phone)) {
        return createErrorResponse("手机号格式不正确", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 检查用户名格式（字母、数字、下划线）
    std::regex username_pattern("^[a-zA-Z0-9_]{3,50}$");
    if (!std::regex_match(username, username_pattern)) {
        return createErrorResponse("用户名只能包含字母、数字和下划线", Constants::VALIDATION_ERROR_CODE);
    }
    
    return createSuccessResponse();
}

bool UserService::isUsernameExists(const std::string& username) const {
    std::string sql = "SELECT COUNT(*) as count FROM users WHERE username = '" 
                     + escapeSQLString(username) + "'";
    
    json result = const_cast<UserService*>(this)->executeQuery(sql);
    if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
        return result["data"][0]["count"].get<long>() > 0;
    }
    return false;
}

json UserService::fetchUsers(int page, int pageSize, const std::string& status, const std::string& keyword) {
    if (page < 1) {
        page = 1;
    }
    if (pageSize <= 0) {
        pageSize = Constants::DEFAULT_PAGE_SIZE;
    } else if (pageSize > Constants::MAX_PAGE_SIZE) {
        pageSize = Constants::MAX_PAGE_SIZE;
    }
    int offset = (page - 1) * pageSize;

    const std::string& id_column = getUserIdColumnName();
    const std::string& created_column = getUserCreatedAtColumnName();
    const std::string& updated_column = getUserUpdatedAtColumnName();

    std::vector<std::string> select_fields = {
        aliasColumn(qualifyUserColumn("", id_column), "user_id"),
        "username",
        "phone",
        "email",
        "role",
        "status"
    };
    select_fields.push_back(aliasColumn(qualifyUserColumn("", created_column), "created_at"));
    select_fields.push_back(aliasColumn(qualifyUserColumn("", updated_column), "updated_at"));

    std::string sql = "SELECT " + joinColumns(select_fields) + " FROM users WHERE 1=1";

    if (!status.empty() && StringUtils::toLower(status) != "all") {
        sql += " AND status = '" + escapeSQLString(status) + "'";
    }

    if (!keyword.empty()) {
        std::string escapedKeyword = escapeSQLString(keyword);
        sql += " AND (username LIKE '%" + escapedKeyword + "%'";
        sql += " OR CAST(" + id_column + " AS CHAR) = '" + escapedKeyword + "')";
    }

    std::string order_column = !created_column.empty() ? created_column : id_column;
    sql += " ORDER BY " + order_column + " DESC";
    sql += " LIMIT " + std::to_string(pageSize) + " OFFSET " + std::to_string(offset);

    return executeQuery(sql);
}

// ==================== 公共接口方法 ====================

UserService::UserService() : BaseService() {
    logInfo("用户服务初始化完成");
}

std::string UserService::getServiceName() const {
    return "UserService";
}

json UserService::getUserById(long user_id) const {
    const std::string& id_column = getUserIdColumnName();
    const std::string& created_column = getUserCreatedAtColumnName();
    const std::string& updated_column = getUserUpdatedAtColumnName();

    std::vector<std::string> select_fields = {
        aliasColumn(qualifyUserColumn("", id_column), "user_id"),
        "username",
        "phone",
        "email",
        "role",
        "status"
    };
    select_fields.push_back(aliasColumn(qualifyUserColumn("", created_column), "created_at"));
    select_fields.push_back(aliasColumn(qualifyUserColumn("", updated_column), "updated_at"));

    std::string sql = "SELECT " + joinColumns(select_fields) + " FROM users WHERE " +
                      id_column + " = " + std::to_string(user_id) + " AND status = 'active'";

    json result = const_cast<UserService*>(this)->executeQuery(sql);
    if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
        return result["data"][0];
    }
    return json::object();
}

json UserService::getAllUsers(int page, int pageSize, const std::string& status) {
    return fetchUsers(page, pageSize, status, "");
}

json UserService::searchUsers(const std::string& keyword, int page, int pageSize) {
    return fetchUsers(page, pageSize, "all", keyword);
}

json UserService::registerUser(const std::string& username, const std::string& password, const std::string& phone) {
    logInfo("用户注册请求: " + username);
    
    // 验证输入
    json validation = validateUserInput(username, password, phone);
    if (!validation["success"].get<bool>()) {
        return validation;
    }
    
    // 检查用户名唯一性
    if (isUsernameExists(username)) {
        return createErrorResponse("用户名已存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 插入新用户
        std::string hashed_password = hashPassword(password);
        std::string sql = "INSERT INTO users (username, password, phone) "
                         "VALUES ('" + escapeSQLString(username) + "', '" 
                         + escapeSQLString(hashed_password) + "', '"
                         + escapeSQLString(phone) + "')";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            long user_id = result["data"]["insert_id"].get<long>();
            
            json response_data;
            response_data["user_id"] = user_id;
            response_data["username"] = username;
            
            logInfo("用户注册成功，用户ID: " + std::to_string(user_id));
            return createSuccessResponse(response_data, "注册成功");
        } else {
            return result;
        }
        
    } catch (const std::exception& e) {
        std::string error_msg = "注册过程中发生异常: " + std::string(e.what());
        logError(error_msg);
        return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
    }
}

json UserService::loginUser(const std::string& username, const std::string& password) {
    logInfo("用户登录请求: " + username);
    
    if (username.empty() || password.empty()) {
        return createErrorResponse("用户名和密码不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        // 先尝试原有的hash方式
        std::string hashed_password = hashPassword(password);
        const std::string& id_column = getUserIdColumnName();
        std::string select_clause = aliasColumn(qualifyUserColumn("", id_column), "user_id") +
                                    ", username, phone, role";
        std::string sql = "SELECT " + select_clause + " FROM users WHERE username = '" +
                          escapeSQLString(username) + "' AND password = '" +
                          escapeSQLString(hashed_password) + "'";
        
        json result = executeQuery(sql);
        
        // 如果原有hash方式失败，尝试MD5方式（兼容数据库初始化数据）
        if (!result["success"].get<bool>() || result["data"].is_array() && result["data"].empty()) {
            sql = "SELECT " + select_clause + " FROM users WHERE username = '" +
                  escapeSQLString(username) + "' AND password = MD5('" +
                  escapeSQLString(password) + "')";
            result = executeQuery(sql);
        }
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        if (result["data"].is_array() && !result["data"].empty()) {
            json user_info = result["data"][0];
            long user_id = user_info["user_id"].get<long>();
            
            // 生成会话令牌
            std::string token = generateToken(user_id);
            
            json response_data;
            response_data["user_id"] = user_id;
            response_data["token"] = token;
            response_data["user_info"] = user_info;
            
            logInfo("用户登录成功，用户ID: " + std::to_string(user_id));
            return createSuccessResponse(response_data, "登录成功");
        } else {
            logWarn("登录失败：用户名或密码错误 - " + username);
            return createErrorResponse("用户名或密码错误", Constants::VALIDATION_ERROR_CODE);
        }
        
    } catch (const std::exception& e) {
        std::string error_msg = "登录过程中发生异常: " + std::string(e.what());
        logError(error_msg);
        return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
    }
}

json UserService::logoutUser(long user_id) {
    logInfo("用户登出请求，用户ID: " + std::to_string(user_id));
    
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        active_sessions_.erase(user_id);
    }
    
    return createSuccessResponse(json::object(), "登出成功");
}

json UserService::getUserInfo(long user_id) {
    logDebug("获取用户信息请求，用户ID: " + std::to_string(user_id));
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    json user_info = getUserById(user_id);
    if (user_info.empty()) {
        return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    return createSuccessResponse(user_info);
}

json UserService::updateUserInfo(long user_id, const json& update_info) {
    logInfo("更新用户信息请求，用户ID: " + std::to_string(user_id));
    
    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 检查用户是否存在
    json user_info = getUserById(user_id);
    if (user_info.empty()) {
        return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        std::vector<std::string> update_fields;
        const std::string& updated_column = getUserUpdatedAtColumnName();
        const std::string& id_column = getUserIdColumnName();
        
        // 构建更新字段
        if (update_info.contains("phone") && update_info["phone"].is_string()) {
            std::string phone = update_info["phone"].get<std::string>();
            if (!phone.empty() && !StringUtils::isValidPhone(phone)) {
                return createErrorResponse("手机号格式不正确", Constants::VALIDATION_ERROR_CODE);
            }
            update_fields.push_back("phone = '" + escapeSQLString(phone) + "'");
        }
        
        if (update_info.contains("email") && update_info["email"].is_string()) {
            std::string email = update_info["email"].get<std::string>();
            if (!email.empty() && !StringUtils::isValidEmail(email)) {
                return createErrorResponse("邮箱格式不正确", Constants::VALIDATION_ERROR_CODE);
            }
            update_fields.push_back("email = '" + escapeSQLString(email) + "'");
        }
        
        if (update_fields.empty()) {
            return createErrorResponse("没有需要更新的字段", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (!updated_column.empty()) {
            update_fields.push_back(updated_column + " = NOW()");
        }
        
        std::string sql = "UPDATE users SET ";
        for (size_t i = 0; i < update_fields.size(); ++i) {
            sql += update_fields[i];
            if (i < update_fields.size() - 1) {
                sql += ", ";
            }
        }
        sql += " WHERE " + id_column + " = " + std::to_string(user_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            logInfo("用户信息更新成功，用户ID: " + std::to_string(user_id));
            return createSuccessResponse(json::object(), "用户信息更新成功");
        } else {
            return result;
        }
        
    } catch (const std::exception& e) {
        std::string error_msg = "更新用户信息异常: " + std::string(e.what());
        logError(error_msg);
        return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
    }
}

json UserService::setUserStatus(long user_id, const std::string& status_or_action) {
    logInfo("设置用户状态，用户ID: " + std::to_string(user_id) + ", 状态: " + status_or_action);

    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }

    std::string normalized = StringUtils::toLower(status_or_action);
    if (normalized == "enable" || normalized == "enabled" || normalized == "active") {
        normalized = "active";
    } else if (normalized == "disable" || normalized == "disabled" || normalized == "inactive" || normalized == "suspended") {
        normalized = "inactive";
    } else if (normalized == "ban" || normalized == "banned") {
        normalized = "banned";
    } else {
        return createErrorResponse("无效的状态类型", Constants::VALIDATION_ERROR_CODE);
    }
    const std::string& id_column = getUserIdColumnName();
    const std::string& updated_column = getUserUpdatedAtColumnName();

    std::string sql = "UPDATE users SET status = '" + escapeSQLString(normalized) + "'";
    if (!updated_column.empty()) {
        sql += ", " + updated_column + " = NOW()";
    }
    sql += " WHERE " + id_column + " = " + std::to_string(user_id);

    json result = executeQuery(sql);
    if (result["success"].get<bool>()) {
        long affected_rows = 0;
        if (result.contains("data") && result["data"].is_object()) {
            if (result["data"].contains("affected_rows")) {
                affected_rows = result["data"]["affected_rows"].get<long>();
            } else if (result["data"].contains("rows_affected")) {
                affected_rows = result["data"]["rows_affected"].get<long>();
            }
        }

        if (affected_rows > 0) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["status"] = normalized;
            return createSuccessResponse(response_data, "用户状态更新成功");
        }
        return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
    }

    return result;
}

json UserService::getUserRoles(long user_id) {
    logInfo("获取用户角色，用户ID: " + std::to_string(user_id));

    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }

    try {
        const std::string& id_column = getUserIdColumnName();
        std::string sql = "SELECT role FROM users WHERE " + id_column + " = " + std::to_string(user_id) + " LIMIT 1";
        json query_result = executeQuery(sql);

        if (!query_result["success"].get<bool>() || query_result["data"].empty()) {
            return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
        }

        std::string role_value;
        const auto& row = query_result["data"][0];
        if (row.contains("role") && row["role"].is_string()) {
            role_value = StringUtils::toLower(row["role"].get<std::string>());
        }

        if (role_value.empty()) {
            role_value = "user";
        }

        json roles = json::array();
        roles.push_back(role_value);

        json permissions = json::array();
        if (role_value == "admin") {
            permissions.push_back("admin:*");
            permissions.push_back("user:manage");
            permissions.push_back("order:manage");
            permissions.push_back("coupon:manage");
            permissions.push_back("inventory:view");
        } else if (role_value == "vip") {
            permissions.push_back("user:basic");
            permissions.push_back("coupon:claim");
            permissions.push_back("vip:exclusive");
        } else {
            permissions.push_back("user:basic");
            permissions.push_back("coupon:claim");
        }

        json response_data;
        response_data["user_id"] = user_id;
        response_data["roles"] = roles;
        response_data["permissions"] = permissions;

        return createSuccessResponse(response_data, "获取用户角色成功");

    } catch (const std::exception& e) {
        return createErrorResponse("获取用户角色异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json UserService::setUserRole(long user_id, const std::string& role) {
    logInfo("设置用户角色，用户ID: " + std::to_string(user_id) + ", 角色: " + role);

    if (user_id <= 0) {
        return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
    }

    std::string normalized = StringUtils::toLower(role);
    static const std::unordered_set<std::string> allowed_roles = {"user", "admin", "vip"};
    if (allowed_roles.find(normalized) == allowed_roles.end()) {
        return createErrorResponse("无效的角色类型", Constants::VALIDATION_ERROR_CODE);
    }

    try {
        const std::string& id_column = getUserIdColumnName();
        const std::string& updated_column = getUserUpdatedAtColumnName();

        std::string sql = "UPDATE users SET role = '" + escapeSQLString(normalized) + "'";
        if (!updated_column.empty()) {
            sql += ", " + updated_column + " = NOW()";
        }
        sql += " WHERE " + id_column + " = " + std::to_string(user_id);

        json update_result = executeQuery(sql);
        if (!update_result["success"].get<bool>()) {
            return update_result;
        }

        long affected_rows = 0;
        if (update_result.contains("data") && update_result["data"].is_object()) {
            if (update_result["data"].contains("affected_rows")) {
                affected_rows = update_result["data"]["affected_rows"].get<long>();
            } else if (update_result["data"].contains("rows_affected")) {
                affected_rows = update_result["data"]["rows_affected"].get<long>();
            }
        }

        if (affected_rows > 0) {
            json response_data;
            response_data["user_id"] = user_id;
            response_data["role"] = normalized;
            return createSuccessResponse(response_data, "用户角色更新成功");
        }

        return createErrorResponse("用户不存在或角色未改变", Constants::VALIDATION_ERROR_CODE);

    } catch (const std::exception& e) {
        return createErrorResponse("设置用户角色异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

json UserService::checkUserPermission(long user_id, const std::string& permission) {
    json roles_result = getUserRoles(user_id);
    if (!roles_result["success"].get<bool>()) {
        return roles_result;
    }
    
    json permissions = roles_result["data"]["permissions"];
    bool has_permission = false;
    
    if (permissions.is_array()) {
        for (const auto& perm : permissions) {
            if (perm.get<std::string>() == permission) {
                has_permission = true;
                break;
            }
        }
    }
    
    json response_data;
    response_data["user_id"] = user_id;
    response_data["permission"] = permission;
    response_data["has_permission"] = has_permission;
    
    return createSuccessResponse(response_data);
}
