#ifndef EMSHOP_CONFIG_LOADER_H
#define EMSHOP_CONFIG_LOADER_H

#include <string>
#include <map>
#include <fstream>
#include <stdexcept>
#include "nlohmann_json.hpp"

using json = nlohmann::json;

/**
 * @brief 配置加载器类 - 从config.json或环境变量加载配置
 * @details 支持配置文件和环境变量两种方式，环境变量优先级更高
 */
class ConfigLoader {
private:
    json config;
    static ConfigLoader* instance;
    
    ConfigLoader() {
        loadConfig();
    }
    
    // 从环境变量获取值
    std::string getEnv(const char* key, const std::string& defaultValue = "") {
        const char* val = std::getenv(key);
        return val ? std::string(val) : defaultValue;
    }
    
    // 加载配置文件
    void loadConfig() {
        // 尝试从config.json加载
        std::ifstream configFile("config.json");
        if (!configFile.is_open()) {
            // 如果没有config.json，尝试使用默认配置
            configFile.open("config.example.json");
        }
        
        if (configFile.is_open()) {
            try {
                configFile >> config;
                configFile.close();
            } catch (const std::exception& e) {
                throw std::runtime_error("Failed to parse config file: " + std::string(e.what()));
            }
        } else {
            // 使用默认配置
            config = getDefaultConfig();
        }
        
        // 环境变量覆盖配置文件
        overrideWithEnv();
    }
    
    // 默认配置
    json getDefaultConfig() {
        return json{
            {"database", {
                {"host", "localhost"},
                {"port", 3306},
                {"name", "emshop"},
                {"user", "root"},
                {"password", ""},
                {"charset", "utf8mb4"}
            }},
            {"server", {
                {"port", 8888},
                {"max_connections", 100},
                {"timeout_seconds", 30}
            }},
            {"logging", {
                {"level", "INFO"},
                {"file", "logs/emshop.log"},
                {"max_size_mb", 50},
                {"max_files", 10}
            }},
            {"security", {
                {"jwt_secret", ""},
                {"session_timeout_minutes", 120},
                {"password_salt", ""}
            }},
            {"business", {
                {"low_stock_threshold", 10},
                {"order_timeout_minutes", 30},
                {"max_cart_items", 99}
            }}
        };
    }
    
    // 用环境变量覆盖配置
    void overrideWithEnv() {
        // 数据库配置
        if (!getEnv("DB_HOST").empty()) 
            config["database"]["host"] = getEnv("DB_HOST");
        if (!getEnv("DB_PORT").empty()) 
            config["database"]["port"] = std::stoi(getEnv("DB_PORT"));
        if (!getEnv("DB_NAME").empty()) 
            config["database"]["name"] = getEnv("DB_NAME");
        if (!getEnv("DB_USER").empty()) 
            config["database"]["user"] = getEnv("DB_USER");
        if (!getEnv("DB_PASSWORD").empty()) 
            config["database"]["password"] = getEnv("DB_PASSWORD");
        
        // 服务器配置
        if (!getEnv("SERVER_PORT").empty()) 
            config["server"]["port"] = std::stoi(getEnv("SERVER_PORT"));
        
        // 日志配置
        if (!getEnv("LOG_LEVEL").empty()) 
            config["logging"]["level"] = getEnv("LOG_LEVEL");
        if (!getEnv("LOG_FILE").empty()) 
            config["logging"]["file"] = getEnv("LOG_FILE");
        
        // 安全配置
        if (!getEnv("JWT_SECRET").empty()) 
            config["security"]["jwt_secret"] = getEnv("JWT_SECRET");
        if (!getEnv("PASSWORD_SALT").empty()) 
            config["security"]["password_salt"] = getEnv("PASSWORD_SALT");
    }
    
public:
    // 单例模式获取实例
    static ConfigLoader& getInstance() {
        if (!instance) {
            instance = new ConfigLoader();
        }
        return *instance;
    }
    
    // 获取字符串配置
    std::string getString(const std::string& section, const std::string& key, 
                         const std::string& defaultValue = "") {
        try {
            if (config.contains(section) && config[section].contains(key)) {
                return config[section][key].get<std::string>();
            }
        } catch (...) {}
        return defaultValue;
    }
    
    // 获取整数配置
    int getInt(const std::string& section, const std::string& key, int defaultValue = 0) {
        try {
            if (config.contains(section) && config[section].contains(key)) {
                return config[section][key].get<int>();
            }
        } catch (...) {}
        return defaultValue;
    }
    
    // 获取布尔配置
    bool getBool(const std::string& section, const std::string& key, bool defaultValue = false) {
        try {
            if (config.contains(section) && config[section].contains(key)) {
                return config[section][key].get<bool>();
            }
        } catch (...) {}
        return defaultValue;
    }
    
    // 重新加载配置
    void reload() {
        loadConfig();
    }
    
    // 获取完整配置JSON
    json getConfig() const {
        return config;
    }
    
    // 禁止拷贝
    ConfigLoader(const ConfigLoader&) = delete;
    ConfigLoader& operator=(const ConfigLoader&) = delete;
};

// 静态成员初始化
ConfigLoader* ConfigLoader::instance = nullptr;

#endif // EMSHOP_CONFIG_LOADER_H
