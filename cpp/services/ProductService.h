#ifndef PRODUCTSERVICE_H
#define PRODUCTSERVICE_H

#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include "../nlohmann_json.hpp"

using json = nlohmann::json;

// 前向声明 - BaseService将在主文件中定义
class BaseService;

/**
 * 商品服务类
 * 处理商品相关的所有业务逻辑
 * 包括商品CRUD、库存管理、分类、搜索等
 */
class ProductService : public BaseService {
private:
    std::mutex stock_mutex_;  // 库存操作互斥锁
    
    // 列名辅助方法
    const std::string& getProductIdColumnName() const;
    const std::string& getProductStockColumnName() const;
    const std::string& getProductCategoryColumnName() const;
    const std::string& getProductStatusColumnName() const;
    const std::string& getProductImageColumnName() const;
    const std::string& getProductCreatedAtColumnName() const;
    const std::string& getProductUpdatedAtColumnName() const;
    std::string qualifyProductColumn(const std::string& alias, const std::string& column_name) const;
    
    // 验证商品输入
    json validateProductInput(const json& product_info) const;
    
    // 检查商品是否存在
    bool isProductExists(long product_id) const;
    
    // 获取商品详细信息（内部方法）
    json getProductById(long product_id) const;
    
public:
    ProductService();
    
    std::string getServiceName() const override;
    
    // 商品CRUD操作
    json addProduct(const json& product_info);
    json updateProduct(long product_id, const json& update_info);
    json deleteProduct(long product_id);
    json getProductDetail(long product_id);
    json getProductList(const std::string& category, int page, int page_size);
    
    // 商品搜索
    json searchProducts(const std::string& keyword, int page, int page_size, 
                       const std::string& sort_by, double min_price, double max_price);
    
    // 分类管理
    json getCategories();
    json getCategoryProducts(const std::string& category, int page, int page_size, const std::string& sort_by);
    
    // 库存管理
    json updateStock(long product_id, int quantity, const std::string& operation);
    json checkStock(long product_id);
    json getLowStockProducts(int threshold);
};

#endif // PRODUCTSERVICE_H
