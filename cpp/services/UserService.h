#ifndef USERSERVICE_H
#define USERSERVICE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <regex>
#include <vector>
#include "../nlohmann_json.hpp"

using json = nlohmann::json;

// 前向声明 - BaseService将在主文件中定义
// 这个头文件需要在BaseService定义之后include
class BaseService;

/**
 * 用户服务类
 * 处理用户相关的所有业务逻辑
 * 包括注册、登录、用户信息管理等
 */
class UserService : public BaseService {
private:
    // 会话管理
    std::unordered_map<long, std::string> active_sessions_;
    std::mutex session_mutex_;

    // 列名辅助方法
    const std::string& getUserIdColumnName() const;
    const std::string& getUserCreatedAtColumnName() const;
    const std::string& getUserUpdatedAtColumnName() const;
    std::string qualifyUserColumn(const std::string& alias, const std::string& column_name) const;
    
    // 密码加密
    std::string hashPassword(const std::string& password) const;
    
    // 生成会话令牌
    std::string generateToken(long user_id);
    
    // 验证用户输入
    json validateUserInput(const std::string& username, const std::string& password, 
                          const std::string& phone = "") const;
    
    // 检查用户名是否已存在
    bool isUsernameExists(const std::string& username) const;
    
    // 通用获取用户列表方法
    json fetchUsers(int page, int pageSize, const std::string& status, const std::string& keyword);
    
public:
    UserService();
    
    std::string getServiceName() const override;
    
    // 核心用户功能（对应JNI接口）
    json registerUser(const std::string& username, const std::string& password, const std::string& phone);
    json loginUser(const std::string& username, const std::string& password);
    json logoutUser(long user_id);
    json getUserInfo(long user_id);
    json updateUserInfo(long user_id, const json& update_info);
    
    // 用户管理功能
    json setUserStatus(long user_id, const std::string& status_or_action);
    json getUserRoles(long user_id);
    json setUserRole(long user_id, const std::string& role);
    
    // 用户查询功能
    json getUserById(long user_id) const;
    json getAllUsers(int page, int pageSize, const std::string& status);
    json searchUsers(const std::string& keyword, int page, int pageSize);
    
    // 权限检查
    json checkUserPermission(long user_id, const std::string& permission);
};

#endif // USERSERVICE_H
