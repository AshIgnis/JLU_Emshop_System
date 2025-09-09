/*
 * JLU Emshop System - Object-Oriented JNI Implementation
 * 面向对象设计的JNI实现文件
 * 
 * 编译命令：g++ -std=c++17 -shared -fPIC -I$JAVA_HOME/include -I$JAVA_HOME/include/linux -L/usr/lib/mysql -lmysqlclient -o emshop_native.so emshop_native_impl_oop.cpp
 * Windows: g++ -std=c++17 -shared -I%JAVA_HOME%\include -I%JAVA_HOME%\include\win32 -L"C:\MySQL\lib" -lmysql -o emshop_native.dll emshop_native_impl_oop.cpp
 * 
 * 设计原则：
 * 1. 单一职责原则 - 每个类只负责特定功能
 * 2. 开放封闭原则 - 对扩展开放，对修改封闭
 * 3. 依赖倒置原则 - 依赖抽象而非具体实现
 * 
 */

// 头文件
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <random>
#include <iomanip>
#include <ctime>

// 特定配置
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #undef ERROR
#endif

// 第三方库头文件 
#include <mysql.h>
#include "nlohmann_json.hpp"
#include "emshop_EmshopNativeInterface.h"

// 命名空间和别名 
using json = nlohmann::json;
// 移除 chrono_literals，改用明确的时间单位声明

// 全局常量
namespace Constants {
    // 数据库配置
    const char* const DB_HOST = "127.0.0.1";
    const int DB_PORT = 3306;
    const char* const DB_NAME = "emshop";
    const char* const DB_USER = "root";
    const char* const DB_PASSWORD = "Quxc060122";
    
    // 连接池配置
    const int INITIAL_POOL_SIZE = 5;
    const int MAX_POOL_SIZE = 20;
    const int CONNECTION_TIMEOUT = 30;
    
    // 业务常量
    const int DEFAULT_PAGE_SIZE = 20;
    const int MAX_PAGE_SIZE = 100;
    const int MAX_PRODUCT_QUANTITY = 999;
    const double MIN_PRICE = 0.01;
    const double MAX_PRICE = 999999.99;
    
    // 状态码
    const int SUCCESS_CODE = 200; // 成功返回
    const int ERROR_CODE = 1001; // 失败返回
    const int VALIDATION_ERROR_CODE = 1002; // 参数校验失败
    const int DATABASE_ERROR_CODE = 1003; // 数据库操作失败
    const int PERMISSION_ERROR_CODE = 1004; // 权限不足
}

namespace EmshopConstants {
    using namespace Constants;
    const int ERROR_GENERAL = ERROR_CODE; // 通用错误
    const int ERROR_VALIDATION = VALIDATION_ERROR_CODE; // 参数校验错误
    const int ERROR_DATABASE = DATABASE_ERROR_CODE; // 数据库错误
    const int ERROR_NOT_FOUND_CODE = 1005; // 资源未找到
    const int ERROR_SYSTEM_BUSY = 1006; // 系统繁忙
    const double MIN_ORDER_AMOUNT = 0.01; // 最小订单金额
}

enum class OrderStatus {
    PENDING,
    PAID,
    SHIPPING,
    DELIVERED,
    CANCELLED,
    REFUNDED
};

enum class UserRole {
    USER,
    VIP,
    MANAGER,
    ADMIN
};

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR_LEVEL
};

// 工具类定义

class Logger {
private:
    static std::mutex log_mutex_;
    static LogLevel current_level_;
    static std::ofstream log_file_;
    
public:
    static void initialize(const std::string& log_file_path = "emshop.log") {
        std::lock_guard<std::mutex> lock(log_mutex_);
        if (!log_file_.is_open()) {
            log_file_.open(log_file_path, std::ios::app);
        }
    }
    
    static void setLevel(LogLevel level) {
        current_level_ = level;
    }
    
    static void log(LogLevel level, const std::string& message) {
        if (level < current_level_) return;
        
        std::lock_guard<std::mutex> lock(log_mutex_);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::string level_str;
        switch (level) {
            case LogLevel::DEBUG: level_str = "DEBUG"; break;
            case LogLevel::INFO: level_str = "INFO"; break;
            case LogLevel::WARN: level_str = "WARN"; break;
            case LogLevel::ERROR_LEVEL: level_str = "ERROR"; break;
        }
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        oss << " [" << level_str << "] " << message << std::endl;
        
        std::string log_entry = oss.str();
        std::cout << log_entry;
        if (log_file_.is_open()) {
            log_file_ << log_entry;
            log_file_.flush();
        }
    }
    
    static void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    static void info(const std::string& message) { log(LogLevel::INFO, message); }
    static void warn(const std::string& message) { log(LogLevel::WARN, message); }
    static void error(const std::string& message) { log(LogLevel::ERROR_LEVEL, message); }
};

// 静态成员初始化
std::mutex Logger::log_mutex_;
LogLevel Logger::current_level_ = LogLevel::INFO;
std::ofstream Logger::log_file_;

class StringUtils {
public:
    // 去除字符串两端空白字符
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
    
    // 转换为小写
    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // 转换为大写
    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    // 字符串分割
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = str.find(delimiter);
        
        while (end != std::string::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        result.push_back(str.substr(start));
        return result;
    }
    
    // 验证邮箱格式
    static bool isValidEmail(const std::string& email) {
        const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, pattern);
    }
    
    // 验证手机号格式
    static bool isValidPhone(const std::string& phone) {
        const std::regex pattern(R"(^1[3-9]\d{9}$)");
        return std::regex_match(phone, pattern);
    }
    
    // 生成随机字符串
    static std::string generateRandomString(size_t length) {
        const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, charset.size() - 1);
        
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i) {
            result += charset[distribution(generator)];
        }
        return result;
    }
};

// 数据库配置类
class DatabaseConfig {
private:
    std::string host_;
    int port_;
    std::string database_;
    std::string username_;
    std::string password_;
    std::string charset_;
    int connection_timeout_;
    int read_timeout_;
    int write_timeout_;
    bool auto_reconnect_;
    
    // 私有构造函数，防止外部实例化
    DatabaseConfig() 
        : host_(Constants::DB_HOST)
        , port_(Constants::DB_PORT)
        , database_(Constants::DB_NAME)
        , username_(Constants::DB_USER)
        , password_(Constants::DB_PASSWORD)
        , charset_("utf8mb4")
        , connection_timeout_(Constants::CONNECTION_TIMEOUT)
        , read_timeout_(30)
        , write_timeout_(30)
        , auto_reconnect_(true) {
        Logger::info("数据库配置初始化完成");
    }
    
public:
    // 获取单例实例
    static DatabaseConfig& getInstance() {
        static DatabaseConfig instance;
        return instance;
    }
    
    // 禁用拷贝构造和赋值操作
    DatabaseConfig(const DatabaseConfig&) = delete;
    DatabaseConfig& operator=(const DatabaseConfig&) = delete;
    
    // Getter 方法
    const std::string& getHost() const { return host_; }
    int getPort() const { return port_; }
    const std::string& getDatabase() const { return database_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getPassword() const { return password_; }
    const std::string& getCharset() const { return charset_; }
    int getConnectionTimeout() const { return connection_timeout_; }
    int getReadTimeout() const { return read_timeout_; }
    int getWriteTimeout() const { return write_timeout_; }
    bool getAutoReconnect() const { return auto_reconnect_; }
    
    // Setter 方法
    void setHost(const std::string& host) { 
        host_ = host; 
        Logger::info("数据库主机地址更新为: " + host);
    }
    
    void setPort(int port) { 
        port_ = port; 
        Logger::info("数据库端口更新为: " + std::to_string(port));
    }
    
    void setDatabase(const std::string& database) { 
        database_ = database; 
        Logger::info("数据库名称更新为: " + database);
    }
    
    void setUsername(const std::string& username) { 
        username_ = username; 
        Logger::info("数据库用户名更新为: " + username);
    }
    
    void setPassword(const std::string& password) { 
        password_ = password; 
        Logger::info("数据库密码已更新");
    }
    
    void setCharset(const std::string& charset) { 
        charset_ = charset; 
        Logger::info("数据库字符集更新为: " + charset);
    }
    
    void setConnectionTimeout(int timeout) { 
        connection_timeout_ = timeout; 
        Logger::info("连接超时时间更新为: " + std::to_string(timeout) + "秒");
    }
    
    void setAutoReconnect(bool reconnect) { 
        auto_reconnect_ = reconnect; 
        Logger::info("自动重连设置为: " + std::string(reconnect ? "启用" : "禁用"));
    }
    
    // 从配置文件加载配置
    bool loadFromFile(const std::string& config_file) {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            Logger::error("无法打开配置文件: " + config_file);
            return false;
        }
        
        try {
            json config;
            file >> config;
            
            if (config.contains("database")) {
                auto db_config = config["database"];
                
                if (db_config.contains("host")) setHost(db_config["host"]);
                if (db_config.contains("port")) setPort(db_config["port"]);
                if (db_config.contains("database")) setDatabase(db_config["database"]);
                if (db_config.contains("username")) setUsername(db_config["username"]);
                if (db_config.contains("password")) setPassword(db_config["password"]);
                if (db_config.contains("charset")) setCharset(db_config["charset"]);
                if (db_config.contains("connection_timeout")) setConnectionTimeout(db_config["connection_timeout"]);
                if (db_config.contains("auto_reconnect")) setAutoReconnect(db_config["auto_reconnect"]);
            }
            
            Logger::info("配置文件加载成功: " + config_file);
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("解析配置文件失败: " + std::string(e.what()));
            return false;
        }
    }
    
    // 保存配置到文件
    bool saveToFile(const std::string& config_file) const {
        std::ofstream file(config_file);
        if (!file.is_open()) {
            Logger::error("无法创建配置文件: " + config_file);
            return false;
        }
        
        try {
            json config;
            config["database"]["host"] = host_;
            config["database"]["port"] = port_;
            config["database"]["database"] = database_;
            config["database"]["username"] = username_;
            config["database"]["password"] = password_;
            config["database"]["charset"] = charset_;
            config["database"]["connection_timeout"] = connection_timeout_;
            config["database"]["auto_reconnect"] = auto_reconnect_;
            
            file << config.dump(4);
            Logger::info("配置文件保存成功: " + config_file);
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("保存配置文件失败: " + std::string(e.what()));
            return false;
        }
    }
    
    // 生成连接字符串用于调试
    std::string getConnectionString() const {
        std::ostringstream oss;
        oss << "mysql://" << username_ << "@" << host_ << ":" << port_ << "/" << database_;
        return oss.str();
    }
};

// 数据库连接池类 
class DatabaseConnection {
private:
    MYSQL* connection_;
    bool is_valid_;
    
public:
    explicit DatabaseConnection(MYSQL* conn) : connection_(conn), is_valid_(conn != nullptr) {
        if (connection_ && mysql_ping(connection_) != 0) {
            Logger::warn("数据库连接ping失败，连接可能已断开");
            is_valid_ = false;
        }
    }
    
    ~DatabaseConnection() {
        if (connection_) {
            // 连接会被返回到池中，这里不直接关闭
        }
    }
    
    MYSQL* get() const { return connection_; }
    bool isValid() const { return is_valid_ && connection_; }
    
    // 禁用拷贝，允许移动
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    DatabaseConnection(DatabaseConnection&& other) noexcept 
        : connection_(other.connection_), is_valid_(other.is_valid_) {
        other.connection_ = nullptr;
        other.is_valid_ = false;
    }
    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept {
        if (this != &other) {
            connection_ = other.connection_;
            is_valid_ = other.is_valid_;
            other.connection_ = nullptr;
            other.is_valid_ = false;
        }
        return *this;
    }
};


class DatabaseConnectionPool {
private:
    std::queue<MYSQL*> available_connections_;
    std::unordered_map<MYSQL*, std::chrono::steady_clock::time_point> connection_timestamps_;
    std::mutex pool_mutex_;
    std::condition_variable connection_available_;
    std::atomic<int> total_connections_;
    std::atomic<int> active_connections_;
    int max_pool_size_;
    bool initialized_;
    std::thread maintenance_thread_;
    std::atomic<bool> shutdown_flag_;
    
    DatabaseConnectionPool() 
        : total_connections_(0)
        , active_connections_(0)
        , max_pool_size_(Constants::MAX_POOL_SIZE)
        , initialized_(false)
        , shutdown_flag_(false) {
    }
    
    // 创建新的数据库连接
    MYSQL* createNewConnection() {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            Logger::error("MySQL初始化失败");
            return nullptr;
        }
        
        // 设置连接选项
        const DatabaseConfig& config = DatabaseConfig::getInstance();
        
        // 设置超时
        unsigned int timeout = config.getConnectionTimeout();
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
        
        // 设置自动重连
        bool reconnect = config.getAutoReconnect();
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        
        // 设置字符集
        mysql_options(conn, MYSQL_SET_CHARSET_NAME, config.getCharset().c_str());
        
        // 建立连接
        if (!mysql_real_connect(conn, 
                               config.getHost().c_str(),
                               config.getUsername().c_str(),
                               config.getPassword().c_str(),
                               config.getDatabase().c_str(),
                               config.getPort(),
                               nullptr, 
                               CLIENT_MULTI_RESULTS)) {
            Logger::error("数据库连接失败: " + std::string(mysql_error(conn)));
            mysql_close(conn);
            return nullptr;
        }
        
        // 设置时区和编码
        if (mysql_query(conn, "SET time_zone = '+08:00'") != 0 ||
            mysql_query(conn, "SET NAMES utf8mb4") != 0) {
            Logger::warn("设置数据库时区或编码失败");
        }
        
        Logger::info("创建新数据库连接: " + std::to_string(reinterpret_cast<uintptr_t>(conn)));
        return conn;
    }
    
    // 验证连接是否有效
    bool validateConnection(MYSQL* conn) {
        if (!conn) return false;
        
        // 使用ping检查连接状态
        if (mysql_ping(conn) != 0) {
            Logger::warn("连接ping失败: " + std::string(mysql_error(conn)));
            return false;
        }
        
        return true;
    }
    // 维护线程，定期清理过期连接
    void maintenanceLoop() {
        while (!shutdown_flag_) {
            std::this_thread::sleep_for(std::chrono::seconds(60));  // 每分钟执行一次维护
            std::unique_lock<std::mutex> lock(pool_mutex_);
            auto now = std::chrono::steady_clock::now();
            std::queue<MYSQL*> valid_connections;
            
            // 检查空闲连接，移除过期或无效的连接
            while (!available_connections_.empty()) {
                MYSQL* conn = available_connections_.front();
                available_connections_.pop();
                
                auto it = connection_timestamps_.find(conn);
                bool should_keep = true;
                
                if (it != connection_timestamps_.end()) {
                    auto age = std::chrono::duration_cast<std::chrono::minutes>(now - it->second);
                    if (age.count() > 10) {  // 连接超过10分钟未使用
                        should_keep = false;
                        Logger::info("移除过期连接: " + std::to_string(reinterpret_cast<uintptr_t>(conn)));
                    }
                }
                
                if (should_keep && validateConnection(conn)) {
                    valid_connections.push(conn);
                } else {
                    mysql_close(conn);
                    connection_timestamps_.erase(conn);
                    total_connections_--;
                }
            }
            
            available_connections_ = std::move(valid_connections);
            
            Logger::debug("连接池维护完成，可用连接: " + std::to_string(available_connections_.size()) +
                         "，总连接数: " + std::to_string(total_connections_.load()));
        }
    }
    
public:
    // 获取单例实例
    static DatabaseConnectionPool& getInstance() {
        static DatabaseConnectionPool instance;
        return instance;
    }
    
    // 禁用拷贝构造和赋值操作
    DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
    DatabaseConnectionPool& operator=(const DatabaseConnectionPool&) = delete;
    
    // 初始化连接池
    bool initialize() {
        if (initialized_) {
            Logger::warn("连接池已经初始化");
            return true;
        }
        
        Logger::info("初始化数据库连接池...");
        
        // 创建初始连接
        for (int i = 0; i < Constants::INITIAL_POOL_SIZE; ++i) {
            MYSQL* conn = createNewConnection();
            if (conn) {
                available_connections_.push(conn);
                connection_timestamps_[conn] = std::chrono::steady_clock::now();
                total_connections_++;
            } else {
                Logger::error("创建初始连接失败，索引: " + std::to_string(i));
            }
        }
        
        if (available_connections_.empty()) {
            Logger::error("连接池初始化失败：无法创建任何连接");
            return false;
        }
        
        // 启动维护线程
        maintenance_thread_ = std::thread(&DatabaseConnectionPool::maintenanceLoop, this);
        
        initialized_ = true;
        Logger::info("连接池初始化成功，创建了 " + std::to_string(available_connections_.size()) + " 个连接");
        return true;
    }
    
    // 获取数据库连接
    MYSQL* getConnection() {
        std::unique_lock<std::mutex> lock(pool_mutex_);
        
        // 等待可用连接，最多等待30秒
        if (!connection_available_.wait_for(lock, std::chrono::seconds(30), [this] { 
            return !available_connections_.empty() || shutdown_flag_; 
        })) {
            Logger::error("获取数据库连接超时");
            return nullptr;
        }
        
        if (shutdown_flag_) {
            return nullptr;
        }
        
        MYSQL* conn = nullptr;
        
        // 从可用连接中获取一个
        if (!available_connections_.empty()) {
            conn = available_connections_.front();
            available_connections_.pop();
            connection_timestamps_.erase(conn);
            active_connections_++;
            
            // 验证连接有效性
            if (!validateConnection(conn)) {
                mysql_close(conn);
                total_connections_--;
                active_connections_--;
                conn = nullptr;
            }
        }
        
        // 如果没有可用连接且未达到最大数量，创建新连接
        if (!conn && total_connections_ < max_pool_size_) {
            lock.unlock();  // 释放锁以避免阻塞其他操作
            conn = createNewConnection();
            lock.lock();
            
            if (conn) {
                total_connections_++;
                active_connections_++;
            }
        }
        
        if (conn) {
            Logger::debug("分配数据库连接: " + std::to_string(reinterpret_cast<uintptr_t>(conn)) +
                         "，活跃连接数: " + std::to_string(active_connections_.load()));
        } else {
            Logger::error("无法获取数据库连接");
        }
        
        return conn;
    }
    
    // 归还数据库连接
    void returnConnection(MYSQL* conn) {
        if (!conn) return;
        
        std::lock_guard<std::mutex> lock(pool_mutex_);
        
        // 验证连接仍然有效
        if (validateConnection(conn)) {
            available_connections_.push(conn);
            connection_timestamps_[conn] = std::chrono::steady_clock::now();
            active_connections_--;
            
            Logger::debug("归还数据库连接: " + std::to_string(reinterpret_cast<uintptr_t>(conn)) +
                         "，可用连接数: " + std::to_string(available_connections_.size()));
        } else {
            // 连接无效，直接关闭
            mysql_close(conn);
            total_connections_--;
            active_connections_--;
            Logger::warn("归还无效连接，已关闭: " + std::to_string(reinterpret_cast<uintptr_t>(conn)));
        }
        
        // 通知等待连接的线程
        connection_available_.notify_one();
    }
    
    // 获取连接池状态
    json getPoolStatus() const {
        json status;
        status["total_connections"] = total_connections_.load();
        status["active_connections"] = active_connections_.load();
        status["available_connections"] = available_connections_.size();
        status["max_pool_size"] = max_pool_size_;
        status["initialized"] = initialized_;
        return status;
    }
    
    // 关闭连接池
    void shutdown() {
        Logger::info("开始关闭数据库连接池...");
        shutdown_flag_ = true;
        
        // 停止维护线程
        if (maintenance_thread_.joinable()) {
            maintenance_thread_.join();
        }
        
        // 关闭所有连接
        std::lock_guard<std::mutex> lock(pool_mutex_);
        while (!available_connections_.empty()) {
            MYSQL* conn = available_connections_.front();
            available_connections_.pop();
            mysql_close(conn);
            Logger::debug("关闭数据库连接: " + std::to_string(reinterpret_cast<uintptr_t>(conn)));
        }
        
        connection_timestamps_.clear();
        total_connections_ = 0;
        active_connections_ = 0;
        initialized_ = false;
        
        Logger::info("数据库连接池已关闭");
    }
    
    ~DatabaseConnectionPool() {
        shutdown();
    }
};

// RAII 数据库连接管理器 - 修复内存安全问题
class ConnectionGuard {
private:
    MYSQL* connection_;
    DatabaseConnectionPool& pool_;
    bool connection_owned_;
    
public:
    explicit ConnectionGuard(DatabaseConnectionPool& pool) 
        : pool_(pool), connection_(nullptr), connection_owned_(false) {
        try {
            connection_ = pool_.getConnection();
            if (!connection_) {
                throw std::runtime_error("无法获取数据库连接");
            }
            connection_owned_ = true;
        } catch (const std::exception& e) {
            Logger::error("ConnectionGuard构造失败: " + std::string(e.what()));
            throw;
        }
    }
    
    ~ConnectionGuard() {
        if (connection_ && connection_owned_) {
            try {
                pool_.returnConnection(connection_);
            } catch (const std::exception& e) {
                Logger::error("ConnectionGuard析构失败: " + std::string(e.what()));
            }
            connection_ = nullptr;
            connection_owned_ = false;
        }
    }
    
    MYSQL* get() const { return connection_; }
    
    bool isValid() const { 
        if (!connection_) return false;
        try {
            return mysql_ping(connection_) == 0;
        } catch (const std::exception& e) {
            Logger::error("数据库连接检查失败: " + std::string(e.what()));
            return false;
        }
    }
    
    // 禁用拷贝，允许移动
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;
    ConnectionGuard(ConnectionGuard&& other) noexcept 
        : connection_(other.connection_), pool_(other.pool_), connection_owned_(other.connection_owned_) {
        other.connection_ = nullptr;
        other.connection_owned_ = false;
    }
    ConnectionGuard& operator=(ConnectionGuard&& other) noexcept {
        if (this != &other) {
            if (connection_ && connection_owned_) {
                try {
                    pool_.returnConnection(connection_);
                } catch (const std::exception& e) {
                    Logger::error("ConnectionGuard移动赋值失败: " + std::string(e.what()));
                }
            }
            connection_ = other.connection_;
            connection_owned_ = other.connection_owned_;
            other.connection_ = nullptr;
            other.connection_owned_ = false;
        }
        return *this;
    }
};

// 基础服务类 
class BaseService {
protected:
    DatabaseConnectionPool& db_pool_;
    
    // 构造函数设为保护，防止直接实例化
    BaseService() : db_pool_(DatabaseConnectionPool::getInstance()) {
        if (!db_pool_.getPoolStatus()["initialized"].get<bool>()) {
            throw std::runtime_error("数据库连接池未初始化");
        }
    }
    
public:
    virtual ~BaseService() = default;
    
    // 创建成功响应的模板方法
    json createSuccessResponse(const json& data = json::object(), 
                             const std::string& message = "操作成功") const {
        json response;
        response["success"] = true;
        response["message"] = message;
        response["data"] = data;
        response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return response;
    }
    
    // 创建错误响应的模板方法
    json createErrorResponse(const std::string& message, 
                           int error_code = Constants::ERROR_CODE) const {
        json response;
        response["success"] = false;
        response["message"] = message;
        response["error_code"] = error_code;
        response["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return response;
    }
    
    // 日志记录方法
    void logInfo(const std::string& message) const {
        Logger::info("[" + getServiceName() + "] " + message);
    }
    
    void logWarn(const std::string& message) const {
        Logger::warn("[" + getServiceName() + "] " + message);
    }
    
    void logError(const std::string& message) const {
        Logger::error("[" + getServiceName() + "] " + message);
    }
    
    void logDebug(const std::string& message) const {
        Logger::debug("[" + getServiceName() + "] " + message);
    }

    virtual std::string getServiceName() const = 0;
    
    // 执行查询的通用方法 - 修复内存安全问题
    json executeQuery(const std::string& sql, const json& params = json::object()) {
        MYSQL_RES* result = nullptr;
        try {
            ConnectionGuard conn(db_pool_);
            if (!conn.isValid()) {
                logError("数据库连接无效");
                return createErrorResponse("数据库连接无效", Constants::DATABASE_ERROR_CODE);
            }
            
            logDebug("执行SQL: " + sql);
            
            if (mysql_query(conn.get(), sql.c_str()) != 0) {
                std::string error_msg = "SQL执行失败: " + std::string(mysql_error(conn.get()));
                logError(error_msg);
                return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
            }
            
            // 如果是SELECT查询，获取结果
            result = mysql_store_result(conn.get());
            if (result) {
                json data = parseResultSet(result);
                mysql_free_result(result);
                result = nullptr; // 标记已释放
                return createSuccessResponse(data);
            } else if (mysql_field_count(conn.get()) == 0) {
                // 非SELECT查询（INSERT, UPDATE, DELETE等）
                json data;
                data["affected_rows"] = static_cast<int>(mysql_affected_rows(conn.get()));
                data["insert_id"] = static_cast<long>(mysql_insert_id(conn.get()));
                return createSuccessResponse(data);
            } else {
                std::string error_msg = "获取查询结果失败: " + std::string(mysql_error(conn.get()));
                logError(error_msg);
                return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
            }
            
        } catch (const std::exception& e) {
            // 确保在异常情况下也释放MySQL结果集
            if (result) {
                mysql_free_result(result);
                result = nullptr;
            }
            std::string error_msg = "查询执行异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        } catch (...) {
            // 捕获所有其他异常
            if (result) {
                mysql_free_result(result);
                result = nullptr;
            }
            std::string error_msg = "查询执行发生未知异常";
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 解析MySQL结果集为JSON - 修复内存安全问题
    json parseResultSet(MYSQL_RES* result) const {
        json rows = json::array();
        
        if (!result) {
            logWarn("MySQL结果集为空");
            return rows;
        }
        
        try {
            // 获取字段信息
            int num_fields = mysql_num_fields(result);
            MYSQL_FIELD* fields = mysql_fetch_fields(result);
            
            if (!fields || num_fields <= 0) {
                logWarn("MySQL字段信息无效");
                return rows;
            }
            
            // 逐行读取数据
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                json row_obj;
                unsigned long* lengths = mysql_fetch_lengths(result);
                
                if (!lengths) {
                    logWarn("无法获取MySQL行数据长度");
                    continue;
                }
                
                for (int i = 0; i < num_fields; i++) {
                    std::string field_name = fields[i].name ? fields[i].name : "unknown_field";
                    
                    if (row[i] == nullptr) {
                        row_obj[field_name] = nullptr;
                    } else {
                        std::string field_value(row[i], lengths[i]);
                        
                        // 根据字段类型进行适当的类型转换
                        switch (fields[i].type) {
                            case MYSQL_TYPE_TINY:
                            case MYSQL_TYPE_SHORT:
                            case MYSQL_TYPE_LONG:
                            case MYSQL_TYPE_LONGLONG:
                            case MYSQL_TYPE_INT24:
                                try {
                                    row_obj[field_name] = std::stoll(field_value);
                                } catch (const std::exception& e) {
                                    logWarn("整数类型转换失败，使用字符串: " + std::string(e.what()));
                                    row_obj[field_name] = field_value;
                                }
                                break;
                            
                            case MYSQL_TYPE_DECIMAL:
                            case MYSQL_TYPE_NEWDECIMAL:
                            case MYSQL_TYPE_FLOAT:
                            case MYSQL_TYPE_DOUBLE:
                                try {
                                    row_obj[field_name] = std::stod(field_value);
                                } catch (const std::exception& e) {
                                    logWarn("浮点类型转换失败，使用字符串: " + std::string(e.what()));
                                    row_obj[field_name] = field_value;
                                }
                                break;
                            
                            case MYSQL_TYPE_BIT:
                                row_obj[field_name] = (field_value == "1");
                                break;
                            
                            default:
                                row_obj[field_name] = field_value;
                                break;
                        }
                    }
                }
                rows.push_back(row_obj);
            }
            
        } catch (const std::exception& e) {
            logError("解析结果集异常: " + std::string(e.what()));
            // 返回已解析的数据，而不是完全失败
        } catch (...) {
            logError("解析结果集发生未知异常");
            // 返回已解析的数据，而不是完全失败
        }
        
        return rows;
    }
    
    // 转义SQL字符串
    std::string escapeSQLString(const std::string& input) const {
        try {
            ConnectionGuard conn(db_pool_);
            if (!conn.isValid()) {
                throw std::runtime_error("数据库连接无效");
            }
            
            std::string escaped;
            escaped.resize(input.length() * 2 + 1);
            
            unsigned long escaped_length = mysql_real_escape_string(
                conn.get(), &escaped[0], input.c_str(), input.length());
            
            escaped.resize(escaped_length);
            return escaped;
            
        } catch (const std::exception& e) {
            logError("SQL字符串转义失败: " + std::string(e.what()));
            std::string result = input;
            std::replace(result.begin(), result.end(), '\'', ' ');
            std::replace(result.begin(), result.end(), '\"', ' ');
            std::replace(result.begin(), result.end(), '\\', ' ');
            return result;
        }
    }
    
    // 验证分页参数
    std::pair<int, int> validatePaginationParams(int page, int page_size) const {
        if (page < 1) page = 1;
        if (page_size < 1) page_size = Constants::DEFAULT_PAGE_SIZE;
        if (page_size > Constants::MAX_PAGE_SIZE) page_size = Constants::MAX_PAGE_SIZE;
        return {page, page_size};
    }
    
    // 生成分页SQL
    std::string addPaginationToSQL(const std::string& sql, int page, int page_size) const {
        int offset = (page - 1) * page_size;
        return sql + " LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);
    }
};

// 用户服务类

/**
 * 用户服务类
 * 处理用户相关的所有业务逻辑
 * 包括注册、登录、用户信息管理等
 */
class UserService : public BaseService {
private:
    std::unordered_map<long, std::string> active_sessions_;
    std::mutex session_mutex_;
    
    // 密码加密
    std::string hashPassword(const std::string& password) const {
        std::hash<std::string> hasher;
        size_t hash_value = hasher(password + "emshop_salt_2025");
        return std::to_string(hash_value);
    }
    
    // 生成会话令牌
    std::string generateToken(long user_id) {
        std::string token = StringUtils::generateRandomString(32) + "_" + std::to_string(user_id);
        
        std::lock_guard<std::mutex> lock(session_mutex_);
        active_sessions_[user_id] = token;
        
        return token;
    }
    
    // 验证用户输入
    json validateUserInput(const std::string& username, const std::string& password, 
                          const std::string& phone = "") const {
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
    
    // 检查用户名是否已存在
    bool isUsernameExists(const std::string& username) const {
        std::string sql = "SELECT COUNT(*) as count FROM users WHERE username = '" 
                         + escapeSQLString(username) + "'";
        
        json result = const_cast<UserService*>(this)->executeQuery(sql);
        if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
            return result["data"][0]["count"].get<long>() > 0;
        }
        return false;
    }
    
    // 获取用户信息（内部方法）
    json getUserById(long user_id) const {
        std::string sql = "SELECT user_id as id, username, phone, email, role, created_at, updated_at "
                         "FROM users WHERE user_id = " + std::to_string(user_id) + " AND status = 'active'";
        
        json result = const_cast<UserService*>(this)->executeQuery(sql);
        if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
            return result["data"][0];
        }
        return json::object();
    }
    
public:
    UserService() : BaseService() {
        logInfo("用户服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "UserService";
    }
    
    // 用户注册
    json registerUser(const std::string& username, const std::string& password, const std::string& phone) {
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
    
    // 用户登录
    json loginUser(const std::string& username, const std::string& password) {
        logInfo("用户登录请求: " + username);
        
        if (username.empty() || password.empty()) {
            return createErrorResponse("用户名和密码不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            std::string hashed_password = hashPassword(password);
            std::string sql = "SELECT user_id, username, phone "
                             "FROM users WHERE username = '" + escapeSQLString(username) 
                             + "' AND password = '" + escapeSQLString(hashed_password) + "'";
            
            json result = executeQuery(sql);
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
    
    // 用户登出
    json logoutUser(long user_id) {
        logInfo("用户登出请求，用户ID: " + std::to_string(user_id));
        
        {
            std::lock_guard<std::mutex> lock(session_mutex_);
            active_sessions_.erase(user_id);
        }
        
        return createSuccessResponse(json::object(), "登出成功");
    }
    
    // 获取用户信息
    json getUserInfo(long user_id) {
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
    
    // 更新用户信息
    json updateUserInfo(long user_id, const json& update_info) {
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
            
            update_fields.push_back("updated_at = NOW()");
            
            std::string sql = "UPDATE users SET ";
            for (size_t i = 0; i < update_fields.size(); ++i) {
                sql += update_fields[i];
                if (i < update_fields.size() - 1) {
                    sql += ", ";
                }
            }
            sql += " WHERE user_id = " + std::to_string(user_id);
            
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
    
    // 验证会话令牌
    bool validateSession(long user_id, const std::string& token) {
        std::lock_guard<std::mutex> lock(session_mutex_);
        auto it = active_sessions_.find(user_id);
        return it != active_sessions_.end() && it->second == token;
    }
    
    // 获取用户角色
    json getUserRoles(long user_id) {
        logDebug("获取用户角色，用户ID: " + std::to_string(user_id));
        
        json user_info = getUserById(user_id);
        if (user_info.empty()) {
            return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        json response_data;
        response_data["user_id"] = user_id;
        response_data["role"] = user_info["role"];
        
        // 根据角色设置权限
        std::vector<std::string> permissions;
        std::string role = user_info["role"].get<std::string>();
        
        if (role == "admin") {
            permissions = {"manage_users", "manage_products", "manage_orders", "view_reports", "system_settings"};
        } else if (role == "manager") {
            permissions = {"manage_products", "manage_orders", "view_reports"};
        } else if (role == "vip") {
            permissions = {"place_orders", "view_history", "priority_support"};
        } else {
            permissions = {"place_orders", "view_history"};
        }
        
        response_data["permissions"] = permissions;
        
        return createSuccessResponse(response_data);
    }
    
    // 设置用户角色（管理员功能）
    json setUserRole(long user_id, const std::string& role) {
        logInfo("设置用户角色，用户ID: " + std::to_string(user_id) + ", 角色: " + role);
        
        // 验证角色
        std::vector<std::string> valid_roles = {"user", "vip", "manager", "admin"};
        if (std::find(valid_roles.begin(), valid_roles.end(), role) == valid_roles.end()) {
            return createErrorResponse("无效的角色类型", Constants::VALIDATION_ERROR_CODE);
        }
        
        std::string sql = "UPDATE users SET role = '" + escapeSQLString(role) 
                         + "', updated_at = NOW() WHERE user_id = " + std::to_string(user_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            long affected_rows = result["data"]["affected_rows"].get<long>();
            if (affected_rows > 0) {
                return createSuccessResponse(json::object(), "用户角色设置成功");
            } else {
                return createErrorResponse("用户不存在", Constants::VALIDATION_ERROR_CODE);
            }
        }
        
        return result;
    }
    
    // 检查用户权限
    json checkUserPermission(long user_id, const std::string& permission) {
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
};

// 商品服务类

class ProductService : public BaseService {
private:
    std::mutex stock_mutex_;  // 库存操作互斥锁
    
    // 验证商品输入
    json validateProductInput(const json& product_info) const {
        if (!product_info.contains("name") || !product_info["name"].is_string() ||
            product_info["name"].get<std::string>().empty()) {
            return createErrorResponse("商品名称不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (!product_info.contains("price") || !product_info["price"].is_number() ||
            product_info["price"].get<double>() < Constants::MIN_PRICE ||
            product_info["price"].get<double>() > Constants::MAX_PRICE) {
            return createErrorResponse("商品价格必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (!product_info.contains("stock") || !product_info["stock"].is_number_integer() ||
            product_info["stock"].get<int>() < 0 ||
            product_info["stock"].get<int>() > Constants::MAX_PRODUCT_QUANTITY) {
            return createErrorResponse("库存数量必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (!product_info.contains("category") || !product_info["category"].is_string() ||
            product_info["category"].get<std::string>().empty()) {
            return createErrorResponse("商品分类不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        return createSuccessResponse();
    }
    
    // 检查商品是否存在
    bool isProductExists(long product_id) const {
        std::string sql = "SELECT COUNT(*) as count FROM products WHERE product_id = " + 
                         std::to_string(product_id) + " AND status != 'deleted'";
        
        json result = const_cast<ProductService*>(this)->executeQuery(sql);
        if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
            return result["data"][0]["count"].get<long>() > 0;
        }
        return false;
    }
    
    // 获取商品详细信息（内部方法）
    json getProductById(long product_id) const {
        std::string sql = "SELECT product_id as id, name, description, price, stock_quantity as stock, "
                         "category_id as category, status, created_at, updated_at FROM products WHERE product_id = " + 
                         std::to_string(product_id) + " AND status != 'deleted'";
        
        json result = const_cast<ProductService*>(this)->executeQuery(sql);
        if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
            return result["data"][0];
        }
        return json::object();
    }
    
public:
    ProductService() : BaseService() {
        logInfo("商品服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "ProductService";
    }
    
    // 添加商品
    json addProduct(const json& product_info) {
        logInfo("添加商品请求: " + product_info.dump());
        
        // 验证输入
        json validation = validateProductInput(product_info);
        if (!validation["success"].get<bool>()) {
            return validation;
        }
        
        try {
            std::string name = escapeSQLString(product_info["name"].get<std::string>());
            std::string description = product_info.contains("description") ? 
                escapeSQLString(product_info["description"].get<std::string>()) : "";
            double price = product_info["price"].get<double>();
            int stock = product_info["stock"].get<int>();
            std::string category = escapeSQLString(product_info["category"].get<std::string>());
            
            std::string sql = "INSERT INTO products (name, description, price, stock, category, "
                             "status, created_at, updated_at) VALUES ('" + name + "', '" + 
                             description + "', " + std::to_string(price) + ", " + 
                             std::to_string(stock) + ", '" + category + "', 'active', NOW(), NOW())";
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                long product_id = result["data"]["insert_id"].get<long>();
                
                json response_data;
                response_data["product_id"] = product_id;
                response_data["name"] = product_info["name"];
                response_data["price"] = price;
                response_data["stock"] = stock;
                response_data["category"] = product_info["category"];
                
                logInfo("商品添加成功，商品ID: " + std::to_string(product_id));
                return createSuccessResponse(response_data, "商品添加成功");
            } else {
                return result;
            }
            
        } catch (const std::exception& e) {
            std::string error_msg = "添加商品异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 更新商品信息
    json updateProduct(long product_id, const json& update_info) {
        logInfo("更新商品请求，商品ID: " + std::to_string(product_id));
        
        if (!isProductExists(product_id)) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            std::vector<std::string> update_fields;
            
            if (update_info.contains("name") && update_info["name"].is_string()) {
                std::string name = update_info["name"].get<std::string>();
                if (name.empty()) {
                    return createErrorResponse("商品名称不能为空", Constants::VALIDATION_ERROR_CODE);
                }
                update_fields.push_back("name = '" + escapeSQLString(name) + "'");
            }
            
            if (update_info.contains("description") && update_info["description"].is_string()) {
                std::string description = update_info["description"].get<std::string>();
                update_fields.push_back("description = '" + escapeSQLString(description) + "'");
            }
            
            if (update_info.contains("price") && update_info["price"].is_number()) {
                double price = update_info["price"].get<double>();
                if (price < Constants::MIN_PRICE || price > Constants::MAX_PRICE) {
                    return createErrorResponse("商品价格必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
                }
                update_fields.push_back("price = " + std::to_string(price));
            }
            
            if (update_info.contains("category") && update_info["category"].is_string()) {
                std::string category = update_info["category"].get<std::string>();
                if (category.empty()) {
                    return createErrorResponse("商品分类不能为空", Constants::VALIDATION_ERROR_CODE);
                }
                update_fields.push_back("category = '" + escapeSQLString(category) + "'");
            }
            
            if (update_fields.empty()) {
                return createErrorResponse("没有需要更新的字段", Constants::VALIDATION_ERROR_CODE);
            }
            
            update_fields.push_back("updated_at = NOW()");
            
            std::string sql = "UPDATE products SET ";
            for (size_t i = 0; i < update_fields.size(); ++i) {
                sql += update_fields[i];
                if (i < update_fields.size() - 1) {
                    sql += ", ";
                }
            }
            sql += " WHERE id = " + std::to_string(product_id);
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                logInfo("商品信息更新成功，商品ID: " + std::to_string(product_id));
                return createSuccessResponse(json::object(), "商品信息更新成功");
            } else {
                return result;
            }
            
        } catch (const std::exception& e) {
            std::string error_msg = "更新商品信息异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 删除商品
    json deleteProduct(long product_id) {
        logInfo("删除商品请求，商品ID: " + std::to_string(product_id));
        
        if (!isProductExists(product_id)) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        std::string sql = "UPDATE products SET status = 'deleted', updated_at = NOW() "
                         "WHERE id = " + std::to_string(product_id);
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            logInfo("商品删除成功，商品ID: " + std::to_string(product_id));
            return createSuccessResponse(json::object(), "商品删除成功");
        }
        
        return result;
    }
    
    // 获取商品详情
    json getProductDetail(long product_id) {
        logDebug("获取商品详情，商品ID: " + std::to_string(product_id));
        
        json product_info = getProductById(product_id);
        if (product_info.empty()) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        return createSuccessResponse(product_info);
    }
    
    // 获取商品列表
    json getProductList(const std::string& category, int page, int page_size) {
        logDebug("获取商品列表，分类: " + category + ", 页码: " + std::to_string(page) + 
                ", 页大小: " + std::to_string(page_size));
        
        std::pair<int, int> validation_result = validatePaginationParams(page, page_size);
        int validated_page = validation_result.first;
        int validated_page_size = validation_result.second;
        
        try {
            std::string where_clause = "WHERE status = 'active'";
            
            if (category != "all" && !category.empty()) {
                // 如果category是数字，直接用作category_id
                if (std::all_of(category.begin(), category.end(), ::isdigit)) {
                    where_clause += " AND category_id = " + category;
                } else {
                    // 如果是分类名称，先查找分类ID
                    std::string category_sql = "SELECT category_id FROM categories WHERE name = '" + 
                                             escapeSQLString(category) + "' AND status = 'active'";
                    json category_result = executeQuery(category_sql);
                    
                    if (category_result["success"].get<bool>() && 
                        !category_result["data"].empty()) {
                        long category_id = category_result["data"][0]["category_id"].get<long>();
                        where_clause += " AND category_id = " + std::to_string(category_id);
                    } else {
                        // 分类不存在，返回空结果
                        json empty_response = createSuccessResponse("操作成功");
                        json data;
                        data["products"] = json::array();
                        data["total"] = 0;
                        data["total_pages"] = 0;
                        data["page"] = validated_page;
                        data["page_size"] = validated_page_size;
                        empty_response["data"] = data;
                        return empty_response;
                    }
                }
            }
            
            // 获取总数
            std::string count_sql = "SELECT COUNT(*) as total FROM products " + where_clause;
            json count_result = executeQuery(count_sql);
            
            if (!count_result["success"].get<bool>()) {
                return count_result;
            }
            
            long total = count_result["data"][0]["total"].get<long>();
            
            // 获取商品列表
            std::string sql = "SELECT product_id as id, name, description, price, stock_quantity as stock, "
                             "category_id as category, created_at, updated_at "
                             "FROM products " + where_clause + " ORDER BY created_at DESC";
            sql = addPaginationToSQL(sql, validated_page, validated_page_size);
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["products"] = result["data"];
                response_data["total"] = total;
                response_data["page"] = validated_page;
                response_data["page_size"] = validated_page_size;
                response_data["total_pages"] = (total + validated_page_size - 1) / validated_page_size;
                
                return createSuccessResponse(response_data);
            } else {
                return result;
            }
            
        } catch (const std::exception& e) {
            std::string error_msg = "获取商品列表异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 搜索商品
    json searchProducts(const std::string& keyword, int page, int page_size, 
                       const std::string& sort_by, double min_price, double max_price) {
        logDebug("搜索商品，关键词: " + keyword);
        
        std::pair<int, int> validation_result = validatePaginationParams(page, page_size);
        int validated_page = validation_result.first;
        int validated_page_size = validation_result.second;
        
        try {
            std::string where_clause = "WHERE p.status = 'active'";
            
            // 关键词搜索
            if (!keyword.empty()) {
                std::string escaped_keyword = escapeSQLString(keyword);
                where_clause += " AND (p.name LIKE '%" + escaped_keyword + "%' OR "
                               "p.description LIKE '%" + escaped_keyword + "%' OR "
                               "p.short_description LIKE '%" + escaped_keyword + "%' OR "
                               "p.brand LIKE '%" + escaped_keyword + "%' OR "
                               "c.name LIKE '%" + escaped_keyword + "%')";
            }
            
            // 价格范围过滤
            if (min_price >= 0) {
                where_clause += " AND p.price >= " + std::to_string(min_price);
            }
            if (max_price >= 0) {
                where_clause += " AND p.price <= " + std::to_string(max_price);
            }
            
            // 排序
            std::string order_clause = "ORDER BY p.created_at DESC";
            if (sort_by == "price_asc") {
                order_clause = "ORDER BY p.price ASC";
            } else if (sort_by == "price_desc") {
                order_clause = "ORDER BY p.price DESC";
            } else if (sort_by == "name_asc") {
                order_clause = "ORDER BY p.name ASC";
            }
            
            // 获取总数
            std::string count_sql = "SELECT COUNT(*) as total FROM products p "
                                   "LEFT JOIN categories c ON p.category_id = c.category_id " + where_clause;
            json count_result = executeQuery(count_sql);
            
            if (!count_result["success"].get<bool>()) {
                return count_result;
            }
            
            long total = count_result["data"][0]["total"].get<long>();
            
            // 获取搜索结果
            std::string sql = "SELECT p.product_id as id, p.name, p.description, p.price, "
                             "p.stock_quantity as stock, c.name as category, p.brand, "
                             "p.main_image, p.rating, p.review_count, p.created_at, p.updated_at "
                             "FROM products p "
                             "LEFT JOIN categories c ON p.category_id = c.category_id " + 
                             where_clause + " " + order_clause;
            sql = addPaginationToSQL(sql, validated_page, validated_page_size);
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["products"] = result["data"];
                response_data["total"] = total;
                response_data["page"] = validated_page;
                response_data["page_size"] = validated_page_size;
                response_data["total_pages"] = (total + validated_page_size - 1) / validated_page_size;
                response_data["keyword"] = keyword;
                response_data["min_price"] = min_price;
                response_data["max_price"] = max_price;
                response_data["sort_by"] = sort_by;
                
                return createSuccessResponse(response_data);
            } else {
                return result;
            }
            
        } catch (const std::exception& e) {
            std::string error_msg = "搜索商品异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 获取商品分类列表
    json getCategories() {
        logDebug("获取商品分类列表");
        
        std::string sql = "SELECT category_id as id, name, description, icon, sort_order "
                         "FROM categories WHERE status = 'active' ORDER BY sort_order, name";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["categories"] = result["data"];
            return createSuccessResponse(response_data);
        }
        
        return result;
    }
    
    // 获取分类下的商品
    json getCategoryProducts(const std::string& category, int page, int page_size, const std::string& sort_by) {
        logDebug("获取分类商品，分类: " + category);
        
        if (category.empty()) {
            return createErrorResponse("分类名称不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        return getProductList(category, page, page_size);
    }
    
    // 更新库存（原子操作）
    json updateStock(long product_id, int quantity, const std::string& operation) {
        logInfo("更新库存，商品ID: " + std::to_string(product_id) + 
               ", 数量: " + std::to_string(quantity) + ", 操作: " + operation);
        
        std::lock_guard<std::mutex> lock(stock_mutex_);
        
        if (!isProductExists(product_id)) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 获取当前库存
            json product_info = getProductById(product_id);
            if (product_info.empty()) {
                return createErrorResponse("获取商品信息失败", Constants::DATABASE_ERROR_CODE);
            }
            
            int current_stock = product_info["stock"].get<int>();
            int new_stock = current_stock;
            
            if (operation == "add") {
                new_stock = current_stock + quantity;
            } else if (operation == "subtract") {
                new_stock = current_stock - quantity;
                if (new_stock < 0) {
                    return createErrorResponse("库存不足", Constants::VALIDATION_ERROR_CODE);
                }
            } else if (operation == "set") {
                new_stock = quantity;
            } else {
                return createErrorResponse("无效的操作类型", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (new_stock < 0 || new_stock > Constants::MAX_PRODUCT_QUANTITY) {
                return createErrorResponse("库存数量超出有效范围", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新库存
            std::string sql = "UPDATE products SET stock_quantity = " + std::to_string(new_stock) + 
                             ", updated_at = NOW() WHERE product_id = " + std::to_string(product_id);
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["product_id"] = product_id;
                response_data["old_stock"] = current_stock;
                response_data["new_stock"] = new_stock;
                response_data["operation"] = operation;
                response_data["quantity"] = quantity;
                
                logInfo("库存更新成功，商品ID: " + std::to_string(product_id) + 
                       ", 新库存: " + std::to_string(new_stock));
                return createSuccessResponse(response_data, "库存更新成功");
            } else {
                return result;
            }
            
        } catch (const std::exception& e) {
            std::string error_msg = "更新库存异常: " + std::string(e.what());
            logError(error_msg);
            return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 检查库存
    json checkStock(long product_id) {
        logDebug("检查库存，商品ID: " + std::to_string(product_id));
        
        json product_info = getProductById(product_id);
        if (product_info.empty()) {
            return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        json response_data;
        response_data["product_id"] = product_id;
        response_data["stock"] = product_info["stock"];
        response_data["product_name"] = product_info["name"];
        response_data["available"] = product_info["stock"].get<int>() > 0;
        
        return createSuccessResponse(response_data);
    }
};

// 购物车服务类
class CartService : public BaseService {
private:
    std::mutex cart_mutex_;
    
    // 检查购物车项是否存在
    bool isCartItemExists(long user_id, long product_id) const {
        std::string sql = "SELECT COUNT(*) as count FROM cart WHERE user_id = " + 
                         std::to_string(user_id) + " AND product_id = " + std::to_string(product_id);
        
        json result = const_cast<CartService*>(this)->executeQuery(sql);
        if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
            return result["data"][0]["count"].get<long>() > 0;
        }
        return false;
    }
    
public:
    CartService() : BaseService() {
        logInfo("购物车服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "CartService";
    }
    
    // 添加商品到购物车
    json addToCart(long user_id, long product_id, int quantity) {
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
            std::string product_sql = "SELECT stock_quantity as stock, name FROM products WHERE product_id = " + 
                                     std::to_string(product_id) + " AND status = 'active'";
            json product_result = executeQuery(product_sql);
            
            if (!product_result["success"].get<bool>()) {
                return product_result;
            }
            
            if (product_result["data"].is_array() && product_result["data"].empty()) {
                return createErrorResponse("商品不存在或已下架", Constants::VALIDATION_ERROR_CODE);
            }
            
            int available_stock = product_result["data"][0]["stock"].get<int>();
            if (available_stock < quantity) {
                return createErrorResponse("库存不足，可用库存: " + std::to_string(available_stock), 
                                         Constants::VALIDATION_ERROR_CODE);
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
    json getCart(long user_id) {
        logDebug("获取购物车内容，用户ID: " + std::to_string(user_id));
        
        if (user_id <= 0) {
            return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            std::string sql = "SELECT c.cart_id as cart_item_id, c.product_id, c.quantity, c.created_at, "
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
    json removeFromCart(long user_id, long product_id) {
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
    json updateCartItemQuantity(long user_id, long product_id, int quantity) {
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
    
    // 清空购物车
    json clearCart(long user_id) {
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
};

// ====================================================================
// 用户地址服务类
// ====================================================================
class AddressService : public BaseService {
private:
    std::mutex address_mutex_;
    
public:
    AddressService() : BaseService() {
        logInfo("用户地址服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "AddressService";
    }
    
    // 添加用户地址
    json addUserAddress(long user_id, const std::string& receiver_name, const std::string& receiver_phone,
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
    json getUserAddresses(long user_id) {
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
    json updateUserAddress(long address_id, const std::string& receiver_name, const std::string& receiver_phone,
                          const std::string& province, const std::string& city, const std::string& district,
                          const std::string& detail_address, const std::string& postal_code, bool is_default) {
        logInfo("更新用户地址，地址ID: " + std::to_string(address_id));
        
        std::lock_guard<std::mutex> lock(address_mutex_);
        
        if (address_id <= 0) {
            return createErrorResponse("无效的地址ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 获取地址的用户ID
            std::string check_sql = "SELECT user_id FROM user_addresses WHERE address_id = " + 
                                   std::to_string(address_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("地址不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            long user_id = check_result["data"][0]["user_id"].get<long>();
            
            // 如果设置为默认地址，先取消其他默认地址
            if (is_default) {
                std::string update_default_sql = "UPDATE user_addresses SET is_default = 0 WHERE user_id = " + 
                                               std::to_string(user_id);
                executeQuery(update_default_sql);
            }
            
            // 更新地址信息
            std::string sql = "UPDATE user_addresses SET receiver_name = '" + receiver_name + 
                             "', receiver_phone = '" + receiver_phone + "', province = '" + province +
                             "', city = '" + city + "', district = '" + district + 
                             "', detail_address = '" + detail_address + "', postal_code = '" + postal_code +
                             "', is_default = " + (is_default ? "1" : "0") + 
                             ", updated_at = NOW() WHERE address_id = " + std::to_string(address_id);
            
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
    json deleteUserAddress(long address_id) {
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
    json setDefaultAddress(long user_id, long address_id) {
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
};

// ====================================================================
// 订单服务类
// ====================================================================
class OrderService : public BaseService {
private:
    std::mutex order_mutex_;
    
    // 生成订单号
    std::string generateOrderNo() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << "EM" << std::put_time(std::localtime(&time_t), "%Y%m%d%H%M%S") 
           << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
public:
    OrderService() : BaseService() {
        logInfo("订单服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "OrderService";
    }
    
    // 从购物车创建订单
    json createOrderFromCart(long user_id, long address_id, const std::string& coupon_code, const std::string& remark) {
        logInfo("从购物车创建订单，用户ID: " + std::to_string(user_id));
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (user_id <= 0 || address_id <= 0) {
            return createErrorResponse("无效的用户ID或地址ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 获取购物车内容
            std::string cart_sql = "SELECT c.product_id, c.quantity, p.name, p.price, "
                                  "(c.quantity * p.price) as subtotal "
                                  "FROM cart c JOIN products p ON c.product_id = p.product_id "
                                  "WHERE c.user_id = " + std::to_string(user_id) + 
                                  " AND c.selected = 1 AND p.status = 'active'";
            
            json cart_result = executeQuery(cart_sql);
            if (!cart_result["success"].get<bool>() || cart_result["data"].empty()) {
                return createErrorResponse("购物车为空或商品不可用", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 计算订单总金额
            double total_amount = 0.0;
            for (const auto& item : cart_result["data"]) {
                total_amount += item["subtotal"].get<double>();
            }
            
            // 生成订单号
            std::string order_no = generateOrderNo();
            
            // 创建订单
            std::string order_sql = "INSERT INTO orders (order_no, user_id, total_amount, final_amount, "
                                   "status, payment_status, remark) VALUES ('" +
                                   order_no + "', " + std::to_string(user_id) + ", " +
                                   std::to_string(total_amount) + ", " + std::to_string(total_amount) + 
                                   ", 'pending', 'unpaid', '" + remark + "')";
            
            json order_result = executeQuery(order_sql);
            if (!order_result["success"].get<bool>()) {
                return order_result;
            }
            
            long order_id = order_result["data"]["insert_id"].get<long>();
            
            // 创建订单明细
            for (const auto& item : cart_result["data"]) {
                std::string item_sql = "INSERT INTO order_items (order_id, product_id, product_name, "
                                      "price, quantity, subtotal) VALUES (" +
                                      std::to_string(order_id) + ", " +
                                      std::to_string(item["product_id"].get<long>()) + ", '" +
                                      item["name"].get<std::string>() + "', " +
                                      std::to_string(item["price"].get<double>()) + ", " +
                                      std::to_string(item["quantity"].get<int>()) + ", " +
                                      std::to_string(item["subtotal"].get<double>()) + ")";
                executeQuery(item_sql);
            }
            
            // 清空购物车
            std::string clear_cart_sql = "DELETE FROM cart WHERE user_id = " + std::to_string(user_id);
            executeQuery(clear_cart_sql);
            
            json response_data;
            response_data["order_id"] = order_id;
            response_data["order_no"] = order_no;
            response_data["total_amount"] = total_amount;
            response_data["item_count"] = cart_result["data"].size();
            
            logInfo("订单创建成功，订单ID: " + std::to_string(order_id));
            return createSuccessResponse(response_data, "订单创建成功");
            
        } catch (const std::exception& e) {
            return createErrorResponse("创建订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 获取用户订单列表
    json getUserOrders(long user_id) {
        logInfo("获取用户订单列表，用户ID: " + std::to_string(user_id));
        
        if (user_id <= 0) {
            return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            std::string sql = "SELECT order_id, order_no, total_amount, discount_amount, "
                             "shipping_fee, final_amount, status, payment_status, "
                             "created_at, updated_at FROM orders WHERE user_id = " +
                             std::to_string(user_id) + " ORDER BY created_at DESC";
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["orders"] = result["data"];
                response_data["total_count"] = result["data"].size();
                
                return createSuccessResponse(response_data, "获取订单列表成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("获取订单列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 获取订单详情
    json getOrderDetail(long order_id) {
        logInfo("获取订单详情，订单ID: " + std::to_string(order_id));
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 获取订单基本信息
            std::string order_sql = "SELECT * FROM orders WHERE order_id = " + std::to_string(order_id);
            json order_result = executeQuery(order_sql);
            
            if (!order_result["success"].get<bool>() || order_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 获取订单明细
            std::string items_sql = "SELECT * FROM order_items WHERE order_id = " + std::to_string(order_id);
            json items_result = executeQuery(items_sql);
            
            json response_data = order_result["data"][0];
            response_data["items"] = items_result["success"].get<bool>() ? items_result["data"] : json::array();
            
            return createSuccessResponse(response_data, "获取订单详情成功");
            
        } catch (const std::exception& e) {
            return createErrorResponse("获取订单详情异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 支付订单
    json payOrder(long order_id, const std::string& payment_method) {
        logInfo("支付订单，订单ID: " + std::to_string(order_id) + ", 支付方式: " + payment_method);
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 检查订单状态
            std::string check_sql = "SELECT status, payment_status, final_amount FROM orders WHERE order_id = " +
                                   std::to_string(order_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string status = check_result["data"][0]["status"].get<std::string>();
            std::string payment_status = check_result["data"][0]["payment_status"].get<std::string>();
            
            if (status != "pending") {
                return createErrorResponse("订单状态不允许支付", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (payment_status != "unpaid") {
                return createErrorResponse("订单已支付", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新订单状态
            std::string update_sql = "UPDATE orders SET status = 'paid', payment_status = 'paid', "
                                    "payment_method = '" + payment_method + "', paid_at = NOW(), "
                                    "updated_at = NOW() WHERE order_id = " + std::to_string(order_id);
            
            json result = executeQuery(update_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["order_id"] = order_id;
                response_data["payment_method"] = payment_method;
                response_data["payment_status"] = "paid";
                response_data["paid_amount"] = check_result["data"][0]["final_amount"];
                
                logInfo("订单支付成功，订单ID: " + std::to_string(order_id));
                return createSuccessResponse(response_data, "订单支付成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("支付订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 取消订单
    json cancelOrder(long order_id, const std::string& reason) {
        logInfo("取消订单，订单ID: " + std::to_string(order_id));
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        try {
            std::string sql = "UPDATE orders SET status = 'cancelled', admin_remark = '" + reason +
                             "', updated_at = NOW() WHERE order_id = " + std::to_string(order_id) +
                             " AND status IN ('pending', 'confirmed')";
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["order_id"] = order_id;
                response_data["status"] = "cancelled";
                response_data["reason"] = reason;
                
                logInfo("订单取消成功，订单ID: " + std::to_string(order_id));
                return createSuccessResponse(response_data, "订单取消成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("取消订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
};

// ====================================================================
// 优惠券服务类
// ====================================================================
class CouponService : public BaseService {
private:
    std::mutex coupon_mutex_;
    
public:
    CouponService() : BaseService() {
        logInfo("优惠券服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "CouponService";
    }
    
    // 获取可用优惠券列表
    json getAvailableCoupons() {
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
    
    // 获取用户优惠券
    json getUserCoupons(long user_id) {
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
    
    // 领取优惠券
    json claimCoupon(long user_id, const std::string& coupon_code) {
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
    
    // 使用优惠券
    json useCoupon(long user_id, long order_id, const std::string& coupon_code) {
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
};

// ====================================================================
// 商品评论服务类
// ====================================================================
class ReviewService : public BaseService {
private:
    std::mutex review_mutex_;
    
public:
    ReviewService() : BaseService() {
        logInfo("商品评论服务初始化完成");
    }
    
    std::string getServiceName() const override {
        return "ReviewService";
    }
    
    // 添加商品评论
    json addProductReview(long user_id, long product_id, long order_id, int rating, 
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
    
    // 获取商品评论列表
    json getProductReviews(long product_id, int page, int page_size, const std::string& sort_by) {
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
    
    // 获取用户评论列表
    json getUserReviews(long user_id, int page, int page_size) {
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
    
    // 审核评论（管理员功能）
    json reviewProductReview(long review_id, const std::string& status, const std::string& admin_note) {
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
    
private:
    // 更新商品评分统计
    void updateProductRating(long product_id) {
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
};

// 服务管理器类
class EmshopServiceManager {
private:
    std::unique_ptr<UserService> user_service_;
    std::unique_ptr<ProductService> product_service_;
    std::unique_ptr<CartService> cart_service_;
    std::unique_ptr<AddressService> address_service_;
    std::unique_ptr<OrderService> order_service_;
    std::unique_ptr<CouponService> coupon_service_;
    std::unique_ptr<ReviewService> review_service_;
    bool initialized_;
    std::mutex init_mutex_;
    
    EmshopServiceManager() : initialized_(false) {}
    
public:
    // 获取单例实例
    static EmshopServiceManager& getInstance() {
        static EmshopServiceManager instance;
        return instance;
    }
    
    // 禁用拷贝构造和赋值操作
    EmshopServiceManager(const EmshopServiceManager&) = delete;
    EmshopServiceManager& operator=(const EmshopServiceManager&) = delete;
    
    // 初始化所有服务
    bool initialize() {
        std::lock_guard<std::mutex> lock(init_mutex_);
        
        if (initialized_) {
            Logger::warn("服务管理器已经初始化");
            return true;
        }
        
        Logger::info("初始化Emshop服务管理器...");
        
        try {
            // 初始化日志系统
            Logger::initialize();
            Logger::setLevel(LogLevel::INFO);
            
            // 初始化数据库连接池
            DatabaseConnectionPool& pool = DatabaseConnectionPool::getInstance();
            if (!pool.initialize()) {
                Logger::error("数据库连接池初始化失败");
                return false;
            }
            
            // 创建服务实例
            user_service_.reset(new UserService());
            product_service_.reset(new ProductService());
            cart_service_.reset(new CartService());
            address_service_.reset(new AddressService());
            order_service_.reset(new OrderService());
            coupon_service_.reset(new CouponService());
            review_service_.reset(new ReviewService());
            
            initialized_ = true;
            Logger::info("Emshop服务管理器初始化成功");
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("服务管理器初始化异常: " + std::string(e.what()));
            return false;
        }
    }
    
    // 获取用户服务
    UserService& getUserService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *user_service_;
    }
    
    // 获取商品服务
    ProductService& getProductService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *product_service_;
    }
    
    // 获取购物车服务
    CartService& getCartService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *cart_service_;
    }
    
    // 获取地址服务
    AddressService& getAddressService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *address_service_;
    }
    
    // 获取订单服务
    OrderService& getOrderService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *order_service_;
    }
    
    // 获取优惠券服务
    CouponService& getCouponService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *coupon_service_;
    }
    
    // 获取评论服务
    ReviewService& getReviewService() {
        if (!initialized_) {
            throw std::runtime_error("服务管理器未初始化");
        }
        return *review_service_;
    }
    
    // 检查是否已初始化
    bool isInitialized() const {
        return initialized_;
    }
    
    // 关闭服务管理器
    void shutdown() {
        std::lock_guard<std::mutex> lock(init_mutex_);
        
        if (!initialized_) {
            return;
        }
        
        Logger::info("关闭Emshop服务管理器...");
        
        // 重置服务实例
        review_service_.reset();
        coupon_service_.reset();
        order_service_.reset();
        address_service_.reset();
        cart_service_.reset();
        product_service_.reset();
        user_service_.reset();
        
        // 关闭数据库连接池
        DatabaseConnectionPool::getInstance().shutdown();
        
        initialized_ = false;
        Logger::info("Emshop服务管理器已关闭");
    }
    
    ~EmshopServiceManager() {
        shutdown();
    }
};

// JNI 辅助函数
class JNIStringConverter {
public:
    // 安全的Java字符串转C++字符串 - 修复内存安全问题
    static std::string jstringToString(JNIEnv* env, jstring jstr) {
        if (!env || !jstr) {
            return "";
        }
        
        const char* chars = env->GetStringUTFChars(jstr, nullptr);
        if (!chars) {
            return "";
        }
        
        std::string result(chars);
        env->ReleaseStringUTFChars(jstr, chars);
        return result;
    }
    
    // 安全的C++字符串转Java字符串
    static jstring stringToJstring(JNIEnv* env, const std::string& str) {
        if (!env) {
            return nullptr;
        }
        return env->NewStringUTF(str.c_str());
    }
    
    // 安全的JSON对象转Java字符串 - 添加异常处理
    static jstring jsonToJstring(JNIEnv* env, const json& obj) {
        if (!env) {
            return nullptr;
        }
        try {
            std::string json_str = obj.dump();
            return env->NewStringUTF(json_str.c_str());
        } catch (const std::exception& e) {
            Logger::error("JSON转换失败: " + std::string(e.what()));
            return env->NewStringUTF("{\"success\":false,\"message\":\"JSON转换错误\"}");
        }
    }
};

// 全局服务管理器初始化标志（已移除，改用静态变量管理）

// 确保服务管理器已初始化 - 修复初始化安全问题
bool ensureServiceManagerInitialized() {
    static std::mutex init_safety_mutex;
    static bool global_init_success = false;
    static bool init_attempted = false;
    
    std::lock_guard<std::mutex> safety_lock(init_safety_mutex);
    
    try {
        // 如果还没有尝试初始化，则进行初始化
        if (!init_attempted) {
            init_attempted = true;
            try {
                global_init_success = EmshopServiceManager::getInstance().initialize();
                if (global_init_success) {
                    Logger::info("服务管理器初始化成功");
                } else {
                    Logger::error("服务管理器初始化失败");
                }
            } catch (const std::exception& e) {
                Logger::error("服务管理器初始化异常: " + std::string(e.what()));
                global_init_success = false;
            } catch (...) {
                Logger::error("服务管理器初始化发生未知异常");
                global_init_success = false;
            }
        }
        
        // 检查当前状态
        bool is_initialized = EmshopServiceManager::getInstance().isInitialized();
        return global_init_success && is_initialized;
        
    } catch (const std::exception& e) {
        Logger::error("初始化检查异常: " + std::string(e.what()));
        return false;
    } catch (...) {
        Logger::error("初始化检查发生未知异常");
        return false;
    }
}

// JNI 接口实现

// 系统初始化接口
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_initializeService
  (JNIEnv *env, jclass cls) {
    try {
        json response;
        bool success = ensureServiceManagerInitialized();
        
        response["success"] = success;
        if (success) {
            response["message"] = "服务初始化成功";
            Logger::info("手动服务初始化成功");
        } else {
            response["message"] = "服务初始化失败";
            Logger::error("手动服务初始化失败");
        }
        response["timestamp"] = std::time(nullptr);
        
        return JNIStringConverter::jsonToJstring(env, response);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "初始化异常: " + std::string(e.what());
        error_response["timestamp"] = std::time(nullptr);
        Logger::error("初始化异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getInitializationStatus
  (JNIEnv *env, jclass cls) {
    try {
        json response;
        bool initialized = ensureServiceManagerInitialized() && EmshopServiceManager::getInstance().isInitialized();
        
        response["initialized"] = initialized;
        response["timestamp"] = std::time(nullptr);
        
        if (initialized) {
            response["message"] = "服务已初始化";
            response["database_pool_status"] = "已初始化";
        } else {
            response["message"] = "服务未初始化";
        }
        
        return JNIStringConverter::jsonToJstring(env, response);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["initialized"] = false;
        error_response["message"] = "状态检查异常: " + std::string(e.what());
        error_response["timestamp"] = std::time(nullptr);
        Logger::error("状态检查异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 用户管理接口
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_login
  (JNIEnv *env, jclass cls, jstring username, jstring password) {
    
    // 参数安全检查
    if (!env) {
        return nullptr;
    }
    
    if (!username || !password) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "参数无效：用户名或密码为空";
        error_response["error_code"] = Constants::VALIDATION_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string user_name = JNIStringConverter::jstringToString(env, username);
        std::string pass = JNIStringConverter::jstringToString(env, password);
        
        Logger::info("处理登录请求，用户名: " + user_name);
        
        UserService& userService = EmshopServiceManager::getInstance().getUserService();
        json result = userService.loginUser(user_name, pass);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "登录过程发生异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("登录异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    } catch (...) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "登录过程发生未知异常";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("登录发生未知异常");
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_register
  (JNIEnv *env, jclass cls, jstring username, jstring password, jstring phone) {
    
    // 参数安全检查
    if (!env) {
        return nullptr;
    }
    
    if (!username || !password || !phone) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "参数无效：用户名、密码或手机号为空";
        error_response["error_code"] = Constants::VALIDATION_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        // 安全的字符串转换
        std::string user_name = JNIStringConverter::jstringToString(env, username);
        std::string pass = JNIStringConverter::jstringToString(env, password);
        std::string phone_num = JNIStringConverter::jstringToString(env, phone);
        
        Logger::info("处理注册请求，用户名: " + user_name);
        
        UserService& userService = EmshopServiceManager::getInstance().getUserService();
        json result = userService.registerUser(user_name, pass, phone_num);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "注册过程发生异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("注册异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    } catch (...) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "注册过程发生未知异常";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("注册发生未知异常");
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserInfo
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        UserService& userService = EmshopServiceManager::getInstance().getUserService();
        json result = userService.getUserInfo(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取用户信息异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getProductList
  (JNIEnv *env, jclass cls, jstring category, jint page, jint pageSize) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string cat = JNIStringConverter::jstringToString(env, category);
        
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.getProductList(cat, static_cast<int>(page), static_cast<int>(pageSize));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取商品列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_searchProducts
  (JNIEnv *env, jclass cls, jstring keyword, jint page, jint pageSize, 
   jstring sortBy, jdouble minPrice, jdouble maxPrice) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string keyword_str = JNIStringConverter::jstringToString(env, keyword);
        std::string sort_by_str = JNIStringConverter::jstringToString(env, sortBy);
        
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.searchProducts(keyword_str, static_cast<int>(page), 
                                                   static_cast<int>(pageSize), sort_by_str,
                                                   static_cast<double>(minPrice), 
                                                   static_cast<double>(maxPrice));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "搜索商品异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getProductDetail
  (JNIEnv *env, jclass cls, jlong productId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.getProductDetail(static_cast<long>(productId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取商品详情异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getCategories
  (JNIEnv *env, jclass cls) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.getCategories();
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取分类列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_addToCart
  (JNIEnv *env, jclass cls, jlong userId, jlong productId, jint quantity) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CartService& cartService = EmshopServiceManager::getInstance().getCartService();
        json result = cartService.addToCart(static_cast<long>(userId), 
                                           static_cast<long>(productId), 
                                           static_cast<int>(quantity));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "添加到购物车异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getCart
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CartService& cartService = EmshopServiceManager::getInstance().getCartService();
        json result = cartService.getCart(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取购物车异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_updateCartItemQuantity
  (JNIEnv *env, jclass cls, jlong userId, jlong productId, jint quantity) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CartService& cartService = EmshopServiceManager::getInstance().getCartService();
        json result = cartService.updateCartItemQuantity(static_cast<long>(userId), 
                                                        static_cast<long>(productId), 
                                                        static_cast<int>(quantity));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "更新购物车数量异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_removeFromCart
  (JNIEnv *env, jclass cls, jlong userId, jlong productId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CartService& cartService = EmshopServiceManager::getInstance().getCartService();
        json result = cartService.removeFromCart(static_cast<long>(userId), 
                                                static_cast<long>(productId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "移除购物车商品异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_clearCart
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CartService& cartService = EmshopServiceManager::getInstance().getCartService();
        json result = cartService.clearCart(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "清空购物车异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 库存管理接口
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_checkStock
  (JNIEnv *env, jclass cls, jlong productId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        Logger::info("检查商品库存，商品ID: " + std::to_string(productId));
        
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.checkStock(static_cast<long>(productId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "检查库存异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("检查库存异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_updateStock
  (JNIEnv *env, jclass cls, jlong productId, jint quantity, jstring operation) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string op = JNIStringConverter::jstringToString(env, operation);
        Logger::info("更新商品库存，商品ID: " + std::to_string(productId) + 
                    ", 数量: " + std::to_string(quantity) + ", 操作: " + op);
        
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.updateStock(static_cast<long>(productId), static_cast<int>(quantity), op);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "更新库存异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("更新库存异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getLowStockProducts
  (JNIEnv *env, jclass cls, jint threshold) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        Logger::info("获取低库存商品，阈值: " + std::to_string(threshold));
        
        std::string sql = "SELECT product_id, name, stock_quantity as stock, price, category_id as category, status, main_image "
                         "FROM products WHERE stock_quantity <= " + std::to_string(threshold) + 
                         " AND status = 'active' ORDER BY stock_quantity ASC";
        
        ProductService& productService = EmshopServiceManager::getInstance().getProductService();
        json result = productService.executeQuery(sql);
        
        if (result["success"].get<bool>()) {
            json response_data;
            response_data["threshold"] = threshold;
            response_data["low_stock_products"] = result["data"];
            response_data["count"] = result["data"].size();
            
            return JNIStringConverter::jsonToJstring(env, productService.createSuccessResponse(response_data, "获取低库存商品列表成功"));
        }
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取低库存商品异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        Logger::error("获取低库存商品异常: " + std::string(e.what()));
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 简化实现：为其他方法返回"功能未实现"的占位符响应
// 这样可以确保编译通过，后续可以逐个完善

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_logout
  (JNIEnv *env, jclass cls, jlong userId) {
    json response;
    response["success"] = true;
    response["message"] = "登出成功";
    return JNIStringConverter::jsonToJstring(env, response);
}

// ====================================================================
// 用户地址管理JNI实现
// ====================================================================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_addUserAddress
  (JNIEnv *env, jclass cls, jlong userId, jstring receiverName, jstring receiverPhone,
   jstring province, jstring city, jstring district, jstring detailAddress, 
   jstring postalCode, jboolean isDefault) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        // 转换字符串参数
        std::string receiver_name = JNIStringConverter::jstringToString(env, receiverName);
        std::string receiver_phone = JNIStringConverter::jstringToString(env, receiverPhone);
        std::string province_str = JNIStringConverter::jstringToString(env, province);
        std::string city_str = JNIStringConverter::jstringToString(env, city);
        std::string district_str = JNIStringConverter::jstringToString(env, district);
        std::string detail_address = JNIStringConverter::jstringToString(env, detailAddress);
        std::string postal_code = JNIStringConverter::jstringToString(env, postalCode);
        
        AddressService& addressService = EmshopServiceManager::getInstance().getAddressService();
        json result = addressService.addUserAddress(static_cast<long>(userId), receiver_name, receiver_phone,
                                                   province_str, city_str, district_str, detail_address,
                                                   postal_code, static_cast<bool>(isDefault));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "添加地址异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserAddresses
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        AddressService& addressService = EmshopServiceManager::getInstance().getAddressService();
        json result = addressService.getUserAddresses(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取地址列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_deleteUserAddress
  (JNIEnv *env, jclass cls, jlong addressId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        AddressService& addressService = EmshopServiceManager::getInstance().getAddressService();
        json result = addressService.deleteUserAddress(static_cast<long>(addressId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "删除地址异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_setDefaultAddress
  (JNIEnv *env, jclass cls, jlong userId, jlong addressId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        AddressService& addressService = EmshopServiceManager::getInstance().getAddressService();
        json result = addressService.setDefaultAddress(static_cast<long>(userId), static_cast<long>(addressId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "设置默认地址异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// ====================================================================
// 订单管理JNI实现
// ====================================================================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_createOrderFromCart
  (JNIEnv *env, jclass cls, jlong userId, jlong addressId, jstring couponCode, jstring remark) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string coupon_code = couponCode ? JNIStringConverter::jstringToString(env, couponCode) : "";
        std::string remark_str = remark ? JNIStringConverter::jstringToString(env, remark) : "";
        
        OrderService& orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService.createOrderFromCart(static_cast<long>(userId), static_cast<long>(addressId),
                                                      coupon_code, remark_str);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "创建订单异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getOrderList
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        OrderService& orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService.getUserOrders(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取订单列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getOrderDetail
  (JNIEnv *env, jclass cls, jlong orderId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        OrderService& orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService.getOrderDetail(static_cast<long>(orderId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取订单详情异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_payOrder
  (JNIEnv *env, jclass cls, jlong orderId, jstring paymentMethod) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string payment_method = JNIStringConverter::jstringToString(env, paymentMethod);
        
        OrderService& orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService.payOrder(static_cast<long>(orderId), payment_method);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "支付订单异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_cancelOrder
  (JNIEnv *env, jclass cls, jlong userId, jlong orderId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        OrderService& orderService = EmshopServiceManager::getInstance().getOrderService();
        json result = orderService.cancelOrder(static_cast<long>(orderId), "用户取消");
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "取消订单异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// ====================================================================
// 优惠券管理JNI实现
// ====================================================================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getAvailableCoupons
  (JNIEnv *env, jclass cls) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CouponService& couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService.getAvailableCoupons();
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取优惠券列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserCoupons
  (JNIEnv *env, jclass cls, jlong userId) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        CouponService& couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService.getUserCoupons(static_cast<long>(userId));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取用户优惠券异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_claimCoupon
  (JNIEnv *env, jclass cls, jlong userId, jstring couponCode) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string coupon_code = JNIStringConverter::jstringToString(env, couponCode);
        
        CouponService& couponService = EmshopServiceManager::getInstance().getCouponService();
        json result = couponService.claimCoupon(static_cast<long>(userId), coupon_code);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "领取优惠券异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// ====================================================================
// 商品评论JNI实现
// ====================================================================

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_addProductReview
  (JNIEnv *env, jclass cls, jlong userId, jlong productId, jlong orderId, 
   jint rating, jstring content, jboolean isAnonymous) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string content_str = JNIStringConverter::jstringToString(env, content);
        
        ReviewService& reviewService = EmshopServiceManager::getInstance().getReviewService();
        json result = reviewService.addProductReview(static_cast<long>(userId), static_cast<long>(productId),
                                                    static_cast<long>(orderId), static_cast<int>(rating),
                                                    content_str, static_cast<bool>(isAnonymous));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "添加评论异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getProductReviews
  (JNIEnv *env, jclass cls, jlong productId, jint page, jint pageSize, jstring sortBy) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        std::string sort_by = sortBy ? JNIStringConverter::jstringToString(env, sortBy) : "created_at";
        
        ReviewService& reviewService = EmshopServiceManager::getInstance().getReviewService();
        json result = reviewService.getProductReviews(static_cast<long>(productId), static_cast<int>(page),
                                                     static_cast<int>(pageSize), sort_by);
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取评论列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_getUserReviews
  (JNIEnv *env, jclass cls, jlong userId, jint page, jint pageSize) {
    
    if (!ensureServiceManagerInitialized()) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "服务未初始化";
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
    
    try {
        ReviewService& reviewService = EmshopServiceManager::getInstance().getReviewService();
        json result = reviewService.getUserReviews(static_cast<long>(userId), static_cast<int>(page),
                                                   static_cast<int>(pageSize));
        
        return JNIStringConverter::jsonToJstring(env, result);
        
    } catch (const std::exception& e) {
        json error_response;
        error_response["success"] = false;
        error_response["message"] = "获取用户评论列表异常: " + std::string(e.what());
        error_response["error_code"] = Constants::DATABASE_ERROR_CODE;
        return JNIStringConverter::jsonToJstring(env, error_response);
    }
}

// 为了确保编译成功，我们为所有其他JNI方法提供简单的占位实现
// 实际项目中应该完整实现所有方法

#define PLACEHOLDER_IMPLEMENTATION(methodName) \
JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_##methodName \
  (JNIEnv *env, jclass cls, ...) { \
    json response; \
    response["success"] = false; \
    response["message"] = #methodName " 功能正在开发中"; \
    response["error_code"] = 9999; \
    return JNIStringConverter::jsonToJstring(env, response); \
}

// 由于参数类型不同，需要为每个方法单独实现占位符
// 这里只是展示核心架构，实际应用中需要完整实现

/*
 * 编译命令示例：
 * g++ -std=c++17 -shared -fPIC -O2 -DNDEBUG \
 *     -I$JAVA_HOME/include -I$JAVA_HOME/include/linux \
 *     -I/usr/include/mysql -L/usr/lib64/mysql \
 *     -lmysqlclient -lpthread \
 *     -o emshop_native.so emshop_native_impl_oop.cpp
 * 
 * Windows编译：
 * g++ -std=c++17 -shared -O2 -DNDEBUG \
 *     -I%JAVA_HOME%\include -I%JAVA_HOME%\include\win32 \
 *     -ID:\MySQL\include -LD:\MySQL\lib \
 *     -lmysql -o emshop_native.dll emshop_native_impl_oop.cpp
 */
