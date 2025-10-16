#include "ProductService.h"
#include <sstream>
#include <cctype>

// ==================== 私有辅助方法 ====================

const std::string& ProductService::getProductIdColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "product_id")) return "product_id";
        if (hasColumn("products", "id")) return "id";
        return "product_id";
    }();
    return column;
}

const std::string& ProductService::getProductStockColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "stock_quantity")) return "stock_quantity";
        if (hasColumn("products", "stock")) return "stock";
        return "stock_quantity";
    }();
    return column;
}

const std::string& ProductService::getProductCategoryColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "category_id")) return "category_id";
        if (hasColumn("products", "category")) return "category";
        return std::string();
    }();
    return column;
}

const std::string& ProductService::getProductStatusColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "status")) return "status";
        return std::string();
    }();
    return column;
}

const std::string& ProductService::getProductImageColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "main_image")) return "main_image";
        if (hasColumn("products", "image_url")) return "image_url";
        return std::string();
    }();
    return column;
}

const std::string& ProductService::getProductCreatedAtColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "created_at")) return "created_at";
        if (hasColumn("products", "create_time")) return "create_time";
        return std::string();
    }();
    return column;
}

const std::string& ProductService::getProductUpdatedAtColumnName() const {
    static const std::string column = [this]() -> std::string {
        if (hasColumn("products", "updated_at")) return "updated_at";
        if (hasColumn("products", "update_time")) return "update_time";
        return std::string();
    }();
    return column;
}

std::string ProductService::qualifyProductColumn(const std::string& alias, const std::string& column_name) const {
    if (column_name.empty()) {
        return "";
    }
    return alias.empty() ? column_name : alias + "." + column_name;
}

json ProductService::validateProductInput(const json& product_info) const {
    if (!product_info.contains("name") || !product_info["name"].is_string() ||
        product_info["name"].get<std::string>().empty()) {
        return createErrorResponse("商品名称不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    if (!product_info.contains("price") || !product_info["price"].is_number() ||
        product_info["price"].get<double>() < Constants::MIN_PRICE ||
        product_info["price"].get<double>() > Constants::MAX_PRICE) {
        return createErrorResponse("商品价格必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 兼容 stock / stock_quantity 字段
    int stock_value = 0;
    if (product_info.contains("stock") && product_info["stock"].is_number_integer()) {
        stock_value = product_info["stock"].get<int>();
    } else if (product_info.contains("stock_quantity") && product_info["stock_quantity"].is_number_integer()) {
        stock_value = product_info["stock_quantity"].get<int>();
    } else {
        return createErrorResponse("库存数量必须提供", Constants::VALIDATION_ERROR_CODE);
    }

    if (stock_value < 0 || stock_value > Constants::MAX_PRODUCT_QUANTITY) {
        return createErrorResponse("库存数量必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 分类可以通过 category_id（数字）或 category（名称）提供
    if (product_info.contains("category_id") && product_info["category_id"].is_number_integer()) {
        if (product_info["category_id"].get<int>() <= 0) {
            return createErrorResponse("分类ID必须为正整数", Constants::VALIDATION_ERROR_CODE);
        }
    } else if (!(product_info.contains("category") && product_info["category"].is_string() &&
               !product_info["category"].get<std::string>().empty())) {
        return createErrorResponse("商品分类不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    return createSuccessResponse();
}

bool ProductService::isProductExists(long product_id) const {
    std::string sql = "SELECT COUNT(*) as count FROM products WHERE product_id = " + 
                     std::to_string(product_id) + " AND status != 'deleted'";
    
    json result = const_cast<ProductService*>(this)->executeQuery(sql);
    if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
        return result["data"][0]["count"].get<long>() > 0;
    }
    return false;
}

json ProductService::getProductById(long product_id) const {
    std::string sql = "SELECT product_id as id, name, description, price, stock_quantity as stock, "
                     "category_id as category, status, created_at, updated_at FROM products WHERE product_id = " + 
                     std::to_string(product_id) + " AND status != 'deleted'";
    
    json result = const_cast<ProductService*>(this)->executeQuery(sql);
    if (result["success"].get<bool>() && result["data"].is_array() && !result["data"].empty()) {
        return result["data"][0];
    }
    return json::object();
}

// ==================== 公共接口方法 ====================

ProductService::ProductService() : BaseService() {
    logInfo("商品服务初始化完成");
}

std::string ProductService::getServiceName() const {
    return "ProductService";
}

json ProductService::addProduct(const json& product_info) {
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

        int stock = product_info.contains("stock") ?
            product_info["stock"].get<int>() : product_info["stock_quantity"].get<int>();

        long category_id = 0;
        if (product_info.contains("category_id") && product_info["category_id"].is_number_integer()) {
            category_id = product_info["category_id"].get<long>();
        } else {
            std::string category_name = escapeSQLString(product_info["category"].get<std::string>());
            std::string category_sql = "SELECT category_id FROM categories WHERE name = '" +
                                       category_name + "' AND status = 'active' LIMIT 1";
            json category_result = executeQuery(category_sql);
            if (!category_result["success"].get<bool>() || category_result["data"].empty()) {
                return createErrorResponse("指定的分类不存在", Constants::VALIDATION_ERROR_CODE);
            }
            category_id = category_result["data"][0]["category_id"].get<long>();
        }

        if (category_id <= 0) {
            return createErrorResponse("无效的分类ID", Constants::VALIDATION_ERROR_CODE);
        }

        std::string sql = "INSERT INTO products (name, description, category_id, price, stock_quantity, "
                         "status, created_at, updated_at) VALUES ('" + name + "', '" + 
                         description + "', " + std::to_string(category_id) + ", " + 
                         std::to_string(price) + ", " + std::to_string(stock) + ", 'active', NOW(), NOW())";
        
        json result = executeQuery(sql);
        if (result["success"].get<bool>()) {
            long product_id = result["data"]["insert_id"].get<long>();
            
            json response_data;
            response_data["product_id"] = product_id;
            response_data["name"] = product_info["name"];
            response_data["price"] = price;
            response_data["stock"] = stock;
            response_data["category_id"] = category_id;
            if (product_info.contains("category")) {
                response_data["category"] = product_info["category"];
            }
            
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

json ProductService::updateProduct(long product_id, const json& update_info) {
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
        
        if (update_info.contains("stock") && update_info["stock"].is_number_integer()) {
            int stock = update_info["stock"].get<int>();
            if (stock < 0 || stock > Constants::MAX_PRODUCT_QUANTITY) {
                return createErrorResponse("库存数量必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
            }
            update_fields.push_back("stock_quantity = " + std::to_string(stock));
        } else if (update_info.contains("stock_quantity") && update_info["stock_quantity"].is_number_integer()) {
            int stock = update_info["stock_quantity"].get<int>();
            if (stock < 0 || stock > Constants::MAX_PRODUCT_QUANTITY) {
                return createErrorResponse("库存数量必须在有效范围内", Constants::VALIDATION_ERROR_CODE);
            }
            update_fields.push_back("stock_quantity = " + std::to_string(stock));
        }

        if (update_info.contains("category_id") && update_info["category_id"].is_number_integer()) {
            long category_id = update_info["category_id"].get<long>();
            if (category_id <= 0) {
                return createErrorResponse("分类ID必须为正整数", Constants::VALIDATION_ERROR_CODE);
            }
            update_fields.push_back("category_id = " + std::to_string(category_id));
        } else if (update_info.contains("category") && update_info["category"].is_string()) {
            std::string category = update_info["category"].get<std::string>();
            if (category.empty()) {
                return createErrorResponse("商品分类不能为空", Constants::VALIDATION_ERROR_CODE);
            }
            std::string category_sql = "SELECT category_id FROM categories WHERE name = '" +
                                       escapeSQLString(category) + "' AND status = 'active' LIMIT 1";
            json category_result = executeQuery(category_sql);
            if (!category_result["success"].get<bool>() || category_result["data"].empty()) {
                return createErrorResponse("指定的分类不存在", Constants::VALIDATION_ERROR_CODE);
            }
            long category_id = category_result["data"][0]["category_id"].get<long>();
            update_fields.push_back("category_id = " + std::to_string(category_id));
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
        sql += " WHERE product_id = " + std::to_string(product_id);
        
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

json ProductService::deleteProduct(long product_id) {
    logInfo("删除商品请求，商品ID: " + std::to_string(product_id));
    
    if (!isProductExists(product_id)) {
        return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    std::string sql = "UPDATE products SET status = 'deleted', updated_at = NOW() "
               "WHERE product_id = " + std::to_string(product_id);
    
    json result = executeQuery(sql);
    if (result["success"].get<bool>()) {
        logInfo("商品删除成功，商品ID: " + std::to_string(product_id));
        return createSuccessResponse(json::object(), "商品删除成功");
    }
    
    return result;
}

json ProductService::getProductDetail(long product_id) {
    logDebug("获取商品详情，商品ID: " + std::to_string(product_id));
    
    json product_info = getProductById(product_id);
    if (product_info.empty()) {
        return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    return createSuccessResponse(product_info);
}

json ProductService::getProductList(const std::string& category, int page, int page_size) {
    logDebug("获取商品列表，分类: [" + category + "], 页码: " + std::to_string(page) + 
            ", 页大小: " + std::to_string(page_size));
    
    std::pair<int, int> validation_result = validatePaginationParams(page, page_size);
    int validated_page = validation_result.first;
    int validated_page_size = validation_result.second;
    
    try {
        std::string where_clause = "WHERE status = 'active'";
        
        // 处理分类筛选
        if (category != "all" && !category.empty() && category != "0") {
            // 如果category是数字，直接用作category_id
            if (std::all_of(category.begin(), category.end(), ::isdigit)) {
                where_clause += " AND category_id = " + category;
                logDebug("按分类ID筛选: " + category);
            } else {
                // 如果是分类名称，先查找分类ID
                std::string category_sql = "SELECT category_id FROM categories WHERE name = '" + 
                                         escapeSQLString(category) + "' AND status = 'active'";
                json category_result = executeQuery(category_sql);
                
                if (category_result["success"].get<bool>() && 
                    !category_result["data"].empty()) {
                    long category_id = category_result["data"][0]["category_id"].get<long>();
                    where_clause += " AND category_id = " + std::to_string(category_id);
                    logDebug("按分类名称筛选: " + category + " -> ID: " + std::to_string(category_id));
                } else {
                    // 分类不存在，返回空结果
                    logInfo("分类不存在: " + category);
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
        } else {
            logDebug("显示所有分类商品");
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

json ProductService::searchProducts(const std::string& keyword, int page, int page_size, 
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

json ProductService::getCategories() {
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

json ProductService::getCategoryProducts(const std::string& category, int page, int page_size, const std::string& sort_by) {
    logDebug("获取分类商品，分类: " + category);
    
    if (category.empty()) {
        return createErrorResponse("分类名称不能为空", Constants::VALIDATION_ERROR_CODE);
    }
    
    return getProductList(category, page, page_size);
}

json ProductService::updateStock(long product_id, int quantity, const std::string& operation) {
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

json ProductService::checkStock(long product_id) {
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

json ProductService::getLowStockProducts(int threshold) {
    logDebug("获取库存概览，低库存阈值: " + std::to_string(threshold));
    if (threshold < 0) {
        threshold = 0;
    }

    const std::string& id_column = getProductIdColumnName();
    const std::string& stock_column = getProductStockColumnName();
    const std::string& category_column = getProductCategoryColumnName();
    const std::string& status_column = getProductStatusColumnName();
    const std::string& image_column = getProductImageColumnName();
    const std::string& created_column = getProductCreatedAtColumnName();
    const std::string& updated_column = getProductUpdatedAtColumnName();

    std::vector<std::string> select_fields = {
        aliasColumn(qualifyProductColumn("", id_column), "product_id"),
        "name",
        aliasColumn(qualifyProductColumn("", stock_column), "stock")
    };

    if (hasColumn("products", "price")) {
        select_fields.push_back("price");
    } else {
        select_fields.push_back(aliasColumn("", "price"));
    }

    select_fields.push_back(aliasColumn(qualifyProductColumn("", category_column), "category"));
    select_fields.push_back(aliasColumn(qualifyProductColumn("", status_column), "status"));
    select_fields.push_back(aliasColumn(qualifyProductColumn("", image_column), "main_image"));
    select_fields.push_back(aliasColumn(created_column.empty() ? std::string() : qualifyProductColumn("", created_column), "created_at"));
    select_fields.push_back(aliasColumn(updated_column.empty() ? std::string() : qualifyProductColumn("", updated_column), "updated_at"));

    const std::string stock_expr = qualifyProductColumn("", stock_column);
    std::string low_stock_expr = "0";
    if (!stock_expr.empty()) {
        low_stock_expr = "CASE WHEN " + stock_expr + " <= " + std::to_string(threshold) + " THEN 1 ELSE 0 END";
    }
    select_fields.push_back(low_stock_expr + " AS is_low_stock");

    std::string where_clause = "WHERE 1=1";
    if (!status_column.empty()) {
        where_clause += " AND " + status_column + " <> 'deleted'";
    }

    std::string sql = "SELECT " + joinColumns(select_fields) + " FROM products " + where_clause +
                      " ORDER BY is_low_stock DESC, stock ASC";

    json query_result = executeQuery(sql);
    if (!query_result["success"].get<bool>()) {
        return query_result;
    }

    json products = query_result["data"];
    int low_stock_count = 0;
    if (products.is_array()) {
        for (auto& item : products) {
            bool is_low = false;
            if (item.contains("is_low_stock")) {
                if (item["is_low_stock"].is_boolean()) {
                    is_low = item["is_low_stock"].get<bool>();
                } else if (item["is_low_stock"].is_number_integer()) {
                    is_low = item["is_low_stock"].get<int>() != 0;
                }
            }
            if (!is_low && item.contains("stock")) {
                try {
                    const int stock_val = item["stock"].is_number_integer() ? item["stock"].get<int>()
                                             : std::stoi(item["stock"].get<std::string>());
                    is_low = stock_val <= threshold;
                } catch (...) {
                    is_low = false;
                }
            }
            item["is_low_stock"] = is_low;
            if (is_low) {
                low_stock_count++;
            }
        }
    }

    json response_data;
    response_data["products"] = products;
    response_data["threshold"] = threshold;
    response_data["low_stock_count"] = low_stock_count;
    response_data["total"] = products.is_array() ? static_cast<int>(products.size()) : 0;

    return createSuccessResponse(response_data, "获取库存成功");
}

// ==================== 限购管理功能 ====================

/**
 * 设置商品限购规则
 * @param product_id 商品ID
 * @param limit 限购数量(0表示不限购)
 * @param period 限购周期: total(总限购), daily(每日), weekly(每周), monthly(每月)
 */
json ProductService::setPurchaseLimit(long product_id, int limit, const std::string& period) {
    logDebug("设置商品限购，商品ID: " + std::to_string(product_id) + 
             ", 限购数量: " + std::to_string(limit) + ", 周期: " + period);
    
    // 检查商品是否存在
    if (!isProductExists(product_id)) {
        return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 验证限购数量
    if (limit < 0) {
        return createErrorResponse("限购数量不能为负数", Constants::VALIDATION_ERROR_CODE);
    }
    
    // 验证限购周期
    if (period != "total" && period != "daily" && period != "weekly" && period != "monthly") {
        return createErrorResponse("无效的限购周期，可选值: total, daily, weekly, monthly", 
                                 Constants::VALIDATION_ERROR_CODE);
    }
    
    // 更新商品限购设置
    std::string sql = "UPDATE products SET purchase_limit = " + std::to_string(limit) + 
                     ", purchase_limit_period = '" + period + "' " +
                     "WHERE " + getProductIdColumnName() + " = " + std::to_string(product_id);
    
    json result = executeQuery(sql);
    if (!result["success"].get<bool>()) {
        return result;
    }
    
    json response_data;
    response_data["product_id"] = product_id;
    response_data["purchase_limit"] = limit;
    response_data["purchase_limit_period"] = period;
    response_data["message"] = limit == 0 ? "已取消限购" : "限购设置成功";
    
    return createSuccessResponse(response_data, "设置限购成功");
}

/**
 * 检查用户是否可以购买指定数量的商品
 * @param user_id 用户ID
 * @param product_id 商品ID
 * @param quantity 购买数量
 */
json ProductService::checkPurchaseLimit(long user_id, long product_id, int quantity) {
    logDebug("检查限购，用户ID: " + std::to_string(user_id) + 
             ", 商品ID: " + std::to_string(product_id) + 
             ", 数量: " + std::to_string(quantity));
    
    // 调用存储过程检查限购
    std::string sql = "CALL check_user_purchase_limit(" + 
                     std::to_string(user_id) + ", " +
                     std::to_string(product_id) + ", " +
                     std::to_string(quantity) + ", " +
                     "@can_purchase, @purchased_count, @limit_count, @limit_period)";
    
    json call_result = executeQuery(sql);
    if (!call_result["success"].get<bool>()) {
        return call_result;
    }
    
    // 获取输出参数
    std::string query_sql = "SELECT @can_purchase AS can_purchase, "
                           "@purchased_count AS purchased_count, "
                           "@limit_count AS limit_count, "
                           "@limit_period AS limit_period";
    
    json query_result = executeQuery(query_sql);
    if (!query_result["success"].get<bool>()) {
        return query_result;
    }
    
    json data = query_result["data"];
    if (!data.is_array() || data.empty()) {
        return createErrorResponse("检查限购失败", Constants::VALIDATION_ERROR_CODE);
    }
    
    json result_row = data[0];
    bool can_purchase = result_row["can_purchase"].get<int>() != 0;
    int purchased_count = result_row["purchased_count"].get<int>();
    int limit_count = result_row["limit_count"].get<int>();
    std::string limit_period = result_row["limit_period"].get<std::string>();
    
    json response_data;
    response_data["can_purchase"] = can_purchase;
    response_data["user_id"] = user_id;
    response_data["product_id"] = product_id;
    response_data["request_quantity"] = quantity;
    response_data["purchased_count"] = purchased_count;
    response_data["limit_count"] = limit_count;
    response_data["limit_period"] = limit_period;
    response_data["remaining_quota"] = limit_count - purchased_count;
    
    if (!can_purchase) {
        std::string period_text;
        if (limit_period == "daily") period_text = "今日";
        else if (limit_period == "weekly") period_text = "本周";
        else if (limit_period == "monthly") period_text = "本月";
        else period_text = "总计";
        
        std::string message = "购买失败：该商品限购 " + std::to_string(limit_count) + " 件/" + period_text +
                            "，您" + period_text + "已购买 " + std::to_string(purchased_count) + " 件，" +
                            "本次购买 " + std::to_string(quantity) + " 件将超出限购";
        return createErrorResponse(message, Constants::PURCHASE_LIMIT_EXCEEDED);
    }
    
    return createSuccessResponse(response_data, "可以购买");
}

/**
 * 获取用户购买历史记录
 * @param user_id 用户ID
 * @param product_id 商品ID
 * @param period 统计周期: all(全部), daily(今日), weekly(本周), monthly(本月)
 */
json ProductService::getUserPurchaseHistory(long user_id, long product_id, const std::string& period) {
    logDebug("获取用户购买历史，用户ID: " + std::to_string(user_id) + 
             ", 商品ID: " + std::to_string(product_id) + ", 周期: " + period);
    
    std::string time_condition;
    if (period == "daily") {
        time_condition = " AND DATE(purchase_time) = CURDATE()";
    } else if (period == "weekly") {
        time_condition = " AND YEARWEEK(purchase_time, 1) = YEARWEEK(CURDATE(), 1)";
    } else if (period == "monthly") {
        time_condition = " AND DATE_FORMAT(purchase_time, '%Y-%m') = DATE_FORMAT(CURDATE(), '%Y-%m')";
    } else {
        time_condition = ""; // all - 不限制时间
    }
    
    std::string sql = "SELECT upr.record_id, upr.product_id, upr.quantity, "
                     "upr.order_id, upr.purchase_time, upr.status, "
                     "p.name AS product_name, p.price "
                     "FROM user_purchase_records upr "
                     "LEFT JOIN products p ON upr.product_id = p." + getProductIdColumnName() + " "
                     "WHERE upr.user_id = " + std::to_string(user_id);
    
    if (product_id > 0) {
        sql += " AND upr.product_id = " + std::to_string(product_id);
    }
    
    sql += time_condition;
    sql += " ORDER BY upr.purchase_time DESC";
    
    json query_result = executeQuery(sql);
    if (!query_result["success"].get<bool>()) {
        return query_result;
    }
    
    json records = query_result["data"];
    
    // 统计有效购买数量
    int total_valid_quantity = 0;
    if (records.is_array()) {
        for (const auto& record : records) {
            if (record["status"].get<std::string>() == "valid") {
                total_valid_quantity += record["quantity"].get<int>();
            }
        }
    }
    
    json response_data;
    response_data["user_id"] = user_id;
    response_data["product_id"] = product_id;
    response_data["period"] = period;
    response_data["records"] = records;
    response_data["total_records"] = records.is_array() ? records.size() : 0;
    response_data["total_valid_quantity"] = total_valid_quantity;
    
    return createSuccessResponse(response_data, "获取购买历史成功");
}

