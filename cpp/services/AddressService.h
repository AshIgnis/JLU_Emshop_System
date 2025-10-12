#ifndef ADDRESSSERVICE_H
#define ADDRESSSERVICE_H

#include <string>
#include <mutex>
#include "../nlohmann_json.hpp"

using json = nlohmann::json;

// 前向声明 - BaseService将在主文件中定义
class BaseService;

/**
 * 用户地址服务类
 * 处理用户收货地址的管理
 * 包括添加、查询、修改、删除地址,设置默认地址等
 */
class AddressService : public BaseService {
private:
    std::mutex address_mutex_;  ///< 地址操作互斥锁,保证线程安全
    
public:
    /**
     * 构造函数
     * 初始化地址服务
     */
    AddressService();
    
    /**
     * 获取服务名称
     * @return 服务名称 "AddressService"
     */
    std::string getServiceName() const override;
    
    /**
     * 添加用户地址
     * @param user_id 用户ID
     * @param receiver_name 收货人姓名
     * @param receiver_phone 收货人电话
     * @param province 省份
     * @param city 城市
     * @param district 区/县
     * @param detail_address 详细地址
     * @param postal_code 邮政编码
     * @param is_default 是否设为默认地址
     * @return JSON响应,包含新建地址的ID
     */
    json addUserAddress(long user_id, const std::string& receiver_name, const std::string& receiver_phone,
                       const std::string& province, const std::string& city, const std::string& district,
                       const std::string& detail_address, const std::string& postal_code, bool is_default);
    
    /**
     * 获取用户的所有地址
     * @param user_id 用户ID
     * @return JSON响应,包含地址列表
     */
    json getUserAddresses(long user_id);
    
    /**
     * 更新用户地址信息
     * @param address_id 地址ID
     * @param receiver_name 收货人姓名
     * @param receiver_phone 收货人电话
     * @param province 省份
     * @param city 城市
     * @param district 区/县
     * @param detail_address 详细地址
     * @param postal_code 邮政编码
     * @return JSON响应
     */
    json updateUserAddress(long address_id, const std::string& receiver_name, const std::string& receiver_phone,
                          const std::string& province, const std::string& city, const std::string& district,
                          const std::string& detail_address, const std::string& postal_code);
    
    /**
     * 删除用户地址
     * @param address_id 地址ID
     * @return JSON响应
     */
    json deleteUserAddress(long address_id);
    
    /**
     * 设置默认地址
     * @param user_id 用户ID
     * @param address_id 要设为默认的地址ID
     * @return JSON响应
     */
    json setDefaultAddress(long user_id, long address_id);
};

#endif // ADDRESSSERVICE_H
