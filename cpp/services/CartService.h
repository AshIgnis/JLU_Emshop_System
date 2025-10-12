#ifndef CART_SERVICE_H
#define CART_SERVICE_H

#include <mutex>
#include "../nlohmann_json.hpp"

using json = nlohmann::json;

// 前向声明
class BaseService;

/**
 * @class CartService
 * @brief 购物车服务类 - 处理购物车相关业务逻辑
 * 
 * 功能:
 * - 添加商品到购物车(支持库存检查、数量累加)
 * - 获取购物车内容(含商品详情、小计、总计)
 * - 移除购物车商品
 * - 更新购物车商品数量
 * - 更新购物车条目选中状态(支持单选/全选)
 * - 清空购物车
 * 
 * 线程安全:
 * - 使用cart_mutex_保护购物车操作
 */
class CartService : public BaseService {
private:
    std::mutex cart_mutex_;  // 购物车操作互斥锁
    
    /**
     * @brief 检查购物车项是否存在
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @return true-存在, false-不存在
     */
    bool isCartItemExists(long user_id, long product_id) const;
    
public:
    CartService();
    
    std::string getServiceName() const override;
    
    /**
     * @brief 添加商品到购物车
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @param quantity 数量
     * @return JSON响应 {success, data:{user_id,product_id,quantity,action}, message}
     */
    json addToCart(long user_id, long product_id, int quantity);
    
    /**
     * @brief 获取购物车内容
     * @param user_id 用户ID
     * @return JSON响应 {success, data:{items,total_items,total_amount}}
     */
    json getCart(long user_id);
    
    /**
     * @brief 从购物车移除商品
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @return JSON响应 {success, message}
     */
    json removeFromCart(long user_id, long product_id);
    
    /**
     * @brief 更新购物车商品数量
     * @param user_id 用户ID
     * @param product_id 商品ID
     * @param quantity 新数量
     * @return JSON响应 {success, data:{user_id,product_id,new_quantity}}
     */
    json updateCartItemQuantity(long user_id, long product_id, int quantity);
    
    /**
     * @brief 更新购物车条目选中状态
     * @param user_id 用户ID
     * @param product_id 商品ID (<=0表示全选/全不选)
     * @param selected 选中状态
     * @return JSON响应 {success, data:{user_id,product_id,selected}}
     */
    json updateCartSelected(long user_id, long product_id, bool selected);
    
    /**
     * @brief 清空购物车
     * @param user_id 用户ID
     * @return JSON响应 {success, data:{user_id,removed_items}}
     */
    json clearCart(long user_id);
};

#endif // CART_SERVICE_H
