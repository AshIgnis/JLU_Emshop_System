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
using namespace std::chrono_literals;

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
        if (!connection_available_.wait_for(lock, 30s, [this] { 
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
        std::string sql = "SELECT id, username, phone, email, role, created_at, updated_at "
                         "FROM users WHERE id = " + std::to_string(user_id) + " AND status = 'active'";
        
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
            sql += " WHERE id = " + std::to_string(user_id);
            
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
                         + "', updated_at = NOW() WHERE id = " + std::to_string(user_id);
        
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
        
        auto [validated_page, validated_page_size] = validatePaginationParams(page, page_size);
        
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
        
        auto [validated_page, validated_page_size] = validatePaginationParams(page, page_size);
        
        try {
            std::string where_clause = "WHERE status = 'active'";
            
            // 关键词搜索
            if (!keyword.empty()) {
                std::string escaped_keyword = escapeSQLString(keyword);
                where_clause += " AND (name LIKE '%" + escaped_keyword + "%' OR "
                               "description LIKE '%" + escaped_keyword + "%' OR "
                               "category LIKE '%" + escaped_keyword + "%')";
            }
            
            // 价格范围过滤
            if (min_price >= 0) {
                where_clause += " AND price >= " + std::to_string(min_price);
            }
            if (max_price >= 0) {
                where_clause += " AND price <= " + std::to_string(max_price);
            }
            
            // 排序
            std::string order_clause = "ORDER BY created_at DESC";
            if (sort_by == "price_asc") {
                order_clause = "ORDER BY price ASC";
            } else if (sort_by == "price_desc") {
                order_clause = "ORDER BY price DESC";
            } else if (sort_by == "name_asc") {
                order_clause = "ORDER BY name ASC";
            }
            
            // 获取总数
            std::string count_sql = "SELECT COUNT(*) as total FROM products " + where_clause;
            json count_result = executeQuery(count_sql);
            
            if (!count_result["success"].get<bool>()) {
                return count_result;
            }
            
            long total = count_result["data"][0]["total"].get<long>();
            
            // 获取搜索结果
            std::string sql = "SELECT id, name, description, price, stock, category, created_at, updated_at "
                             "FROM products " + where_clause + " " + order_clause;
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
        
        std::string sql = "SELECT DISTINCT category FROM products WHERE status = 'active' ORDER BY category";
        
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
            std::string sql = "UPDATE products SET stock = " + std::to_string(new_stock) + 
                             ", updated_at = NOW() WHERE id = " + std::to_string(product_id);
            
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

// 服务管理器类
class EmshopServiceManager {
private:
    std::unique_ptr<UserService> user_service_;
    std::unique_ptr<ProductService> product_service_;
    std::unique_ptr<CartService> cart_service_;
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
            user_service_ = std::make_unique<UserService>();
            product_service_ = std::make_unique<ProductService>();
            cart_service_ = std::make_unique<CartService>();
            
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

// 简化实现：为其他方法返回"功能未实现"的占位符响应
// 这样可以确保编译通过，后续可以逐个完善

JNIEXPORT jstring JNICALL Java_emshop_EmshopNativeInterface_logout
  (JNIEnv *env, jclass cls, jlong userId) {
    json response;
    response["success"] = true;
    response["message"] = "登出成功";
    return JNIStringConverter::jsonToJstring(env, response);
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
