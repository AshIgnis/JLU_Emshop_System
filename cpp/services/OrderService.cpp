/**
 * @file OrderService.cpp
 * @brief 订单服务类实现
 * @date 2025-01-13
 */

#include "OrderService.h"


    const std::string& OrderService::getOrderIdColumnName() const {
        static const std::string column = [this]() -> std::string {
            if (hasColumn("orders", "order_id")) return "order_id";
            if (hasColumn("orders", "id")) return "id";
            return "order_id";
        }();
        return column;
    }

    const std::string& OrderService::getOrderNoColumnName() const {
        static const std::string column = [this]() -> std::string {
            if (hasColumn("orders", "order_no")) return "order_no";
            return std::string();
        }();
        return column;
    }

    const std::string& OrderService::getOrderCreatedAtColumnName() const {
        static const std::string column = [this]() -> std::string {
            if (hasColumn("orders", "created_at")) return "created_at";
            if (hasColumn("orders", "create_time")) return "create_time";
            return std::string();
        }();
        return column;
    }

    const std::string& OrderService::getOrderUpdatedAtColumnName() const {
        static const std::string column = [this]() -> std::string {
            if (hasColumn("orders", "updated_at")) return "updated_at";
            if (hasColumn("orders", "update_time")) return "update_time";
            return std::string();
        }();
        return column;
    }

    const std::string& OrderService::getUsersPrimaryKeyColumn() const {
        static const std::string column = [this]() -> std::string {
            if (hasColumn("users", "user_id")) return "user_id";
            if (hasColumn("users", "id")) return "id";
            return "user_id";
        }();
        return column;
    }

bool OrderService::orderHasColumn(const std::string& column) const {
        return hasColumn("orders", column);
    }
    
    // 生成订单号
std::string OrderService::generateOrderNo() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << "EM" << std::put_time(std::localtime(&time_t), "%Y%m%d%H%M%S") 
           << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    OrderService::OrderService() : BaseService() {
        logInfo("订单服务初始化完成");
    }
    
std::string OrderService::getServiceName() const {
        return "OrderService";
    }
    
    // 从购物车创建订单
json OrderService::createOrderFromCart(long user_id, long address_id, const std::string& coupon_code, const std::string& remark) {
        logInfo("从购物车创建订单，用户ID: " + std::to_string(user_id));
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (user_id <= 0 || address_id <= 0) {
            return createErrorResponse("无效的用户ID或地址ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 开启事务
            executeQuery("BEGIN");
            bool needRollback = true; // 若提前return会在catch中ROLLBACK
            // 获取购物车内容
            std::string cart_sql = "SELECT c.product_id, c.quantity, c.selected, p.name, p.price, "
                                  "(c.quantity * p.price) as subtotal "
                                  "FROM cart c JOIN products p ON c.product_id = p.product_id "
                                  "WHERE c.user_id = " + std::to_string(user_id) + 
                                  " AND c.selected = 1 AND p.status = 'active'";
            
            json cart_result = executeQuery(cart_sql);
            if (!cart_result["success"].get<bool>() || cart_result["data"].empty()) {
                return createErrorResponse("购物车为空或商品不可用", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 计算订单总金额并收集 product_id -> quantity
            double total_amount = 0.0;
            std::vector<long> productIds; productIds.reserve(cart_result["data"].size());
            std::unordered_map<long,int> quantityMap;
            for (const auto& item : cart_result["data"]) {
                long pid = item["product_id"].get<long>();
                int qty = item["quantity"].get<int>();
                total_amount += item["subtotal"].get<double>();
                productIds.push_back(pid);
                quantityMap[pid] += qty;
            }
            // 校验库存：一次性查询当前库存（使用FOR UPDATE加锁）
            if (!productIds.empty()) {
                std::ostringstream oss; 
                oss << "SELECT p.product_id, p.stock_quantity, p.name, p.status FROM products p WHERE p.product_id IN (";
                for (size_t i=0;i<productIds.size();++i) { if (i) oss << ","; oss << productIds[i]; }
                oss << ") FOR UPDATE";  // 悲观锁：锁定这些行直到事务结束
                
                json stock_result = executeQuery(oss.str());
                if (!stock_result["success"].get<bool>()) {
                    return createErrorResponse("库存查询失败", Constants::DATABASE_ERROR_CODE);
                }
                
                // 建立库存映射和商品名称映射
                std::unordered_map<long,int> stockMap;
                std::unordered_map<long,std::string> nameMap;
                std::unordered_map<long,std::string> statusMap;
                
                for (auto &row : stock_result["data"]) {
                    long pid = row["product_id"].get<long>();
                    stockMap[pid] = row["stock_quantity"].get<int>();
                    if (row.contains("name") && row["name"].is_string()) {
                        nameMap[pid] = row["name"].get<std::string>();
                    }
                    if (row.contains("status") && row["status"].is_string()) {
                        statusMap[pid] = row["status"].get<std::string>();
                    }
                }
                
                // 逐个检查商品
                for (auto &kv : quantityMap) {
                    long pid = kv.first; 
                    int need = kv.second; 
                    int have = stockMap.count(pid) ? stockMap[pid] : -1;
                    std::string pname = nameMap.count(pid) ? nameMap[pid] : ("商品ID:" + std::to_string(pid));
                    std::string pstatus = statusMap.count(pid) ? statusMap[pid] : "unknown";
                    
                    if (have < 0) {
                        return createErrorResponse("商品「" + pname + "」不存在或已下架", Constants::VALIDATION_ERROR_CODE);
                    }
                    
                    if (pstatus != "active") {
                        return createErrorResponse("商品「" + pname + "」已下架，无法购买", Constants::VALIDATION_ERROR_CODE);
                    }
                    
                    if (have == 0) {
                        return createErrorResponse("很抱歉，商品「" + pname + "」已售罄（库存为0）", Constants::VALIDATION_ERROR_CODE);
                    }
                    
                    if (have < need) {
                        return createErrorResponse("商品「" + pname + "」库存不足，需要 " + std::to_string(need) + 
                                                 " 件，但仅剩 " + std::to_string(have) + " 件", Constants::VALIDATION_ERROR_CODE);
                    }
                }
            }

            // 收货地址快照
            std::string shipping_address;
            {
                std::string addr_sql = "SELECT receiver_name, receiver_phone, province, city, district, detail_address "
                                       "FROM user_addresses WHERE address_id = " + std::to_string(address_id) +
                                       " AND user_id = " + std::to_string(user_id) + " LIMIT 1";
                json addr_result = executeQuery(addr_sql);
                if (!addr_result["success"].get<bool>() || addr_result["data"].empty()) {
                    return createErrorResponse("地址不存在或不属于该用户", Constants::VALIDATION_ERROR_CODE);
                }
                const auto &a = addr_result["data"][0];
                // 存为易读的文本，便于前端直接展示
                std::ostringstream oss;
                oss << ((a.contains("receiver_name") && a["receiver_name"].is_string()) ? a["receiver_name"].get<std::string>() : std::string())
                    << " "
                    << ((a.contains("receiver_phone") && a["receiver_phone"].is_string()) ? a["receiver_phone"].get<std::string>() : std::string())
                    << " | "
                    << ((a.contains("province") && a["province"].is_string()) ? a["province"].get<std::string>() : std::string())
                    << ((a.contains("city") && a["city"].is_string()) ? a["city"].get<std::string>() : std::string())
                    << ((a.contains("district") && a["district"].is_string()) ? a["district"].get<std::string>() : std::string())
                    << " "
                    << ((a.contains("detail_address") && a["detail_address"].is_string()) ? a["detail_address"].get<std::string>() : std::string());
                shipping_address = oss.str();
            }

            // 优惠券应用（验证用户拥有且未使用的优惠券）
            double discount_amount = 0.0;
            long user_coupon_id = 0;  // 记录user_coupons表的ID
            long coupon_id_used = 0;   // 记录优惠券ID
            
            if (!coupon_code.empty()) {
                // 1. 首先查询优惠券基本信息
                std::string coup_sql = "SELECT coupon_id, code, type, value, min_amount, start_time, end_time, status "
                                       "FROM coupons WHERE code = '" + escapeSQLString(coupon_code) + "' LIMIT 1";
                json coup_result = executeQuery(coup_sql);
                
                if (!coup_result["success"].get<bool>() || coup_result["data"].empty()) {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("优惠券不存在: " + coupon_code, Constants::VALIDATION_ERROR_CODE);
                }
                
                auto c = coup_result["data"][0];
                
                // 检查优惠券状态
                std::string coupon_status = c.contains("status") && c["status"].is_string() ? 
                                          c["status"].get<std::string>() : "inactive";
                if (coupon_status != "active") {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("优惠券已失效", Constants::VALIDATION_ERROR_CODE);
                }
                
                // 获取coupon_id
                if (c.contains("coupon_id")) {
                    if (c["coupon_id"].is_number_integer()) {
                        coupon_id_used = c["coupon_id"].get<long>();
                    } else if (c["coupon_id"].is_string()) {
                        try { coupon_id_used = std::stol(c["coupon_id"].get<std::string>()); } catch(...) {}
                    }
                }
                
                if (coupon_id_used == 0) {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("优惠券ID无效", Constants::VALIDATION_ERROR_CODE);
                }
                
                // 2. 验证用户是否拥有该优惠券且未使用
                std::string user_coupon_sql = "SELECT id, status FROM user_coupons "
                                             "WHERE user_id = " + std::to_string(user_id) + 
                                             " AND coupon_id = " + std::to_string(coupon_id_used) + 
                                             " AND status = 'unused' LIMIT 1";
                json user_coupon_result = executeQuery(user_coupon_sql);
                
                if (!user_coupon_result["success"].get<bool>() || user_coupon_result["data"].empty()) {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("您没有该优惠券或优惠券已使用", Constants::VALIDATION_ERROR_CODE);
                }
                
                auto uc = user_coupon_result["data"][0];
                if (uc.contains("id")) {
                    if (uc["id"].is_number_integer()) {
                        user_coupon_id = uc["id"].get<long>();
                    } else if (uc["id"].is_string()) {
                        try { user_coupon_id = std::stol(uc["id"].get<std::string>()); } catch(...) {}
                    }
                }
                
                if (user_coupon_id == 0) {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("用户优惠券记录无效", Constants::VALIDATION_ERROR_CODE);
                }
                
                // 3. 计算折扣金额
                std::string ctype = c.contains("type") && c["type"].is_string() ? c["type"].get<std::string>() : "";
                double cval = 0.0;
                if (c.contains("value")) {
                    if (c["value"].is_number()) cval = c["value"].get<double>();
                    else if (c["value"].is_string()) {
                        try { cval = std::stod(c["value"].get<std::string>()); } catch (...) {}
                    }
                }
                double minAmt = 0.0;
                if (c.contains("min_amount")) {
                    if (c["min_amount"].is_number()) minAmt = c["min_amount"].get<double>();
                    else if (c["min_amount"].is_string()) {
                        try { minAmt = std::stod(c["min_amount"].get<std::string>()); } catch (...) {}
                    }
                }
                
                if (total_amount < minAmt) {
                    executeQuery("ROLLBACK");
                    return createErrorResponse("订单金额不满足优惠券使用条件(最低:" + 
                                             std::to_string(minAmt) + "元)", Constants::VALIDATION_ERROR_CODE);
                }
                
                std::string t = StringUtils::toLower(ctype);
                if (t == "percentage" || t == "percent" || t == "discount") {
                    discount_amount = total_amount * (cval / 100.0);
                } else {
                    // 默认为固定金额
                    discount_amount = cval;
                }
                if (discount_amount < 0) discount_amount = 0;
                if (discount_amount > total_amount) discount_amount = total_amount;
                
                logInfo("优惠券验证通过: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", 折扣=" + std::to_string(discount_amount));
            }
            
            double final_amount = total_amount - discount_amount;
            
            // 生成订单号
            std::string order_no = generateOrderNo();
            
            // 创建订单
            std::string order_sql = "INSERT INTO orders (order_no, user_id, total_amount, discount_amount, final_amount, "
                                   "status, payment_status, shipping_address, remark) VALUES ('" +
                                   order_no + "', " + std::to_string(user_id) + ", " +
                                   std::to_string(total_amount) + ", " + std::to_string(discount_amount) + ", " + std::to_string(final_amount) + 
                                   ", 'pending', 'unpaid', JSON_QUOTE('" + escapeSQLString(shipping_address) + "'), '" + escapeSQLString(remark) + "')";
            
            json order_result = executeQuery(order_sql);
            if (!order_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
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

            // 创建订单时不扣减库存,只记录库存信息(库存将在支付时扣减)
            json stock_changes = json::array();
            for (auto &kv : quantityMap) {
                long pid = kv.first; int used = kv.second;
                
                // 查询当前库存
                std::string qsql = "SELECT stock_quantity FROM products WHERE product_id = " + std::to_string(pid) + " LIMIT 1";
                json qres = executeQuery(qsql);
                int remain = -1;
                if (qres["success"].get<bool>() && !qres["data"].empty()) {
                    auto row = qres["data"][0];
                    if (row.contains("stock_quantity") && row["stock_quantity"].is_number_integer()) {
                        remain = row["stock_quantity"].get<int>();
                    }
                }
                json entry; entry["product_id"] = pid; entry["quantity_reserved"] = used; entry["current_stock"] = remain;
                stock_changes.push_back(entry);
            }
            
            // 标记优惠券为已使用
            if (user_coupon_id > 0) {
                std::string mark_used_sql = "UPDATE user_coupons SET status = 'used', " 
                                          "order_id = " + std::to_string(order_id) + 
                                          ", used_at = NOW() WHERE id = " + std::to_string(user_coupon_id);
                json mark_result = executeQuery(mark_used_sql);
                if (!mark_result["success"].get<bool>()) {
                    executeQuery("ROLLBACK");
                    logError("标记优惠券失败: user_coupon_id=" + std::to_string(user_coupon_id));
                    return createErrorResponse("标记优惠券失败", Constants::DATABASE_ERROR_CODE);
                }
                logInfo("优惠券标记为已使用: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", order_id=" + std::to_string(order_id));
            }
            
            // 清空购物车
            std::string clear_cart_sql = "DELETE FROM cart WHERE user_id = " + std::to_string(user_id);
            executeQuery(clear_cart_sql);
            
            json response_data;
            response_data["order_id"] = order_id;
            response_data["order_no"] = order_no;
            response_data["total_amount"] = total_amount;
            response_data["discount_amount"] = discount_amount;
            response_data["final_amount"] = final_amount;
            response_data["shipping_address"] = shipping_address;
            response_data["item_count"] = cart_result["data"].size();
            response_data["stock_changes"] = stock_changes;
            if (user_coupon_id > 0) {
                response_data["coupon_used"] = true;
                response_data["coupon_id"] = coupon_id_used;
            }
            
            // 提交事务
            executeQuery("COMMIT");
            needRollback = false;
            logInfo("订单创建成功，订单ID: " + std::to_string(order_id));
            return createSuccessResponse(response_data, "订单创建成功");
            
        } catch (const std::exception& e) {
            try { executeQuery("ROLLBACK"); } catch(...) {}
            return createErrorResponse("创建订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 直接购买创建订单（不依赖购物车，或用于仅选中单个条目下单）
json OrderService::createOrderDirect(long user_id, long product_id, int quantity, long address_id, const std::string& coupon_code, const std::string& remark) {
        logInfo("直接创建订单，用户ID: " + std::to_string(user_id) + ", 商品ID: " + std::to_string(product_id) + ", 数量: " + std::to_string(quantity));
        std::lock_guard<std::mutex> lock(order_mutex_);
        if (user_id <= 0 || address_id <= 0 || product_id <= 0 || quantity <= 0) {
            return createErrorResponse("无效的下单参数", Constants::VALIDATION_ERROR_CODE);
        }
        try {
            // 查询商品信息（加锁防止并发）
            std::string prod_sql = "SELECT product_id, name, price, status, stock_quantity FROM products WHERE product_id = " + 
                                  std::to_string(product_id) + " FOR UPDATE";
            json prod_result = executeQuery(prod_sql);
            
            if (!prod_result["success"].get<bool>() || prod_result["data"].empty()) {
                return createErrorResponse("商品不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            auto p = prod_result["data"][0];
            std::string product_status = p.contains("status") && p["status"].is_string() ? 
                                        p["status"].get<std::string>() : "inactive";
            std::string product_name = p.contains("name") && p["name"].is_string() ? 
                                      p["name"].get<std::string>() : "未知商品";
            
            if (product_status != "active") {
                return createErrorResponse("商品「" + product_name + "」已下架或不可售", Constants::VALIDATION_ERROR_CODE);
            }
            
            int current_stock = p.contains("stock_quantity") && p["stock_quantity"].is_number_integer() ? 
                               p["stock_quantity"].get<int>() : 0;
            
            // 明确提示库存为0的情况
            if (current_stock == 0) {
                return createErrorResponse("很抱歉，商品「" + product_name + "」已售罄（库存为0）", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (current_stock < quantity) {
                return createErrorResponse("商品「" + product_name + "」库存不足，需要 " + std::to_string(quantity) + 
                                         " 件，但仅剩 " + std::to_string(current_stock) + " 件", Constants::VALIDATION_ERROR_CODE);
            }
            double unit_price = 0.0;
            if (p.contains("price")) {
                if (p["price"].is_number()) unit_price = p["price"].get<double>();
                else if (p["price"].is_string()) { try { unit_price = std::stod(p["price"].get<std::string>()); } catch(...){} }
            }
            double subtotal = unit_price * quantity;

            // 地址快照（与购物车下单保持一致）
            std::string shipping_address;
            {
                std::string addr_sql = "SELECT receiver_name, receiver_phone, province, city, district, detail_address "
                                       "FROM user_addresses WHERE address_id = " + std::to_string(address_id) +
                                       " AND user_id = " + std::to_string(user_id) + " LIMIT 1";
                json addr_result = executeQuery(addr_sql);
                if (!addr_result["success"].get<bool>() || addr_result["data"].empty()) {
                    return createErrorResponse("地址不存在或不属于该用户", Constants::VALIDATION_ERROR_CODE);
                }
                const auto &a = addr_result["data"][0];
                std::ostringstream oss;
                oss << ((a.contains("receiver_name") && a["receiver_name"].is_string()) ? a["receiver_name"].get<std::string>() : std::string())
                    << " "
                    << ((a.contains("receiver_phone") && a["receiver_phone"].is_string()) ? a["receiver_phone"].get<std::string>() : std::string())
                    << " | "
                    << ((a.contains("province") && a["province"].is_string()) ? a["province"].get<std::string>() : std::string())
                    << ((a.contains("city") && a["city"].is_string()) ? a["city"].get<std::string>() : std::string())
                    << ((a.contains("district") && a["district"].is_string()) ? a["district"].get<std::string>() : std::string())
                    << " "
                    << ((a.contains("detail_address") && a["detail_address"].is_string()) ? a["detail_address"].get<std::string>() : std::string());
                shipping_address = oss.str();
            }

            // 优惠券应用（验证用户拥有且未使用的优惠券）
            double total_amount = subtotal;
            double discount_amount = 0.0;
            long user_coupon_id = 0;  // 记录user_coupons表的ID
            long coupon_id_used = 0;   // 记录优惠券ID
            
            if (!coupon_code.empty()) {
                // 1. 查询优惠券基本信息(验证存在和active状态)
                std::string coup_sql = "SELECT coupon_id, code, type, value, min_amount, start_time, end_time, status "
                                       "FROM coupons WHERE code = '" + escapeSQLString(coupon_code) + "' LIMIT 1";
                json coup_result = executeQuery(coup_sql);
                
                if (!coup_result["success"].get<bool>() || coup_result["data"].empty()) {
                    return createErrorResponse("优惠券不存在: " + coupon_code, Constants::VALIDATION_ERROR_CODE);
                }
                
                auto c = coup_result["data"][0];
                
                // 验证优惠券状态
                std::string coupon_status = c.contains("status") && c["status"].is_string() ? 
                                           c["status"].get<std::string>() : "inactive";
                if (coupon_status != "active") {
                    return createErrorResponse("优惠券不可用(状态: " + coupon_status + ")", Constants::VALIDATION_ERROR_CODE);
                }
                
                // 获取优惠券ID
                if (c.contains("coupon_id") && c["coupon_id"].is_number_integer()) {
                    coupon_id_used = c["coupon_id"].get<long>();
                } else {
                    return createErrorResponse("优惠券数据异常", Constants::DATABASE_ERROR_CODE);
                }
                
                // 2. 验证用户是否拥有该优惠券且未使用
                std::string user_coupon_sql = "SELECT id, status FROM user_coupons "
                                             "WHERE user_id = " + std::to_string(user_id) + 
                                             " AND coupon_id = " + std::to_string(coupon_id_used) + 
                                             " AND status = 'unused' LIMIT 1";
                json user_coupon_result = executeQuery(user_coupon_sql);
                
                if (!user_coupon_result["success"].get<bool>() || user_coupon_result["data"].empty()) {
                    return createErrorResponse("您没有该优惠券或优惠券已使用", Constants::VALIDATION_ERROR_CODE);
                }
                
                auto uc = user_coupon_result["data"][0];
                if (uc.contains("id") && uc["id"].is_number_integer()) {
                    user_coupon_id = uc["id"].get<long>();
                } else {
                    return createErrorResponse("优惠券记录异常", Constants::DATABASE_ERROR_CODE);
                }
                
                // 3. 验证订单金额满足最低要求
                double minAmt = 0.0;
                if (c.contains("min_amount")) {
                    if (c["min_amount"].is_number()) minAmt = c["min_amount"].get<double>();
                    else if (c["min_amount"].is_string()) { 
                        try { minAmt = std::stod(c["min_amount"].get<std::string>()); } 
                        catch(...){ minAmt = 0.0; }
                    }
                }
                
                if (total_amount < minAmt) {
                    return createErrorResponse("订单金额不满足优惠券使用条件(需满" + 
                                             std::to_string(minAmt) + "元)", Constants::VALIDATION_ERROR_CODE);
                }
                
                // 4. 计算折扣金额
                std::string ctype = c.contains("type") && c["type"].is_string() ? 
                                   c["type"].get<std::string>() : "";
                double cval = 0.0;
                if (c.contains("value")) {
                    if (c["value"].is_number()) cval = c["value"].get<double>();
                    else if (c["value"].is_string()) { 
                        try { cval = std::stod(c["value"].get<std::string>()); } 
                        catch(...){ cval = 0.0; }
                    }
                }
                
                std::string t = StringUtils::toLower(ctype);
                if (t == "percentage" || t == "percent" || t == "discount") {
                    discount_amount = total_amount * (cval / 100.0);
                } else {
                    discount_amount = cval;
                }
                
                if (discount_amount < 0) discount_amount = 0;
                if (discount_amount > total_amount) discount_amount = total_amount;
                
                logInfo("优惠券验证通过: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", coupon_id=" + std::to_string(coupon_id_used) + 
                       ", discount=" + std::to_string(discount_amount));
            }
            
            double final_amount = total_amount - discount_amount;

            // 生成订单号并插入
            std::string order_no = generateOrderNo();
            std::string order_sql = "INSERT INTO orders (order_no, user_id, total_amount, discount_amount, final_amount, status, payment_status, shipping_address, remark) VALUES ('" +
                                   order_no + "', " + std::to_string(user_id) + ", " +
                                   std::to_string(total_amount) + ", " + std::to_string(discount_amount) + ", " + std::to_string(final_amount) +
                                   ", 'pending', 'unpaid', JSON_QUOTE('" + escapeSQLString(shipping_address) + "'), '" + escapeSQLString(remark) + "')";
            executeQuery("BEGIN");
            json order_result = executeQuery(order_sql);
            if (!order_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return order_result;
            }
            long order_id = order_result["data"]["insert_id"].get<long>();

            // 插入订单明细
            std::string item_sql = "INSERT INTO order_items (order_id, product_id, product_name, price, quantity, subtotal) VALUES (" +
                                   std::to_string(order_id) + ", " +
                                   std::to_string(product_id) + ", '" + escapeSQLString(product_name) + "', " +
                                   std::to_string(unit_price) + ", " + std::to_string(quantity) + ", " + std::to_string(subtotal) + ")";
            executeQuery(item_sql);

            // 标记优惠券为已使用
            if (user_coupon_id > 0) {
                std::string mark_used_sql = "UPDATE user_coupons SET status = 'used', " 
                                          "order_id = " + std::to_string(order_id) + 
                                          ", used_at = NOW() WHERE id = " + std::to_string(user_coupon_id);
                json mark_result = executeQuery(mark_used_sql);
                if (!mark_result["success"].get<bool>()) {
                    executeQuery("ROLLBACK");
                    logError("标记优惠券失败: user_coupon_id=" + std::to_string(user_coupon_id));
                    return createErrorResponse("标记优惠券失败", Constants::DATABASE_ERROR_CODE);
                }
                logInfo("优惠券标记为已使用: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", order_id=" + std::to_string(order_id));
            }

            json response_data;
            response_data["order_id"] = order_id;
            response_data["order_no"] = order_no;
            response_data["total_amount"] = total_amount;
            response_data["discount_amount"] = discount_amount;
            response_data["final_amount"] = final_amount;
            response_data["shipping_address"] = shipping_address;
            response_data["item_count"] = 1;
            response_data["items"] = json::array({ {
                {"product_id", product_id}, {"product_name", product_name}, {"price", unit_price}, {"quantity", quantity}, {"subtotal", subtotal}
            } });
            if (user_coupon_id > 0) {
                response_data["coupon_used"] = true;
                response_data["coupon_id"] = coupon_id_used;
            }
            // 创建订单时不扣减库存,只查询当前库存(库存将在支付时扣减)
            int remaining_stock = -1;{
                std::string qsql = "SELECT stock_quantity FROM products WHERE product_id = " + std::to_string(product_id) + " LIMIT 1";
                json qres = executeQuery(qsql);
                if (qres["success"].get<bool>() && !qres["data"].empty()) {
                    auto row = qres["data"][0];
                    if (row.contains("stock_quantity") && row["stock_quantity"].is_number_integer()) remaining_stock = row["stock_quantity"].get<int>();
                }
            }
            json stock_changes = json::array();
            stock_changes.push_back({ {"product_id", product_id}, {"quantity_reserved", quantity}, {"current_stock", remaining_stock} });
            response_data["stock_changes"] = stock_changes;

            executeQuery("COMMIT");
            logInfo("直接订单创建成功(库存将在支付时扣减)，订单ID: " + std::to_string(order_id));
            return createSuccessResponse(response_data, "订单创建成功");
        } catch (const std::exception& e) {
            try { executeQuery("ROLLBACK"); } catch(...) {}
            return createErrorResponse("创建订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 获取用户订单列表
json OrderService::getUserOrders(long user_id) {
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
json OrderService::getOrderDetail(long order_id) {
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

    // 删除订单（仅允许已取消的订单）
json OrderService::deleteOrder(long order_id) {
        logInfo("删除订单，订单ID: " + std::to_string(order_id));
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        try {
            // 仅允许删除已取消的订单
            std::string sql = "DELETE FROM orders WHERE order_id = " + std::to_string(order_id) + " AND status = 'cancelled'";
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                long affected = 0;
                if (result["data"].is_object() && result["data"].contains("affected_rows")) {
                    affected = result["data"]["affected_rows"].get<long>();
                }
                if (affected > 0) {
                    json resp;
                    resp["order_id"] = order_id;
                    resp["deleted"] = true;
                    return createSuccessResponse(resp, "订单删除成功");
                } else {
                    return createErrorResponse("仅已取消的订单可删除或订单不存在", Constants::VALIDATION_ERROR_CODE);
                }
            }
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("删除订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 支付订单
json OrderService::payOrder(long order_id, const std::string& payment_method) {
        logInfo("支付订单，订单ID: " + std::to_string(order_id) + ", 支付方式: " + payment_method);
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 检查订单状态
            std::string check_sql = "SELECT status, payment_status FROM orders WHERE order_id = " + 
                                   std::to_string(order_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            std::string payment_status = check_result["data"][0]["payment_status"].get<std::string>();
            
            if (current_status != "pending" && current_status != "confirmed") {
                return createErrorResponse("订单状态不允许支付", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (payment_status == "paid") {
                return createErrorResponse("订单已支付", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 模拟支付处理（实际项目中这里会调用第三方支付接口）
            std::string transaction_id = generateTransactionId();
            
            // 开启事务
            executeQuery("BEGIN");
            
            // 更新订单状态
            std::string update_sql = "UPDATE orders SET status = 'paid', payment_status = 'paid', "
                                   "payment_method = '" + payment_method + "', "
                                   "paid_at = NOW(), updated_at = NOW() "
                                   "WHERE order_id = " + std::to_string(order_id);
            
            json result = executeQuery(update_sql);
            if (!result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return result;
            }
            
            // ========== 支付成功后创建购买记录（用于限购统计）==========
            // 查询订单明细
            std::string items_sql = "SELECT oi.product_id, oi.quantity, o.user_id "
                                   "FROM order_items oi "
                                   "JOIN orders o ON oi.order_id = o.order_id "
                                   "WHERE oi.order_id = " + std::to_string(order_id);
            logInfo("查询订单明细SQL: " + items_sql);
            json items_result = executeQuery(items_sql);
            
            // 记录查询结果
            logInfo("订单明细查询结果: success=" + std::to_string(items_result["success"].get<bool>()) + 
                   ", data_size=" + std::to_string(items_result["data"].size()));
            
            if (!items_result["success"].get<bool>()) {
                logError("查询订单明细失败: order_id=" + std::to_string(order_id));
                executeQuery("ROLLBACK");
                return createErrorResponse("查询订单明细失败", Constants::DATABASE_ERROR_CODE);
            }
            
            if (items_result["data"].empty()) {
                logError("订单明细为空: order_id=" + std::to_string(order_id));
                executeQuery("ROLLBACK");
                return createErrorResponse("订单明细为空", Constants::DATABASE_ERROR_CODE);
            }
            
            // 创建购买记录
            long user_id = items_result["data"][0]["user_id"].get<long>();
            logInfo("开始创建购买记录: user_id=" + std::to_string(user_id) + 
                   ", order_id=" + std::to_string(order_id));
            
            for (const auto& item : items_result["data"]) {
                long pid = item["product_id"].get<long>();
                int qty = item["quantity"].get<int>();
                
                std::string purchase_record_sql = 
                    "INSERT INTO user_purchase_records (user_id, product_id, quantity, order_id, status) VALUES (" +
                    std::to_string(user_id) + ", " +
                    std::to_string(pid) + ", " +
                    std::to_string(qty) + ", " +
                    std::to_string(order_id) + ", " +
                    "'valid')";
                
                logInfo("插入购买记录SQL: " + purchase_record_sql);
                json record_result = executeQuery(purchase_record_sql);
                
                if (!record_result["success"].get<bool>()) {
                    logError("创建购买记录失败: user_id=" + std::to_string(user_id) + 
                             ", product_id=" + std::to_string(pid) + 
                             ", error=" + (record_result.contains("message") ? record_result["message"].get<std::string>() : "unknown"));
                    executeQuery("ROLLBACK");
                    return createErrorResponse("创建购买记录失败", Constants::DATABASE_ERROR_CODE);
                } else {
                    logInfo("购买记录已创建: user_id=" + std::to_string(user_id) + 
                           ", product_id=" + std::to_string(pid) + 
                           ", quantity=" + std::to_string(qty) + 
                           ", order_id=" + std::to_string(order_id));
                }
            }
            
            // 提交事务
            executeQuery("COMMIT");
            
            json response_data;
            response_data["order_id"] = order_id;
            response_data["payment_method"] = payment_method;
            response_data["transaction_id"] = transaction_id;
            response_data["status"] = "paid";
            response_data["payment_status"] = "paid";
            
            logInfo("订单支付成功，订单ID: " + std::to_string(order_id));
            return createSuccessResponse(response_data, "支付成功");
        } catch (const std::exception& e) {
            return createErrorResponse("支付订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 生成交易ID
std::string OrderService::generateTransactionId() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << "TXN" << std::put_time(std::localtime(&time_t), "%Y%m%d%H%M%S") 
           << std::setfill('0') << std::setw(3) << ms.count()
           << std::setw(4) << (rand() % 10000);
        return ss.str();
    }
    
    // 发货订单
json OrderService::shipOrder(long order_id, const std::string& tracking_number, const std::string& shipping_method) {
        logInfo("发货订单，订单ID: " + std::to_string(order_id) + ", 快递单号: " + tracking_number);
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (tracking_number.empty()) {
            return createErrorResponse("快递单号不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 检查订单状态
            std::string check_sql = "SELECT status, payment_status FROM orders WHERE order_id = " + 
                                   std::to_string(order_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            std::string payment_status = check_result["data"][0]["payment_status"].get<std::string>();
            
            if (current_status != "paid") {
                return createErrorResponse("订单状态不允许发货", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (payment_status != "paid") {
                return createErrorResponse("订单未支付，不能发货", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新订单状态
            std::string update_sql = "UPDATE orders SET status = 'shipped', "
                                   "tracking_number = '" + tracking_number + "', "
                                   "shipping_method = '" + shipping_method + "', "
                                   "shipped_at = NOW(), updated_at = NOW() "
                                   "WHERE order_id = " + std::to_string(order_id);
            
            json result = executeQuery(update_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["order_id"] = order_id;
                response_data["tracking_number"] = tracking_number;
                response_data["shipping_method"] = shipping_method;
                response_data["status"] = "shipped";
                response_data["shipped_at"] = getCurrentTimestamp();
                
                logInfo("订单发货成功，订单ID: " + std::to_string(order_id));
                return createSuccessResponse(response_data, "发货成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("发货订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 确认收货
json OrderService::confirmDelivery(long order_id) {
        logInfo("确认收货，订单ID: " + std::to_string(order_id));
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            // 检查订单状态
            std::string check_sql = "SELECT status FROM orders WHERE order_id = " + std::to_string(order_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            
            if (current_status != "shipped" && current_status != "delivered") {
                return createErrorResponse("订单状态不允许确认收货", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新订单状态为已完成
            std::string update_sql = "UPDATE orders SET status = 'completed', "
                                   "delivered_at = NOW(), updated_at = NOW() "
                                   "WHERE order_id = " + std::to_string(order_id);
            
            json result = executeQuery(update_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["order_id"] = order_id;
                response_data["status"] = "completed";
                response_data["delivered_at"] = getCurrentTimestamp();
                
                logInfo("确认收货成功，订单ID: " + std::to_string(order_id));
                return createSuccessResponse(response_data, "确认收货成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("确认收货异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 申请退款(用户功能) - 改进版
json OrderService::requestRefund(long order_id, long user_id, const std::string& reason) {
        logInfo("用户申请退款，订单ID: " + std::to_string(order_id) + ", 用户ID: " + std::to_string(user_id) + ", 原因: " + reason);
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0 || user_id <= 0) {
            return createErrorResponse("无效的订单ID或用户ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        if (reason.empty()) {
            return createErrorResponse("退款原因不能为空", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 使用ConnectionGuard管理单一连接,确保事务在同一连接上执行
        try {
            ConnectionGuard conn(db_pool_);
            if (!conn.isValid()) {
                logError("数据库连接无效");
                return createErrorResponse("数据库连接失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 开启事务
            json begin_result = executeQueryWithConnection(conn.get(), "START TRANSACTION");
            if (!begin_result["success"].get<bool>()) {
                logError("开启事务失败");
                return createErrorResponse("数据库事务启动失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 检查订单状态和归属
            std::string check_sql = "SELECT status, payment_status, total_amount, user_id FROM orders WHERE order_id = " + 
                                   std::to_string(order_id) + " FOR UPDATE";
            json check_result = executeQueryWithConnection(conn.get(), check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            long order_user_id = check_result["data"][0]["user_id"].get<long>();
            if (order_user_id != user_id) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                return createErrorResponse("无权操作此订单", Constants::UNAUTHORIZED_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            std::string payment_status = "unpaid"; // 默认值
            if (check_result["data"][0].contains("payment_status") && !check_result["data"][0]["payment_status"].is_null()) {
                payment_status = check_result["data"][0]["payment_status"].get<std::string>();
            }
            double total_amount = check_result["data"][0]["total_amount"].get<double>();
            
            // 检查订单是否已支付(只要status是paid/shipped/delivered/completed之一,或payment_status是paid,就认为已支付)
            bool is_paid = (current_status == "paid" || current_status == "shipped" || 
                           current_status == "delivered" || current_status == "completed" || 
                           payment_status == "paid");
            
            if (!is_paid) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                return createErrorResponse("订单未支付，无法申请退款", Constants::VALIDATION_ERROR_CODE);
            }
            
            if (current_status == "cancelled" || current_status == "refunded" || current_status == "refunding") {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                return createErrorResponse("订单状态不允许申请退款", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 检查是否已有待审核的退款申请
            std::string check_refund_sql = "SELECT refund_id FROM refund_requests WHERE order_id = " + 
                                          std::to_string(order_id) + " AND status = 'pending'";
            json check_refund_result = executeQueryWithConnection(conn.get(), check_refund_sql);
            
            if (check_refund_result["success"].get<bool>() && !check_refund_result["data"].empty()) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                return createErrorResponse("该订单已有待审核的退款申请", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 检查是否已有被拒绝的退款申请
            std::string check_rejected_sql = "SELECT refund_id, admin_reply FROM refund_requests WHERE order_id = " + 
                                            std::to_string(order_id) + " AND status = 'rejected'";
            json check_rejected_result = executeQueryWithConnection(conn.get(), check_rejected_sql);
            
            if (check_rejected_result["success"].get<bool>() && !check_rejected_result["data"].empty()) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                std::string admin_reply = "";
                if (check_rejected_result["data"][0].contains("admin_reply") && 
                    !check_rejected_result["data"][0]["admin_reply"].is_null()) {
                    admin_reply = check_rejected_result["data"][0]["admin_reply"].get<std::string>();
                }
                std::string error_msg = "该订单的退款申请已被管理员拒绝，不能再次申请退款";
                if (!admin_reply.empty()) {
                    error_msg += "。拒绝原因: " + admin_reply;
                }
                return createErrorResponse(error_msg, Constants::VALIDATION_ERROR_CODE);
            }
            
            // 创建退款申请记录
            std::string insert_refund_sql = "INSERT INTO refund_requests (order_id, user_id, reason, refund_amount, status, created_at) "
                                           "VALUES (" + std::to_string(order_id) + ", " + std::to_string(user_id) + ", '" + 
                                           escapeSQLString(reason) + "', " + std::to_string(total_amount) + ", 'pending', NOW())";
            json insert_result = executeQueryWithConnection(conn.get(), insert_refund_sql);
            
            if (!insert_result["success"].get<bool>()) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                logError("创建退款申请失败: " + insert_result["message"].get<std::string>());
                return createErrorResponse("创建退款申请失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 更新订单状态为refunding
            std::string update_sql = "UPDATE orders SET status = 'refunding', updated_at = NOW() WHERE order_id = " + std::to_string(order_id);
            json update_result = executeQueryWithConnection(conn.get(), update_sql);
            
            if (!update_result["success"].get<bool>()) {
                executeQueryWithConnection(conn.get(), "ROLLBACK");
                std::string error_msg = "更新订单状态失败";
                if (update_result.contains("message") && !update_result["message"].is_null()) {
                    error_msg += ": " + update_result["message"].get<std::string>();
                }
                logError("更新订单状态失败，订单ID: " + std::to_string(order_id) + ", 错误: " + error_msg);
                return createErrorResponse(error_msg, Constants::DATABASE_ERROR_CODE);
            }
            
            // 获取退款申请ID
            long refund_id = 0;
            if (insert_result.contains("data") && insert_result["data"].contains("insert_id")) {
                refund_id = insert_result["data"]["insert_id"].get<long>();
            }
            
            // 提交事务
            json commit_result = executeQueryWithConnection(conn.get(), "COMMIT");
            
            if (!commit_result["success"].get<bool>()) {
                logError("提交事务失败");
                executeQueryWithConnection(conn.get(), "ROLLBACK");  // 尝试回滚
                return createErrorResponse("退款申请提交失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 事务提交成功后,在事务外创建通知(避免通知失败导致事务回滚)
            try {
                createNotification(user_id, "order_update", "退款申请已提交", 
                                 "您的订单 #" + std::to_string(order_id) + " 退款申请已提交，等待管理员审核", order_id);
            } catch (const std::exception& e) {
                logInfo("创建通知失败(不影响退款申请): " + std::string(e.what()));
            }
            
            logInfo("退款申请成功: refund_id=" + std::to_string(refund_id) + ", order_id=" + std::to_string(order_id));
            
            json response_data;
            response_data["refund_id"] = refund_id;
            response_data["order_id"] = order_id;
            response_data["status"] = "refunding";
            response_data["refund_amount"] = total_amount;
            response_data["refund_reason"] = reason;
            
            logInfo("退款申请已提交，订单ID: " + std::to_string(order_id) + ", 退款ID: " + std::to_string(refund_id));
            return createSuccessResponse(response_data, "退款申请已提交，等待管理员审核");
            
        } catch (const std::exception& e) {
            logError("申请退款异常: " + std::string(e.what()));
            return createErrorResponse("申请退款异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 取消订单
json OrderService::cancelOrder(long order_id, const std::string& reason) {
        logInfo("取消订单，订单ID: " + std::to_string(order_id));
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        try {
            // 开启事务
            executeQuery("BEGIN");
            
            // 检查订单状态
            std::string check_sql = "SELECT status, payment_status FROM orders WHERE order_id = " + 
                                   std::to_string(order_id) + " FOR UPDATE";
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            
            // 只允许pending和confirmed状态的订单取消
            if (current_status != "pending" && current_status != "confirmed") {
                executeQuery("ROLLBACK");
                return createErrorResponse("订单状态不允许取消", Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新订单状态
            std::string update_sql = "UPDATE orders SET status = 'cancelled', updated_at = NOW() "
                                   "WHERE order_id = " + std::to_string(order_id);
            json update_result = executeQuery(update_sql);
            
            if (!update_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return update_result;
            }
            
            // 恢复优惠券（如果订单使用了优惠券）⭐新增⭐
            std::string coupon_check_sql = "SELECT id, coupon_id FROM user_coupons "
                                          "WHERE order_id = " + std::to_string(order_id) + 
                                          " AND status = 'used' LIMIT 1";
            json coupon_check_result = executeQuery(coupon_check_sql);
            
            if (coupon_check_result["success"].get<bool>() && !coupon_check_result["data"].empty()) {
                auto coupon_data = coupon_check_result["data"][0];
                long user_coupon_id = coupon_data["id"].get<long>();
                long coupon_id = coupon_data["coupon_id"].get<long>();
                
                // 恢复优惠券状态为unused
                std::string restore_coupon_sql = "UPDATE user_coupons SET status = 'unused', "
                                                "order_id = NULL, used_at = NULL "
                                                "WHERE id = " + std::to_string(user_coupon_id);
                json restore_coupon_result = executeQuery(restore_coupon_sql);
                
                if (!restore_coupon_result["success"].get<bool>()) {
                    executeQuery("ROLLBACK");
                    logError("恢复优惠券失败: user_coupon_id=" + std::to_string(user_coupon_id));
                    return createErrorResponse("恢复优惠券失败", Constants::DATABASE_ERROR_CODE);
                }
                
                logInfo("优惠券已恢复: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", coupon_id=" + std::to_string(coupon_id));
            }
            
            // 返还库存 - 查询订单明细
            std::string items_sql = "SELECT product_id, quantity FROM order_items WHERE order_id = " + 
                                   std::to_string(order_id);
            json items_result = executeQuery(items_sql);
            
            if (items_result["success"].get<bool>() && !items_result["data"].empty()) {
                for (const auto& item : items_result["data"]) {
                    long product_id = item["product_id"].get<long>();
                    int quantity = item["quantity"].get<int>();
                    
                    // 返还库存
                    std::string restore_sql = "UPDATE products SET stock_quantity = stock_quantity + " + 
                                            std::to_string(quantity) + 
                                            ", updated_at = NOW() WHERE product_id = " + 
                                            std::to_string(product_id);
                    json restore_result = executeQuery(restore_sql);
                    
                    if (!restore_result["success"].get<bool>()) {
                        executeQuery("ROLLBACK");
                        return createErrorResponse("库存返还失败", Constants::DATABASE_ERROR_CODE);
                    }
                    
                    // 记录库存变动日志
                    logStockChange(product_id, quantity, "order_canceled", "order", order_id, 0);
                    
                    logInfo("返还库存: 商品ID=" + std::to_string(product_id) + 
                           ", 数量=" + std::to_string(quantity));
                }
            }
            
            // 提交事务
            executeQuery("COMMIT");
            
            json response_data;
            response_data["order_id"] = order_id;
            response_data["status"] = "cancelled";
            response_data["reason"] = reason;
            
            logInfo("订单取消成功，订单ID: " + std::to_string(order_id));
            return createSuccessResponse(response_data, "订单取消成功");
            
        } catch (const std::exception& e) {
            try { executeQuery("ROLLBACK"); } catch(...) {}
            return createErrorResponse("取消订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 更新订单状态（管理员功能）
json OrderService::updateOrderStatus(long order_id, const std::string& new_status) {
        logInfo("更新订单状态，订单ID: " + std::to_string(order_id) + ", 新状态: " + new_status);
        
        std::lock_guard<std::mutex> lock(order_mutex_);
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 验证状态值
        std::vector<std::string> valid_statuses = {
            "pending", "confirmed", "paid", "shipped", "delivered", "completed", "cancelled", "refunded"
        };
        
        if (std::find(valid_statuses.begin(), valid_statuses.end(), new_status) == valid_statuses.end()) {
            return createErrorResponse("无效的订单状态", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            const std::string& id_column = getOrderIdColumnName();
            const std::string& updated_column = getOrderUpdatedAtColumnName();
            bool has_payment_status = orderHasColumn("payment_status");
            // 获取当前订单状态
            std::string check_sql = "SELECT status";
            if (has_payment_status) {
                check_sql += ", payment_status";
            }
            check_sql += " FROM orders WHERE " + id_column + " = " + std::to_string(order_id);
            json check_result = executeQuery(check_sql);
            
            if (!check_result["success"].get<bool>() || check_result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            std::string current_status = check_result["data"][0]["status"].get<std::string>();
            std::string payment_status = has_payment_status && check_result["data"][0].contains("payment_status") &&
                                         check_result["data"][0]["payment_status"].is_string()
                                         ? check_result["data"][0]["payment_status"].get<std::string>()
                                         : "";
            
            // 状态转换验证
            if (!isValidStatusTransition(current_status, new_status)) {
                return createErrorResponse("不允许的状态转换: " + current_status + " -> " + new_status, 
                                         Constants::VALIDATION_ERROR_CODE);
            }
            
            // 更新订单状态
            std::vector<std::string> update_fields;
            update_fields.push_back("status = '" + escapeSQLString(new_status) + "'");
            if (!updated_column.empty()) {
                update_fields.push_back(updated_column + " = NOW()");
            }
            
            // 根据状态设置相应的时间戳
            if (new_status == "paid") {
                if (orderHasColumn("paid_at")) {
                    update_fields.push_back("paid_at = NOW()");
                }
                if (has_payment_status) {
                    update_fields.push_back("payment_status = 'paid'");
                }
            } else if (new_status == "shipped") {
                if (orderHasColumn("shipped_at")) {
                    update_fields.push_back("shipped_at = NOW()");
                }
            } else if (new_status == "delivered" || new_status == "completed") {
                if (orderHasColumn("delivered_at")) {
                    update_fields.push_back("delivered_at = NOW()");
                }
            } else if (new_status == "refunded") {
                if (has_payment_status) {
                    update_fields.push_back("payment_status = 'refunded'");
                }
            }
            
            std::string update_sql = "UPDATE orders SET " + joinColumns(update_fields) +
                                     " WHERE " + id_column + " = " + std::to_string(order_id);
            
            json result = executeQuery(update_sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["order_id"] = order_id;
                response_data["old_status"] = current_status;
                response_data["new_status"] = new_status;
                response_data["updated_at"] = getCurrentTimestamp();
                if (has_payment_status) {
                    response_data["payment_status_before"] = payment_status;
                }
                
                logInfo("订单状态更新成功，订单ID: " + std::to_string(order_id) + 
                       ", 从 " + current_status + " 更新为 " + new_status);
                return createSuccessResponse(response_data, "订单状态更新成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("更新订单状态异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 验证状态转换是否合法
bool OrderService::isValidStatusTransition(const std::string& from_status, const std::string& to_status) {
        // 定义状态转换规则
        std::map<std::string, std::vector<std::string>> transition_rules = {
            {"pending", {"confirmed", "cancelled"}},
            {"confirmed", {"paid", "cancelled"}},
            {"paid", {"shipped", "cancelled", "refunded"}},
            {"shipped", {"delivered", "cancelled"}},
            {"delivered", {"completed", "refunded"}},
            {"completed", {"refunded"}},
            {"cancelled", {}},  // 取消状态不能转换到其他状态
            {"refunded", {}}    // 退款状态不能转换到其他状态
        };
        
        auto it = transition_rules.find(from_status);
        if (it == transition_rules.end()) {
            return false;
        }
        
        const auto& allowed_transitions = it->second;
        return std::find(allowed_transitions.begin(), allowed_transitions.end(), to_status) != allowed_transitions.end();
    }
    
    // 按状态获取订单列表
json OrderService::getOrdersByStatus(long user_id, const std::string& status) {
        logInfo("按状态获取订单列表，用户ID: " + std::to_string(user_id) + ", 状态: " + status);
        
        if (user_id <= 0) {
            return createErrorResponse("无效的用户ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            const std::string& id_column = getOrderIdColumnName();
            const std::string& created_column = getOrderCreatedAtColumnName();
            const std::string& updated_column = getOrderUpdatedAtColumnName();
            std::string order_user_column = orderHasColumn("user_id") ? "user_id" :
                                            (orderHasColumn("customer_id") ? "customer_id" : "user_id");

            std::vector<std::string> select_fields = {
                aliasColumn("" + id_column, "order_id"),
                aliasColumn(getOrderNoColumnName().empty() ? std::string() : getOrderNoColumnName(), "order_no"),
                aliasColumn(orderHasColumn("total_amount") ? "total_amount" : std::string(), "total_amount"),
                aliasColumn(orderHasColumn("discount_amount") ? "discount_amount" : std::string(), "discount_amount"),
                aliasColumn(orderHasColumn("shipping_fee") ? "shipping_fee" : std::string(), "shipping_fee"),
                aliasColumn(orderHasColumn("final_amount") ? "final_amount" : std::string(), "final_amount"),
                aliasColumn(orderHasColumn("status") ? "status" : std::string(), "status"),
                aliasColumn(orderHasColumn("payment_status") ? "payment_status" : std::string(), "payment_status"),
                aliasColumn(created_column.empty() ? std::string() : created_column, "created_at"),
                aliasColumn(updated_column.empty() ? std::string() : updated_column, "updated_at")
            };

            std::string sql = "SELECT " + joinColumns(select_fields) + " FROM orders WHERE " +
                              order_user_column + " = " + std::to_string(user_id);

            if (status != "all" && !status.empty()) {
                sql += " AND status = '" + escapeSQLString(status) + "'";
            }
            
            std::string order_by_column = !created_column.empty() ? created_column : id_column;
            sql += " ORDER BY " + order_by_column + " DESC";
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                json response_data;
                response_data["user_id"] = user_id;
                response_data["status_filter"] = status;
                response_data["orders"] = result["data"];
                response_data["total_count"] = result["data"].size();
                
                return createSuccessResponse(response_data, "获取订单列表成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("获取订单列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 获取所有订单（管理员功能）
json OrderService::getAllOrders(const std::string& status, int page, int page_size, 
                     const std::string& start_date, const std::string& end_date) {
        logInfo("获取所有订单，状态: " + status + ", 页码: " + std::to_string(page));
        
        if (page <= 0) page = 1;
        if (page_size <= 0) page_size = 20;
        if (page_size > 100) page_size = 100;  // 限制每页最大数量
        
        try {
            const std::string& id_column = getOrderIdColumnName();
            const std::string& order_no_column = getOrderNoColumnName();
            const std::string& created_column = getOrderCreatedAtColumnName();
            const std::string& updated_column = getOrderUpdatedAtColumnName();
            std::string order_user_column = orderHasColumn("user_id") ? "user_id" :
                                            (orderHasColumn("customer_id") ? "customer_id" : "user_id");
            const std::string& user_pk_column = getUsersPrimaryKeyColumn();

            std::vector<std::string> select_fields;
            select_fields.push_back(aliasColumn("o." + id_column, "order_id"));
            select_fields.push_back(order_no_column.empty() ? aliasColumn("", "order_no")
                                                           : aliasColumn("o." + order_no_column, "order_no"));
            select_fields.push_back(aliasColumn("o." + order_user_column, "user_id"));
            select_fields.push_back(aliasColumn("u." + user_pk_column, "user_table_id"));
            select_fields.push_back(aliasColumn("u.username", "username"));
            if (orderHasColumn("total_amount")) {
                select_fields.push_back(aliasColumn("o.total_amount", "total_amount"));
            }
            if (orderHasColumn("discount_amount")) {
                select_fields.push_back(aliasColumn("o.discount_amount", "discount_amount"));
            } else {
                select_fields.push_back(aliasColumn("", "discount_amount"));
            }
            if (orderHasColumn("shipping_fee")) {
                select_fields.push_back(aliasColumn("o.shipping_fee", "shipping_fee"));
            } else {
                select_fields.push_back(aliasColumn("", "shipping_fee"));
            }
            if (orderHasColumn("final_amount")) {
                select_fields.push_back(aliasColumn("o.final_amount", "final_amount"));
            }
            if (orderHasColumn("status")) {
                select_fields.push_back(aliasColumn("o.status", "status"));
            }
            if (orderHasColumn("payment_status")) {
                select_fields.push_back(aliasColumn("o.payment_status", "payment_status"));
            } else {
                select_fields.push_back(aliasColumn("", "payment_status"));
            }
            if (orderHasColumn("payment_method")) {
                select_fields.push_back(aliasColumn("o.payment_method", "payment_method"));
            } else {
                select_fields.push_back(aliasColumn("", "payment_method"));
            }
            if (orderHasColumn("tracking_number")) {
                select_fields.push_back(aliasColumn("o.tracking_number", "tracking_number"));
            } else {
                select_fields.push_back(aliasColumn("", "tracking_number"));
            }
            if (orderHasColumn("shipping_method")) {
                select_fields.push_back(aliasColumn("o.shipping_method", "shipping_method"));
            } else {
                select_fields.push_back(aliasColumn("", "shipping_method"));
            }
            select_fields.push_back(aliasColumn(created_column.empty() ? std::string() : ("o." + created_column), "created_at"));
            select_fields.push_back(aliasColumn(updated_column.empty() ? std::string() : ("o." + updated_column), "updated_at"));
            if (orderHasColumn("paid_at")) {
                select_fields.push_back(aliasColumn("o.paid_at", "paid_at"));
            } else {
                select_fields.push_back(aliasColumn("", "paid_at"));
            }
            if (orderHasColumn("shipped_at")) {
                select_fields.push_back(aliasColumn("o.shipped_at", "shipped_at"));
            } else {
                select_fields.push_back(aliasColumn("", "shipped_at"));
            }
            if (orderHasColumn("delivered_at")) {
                select_fields.push_back(aliasColumn("o.delivered_at", "delivered_at"));
            } else {
                select_fields.push_back(aliasColumn("", "delivered_at"));
            }
            if (orderHasColumn("remark")) {
                select_fields.push_back(aliasColumn("o.remark", "remark"));
            }

            std::string sql = "SELECT " + joinColumns(select_fields) +
                              " FROM orders o LEFT JOIN users u ON o." + order_user_column +
                              " = u." + user_pk_column + " WHERE 1=1";
            
            if (status != "all" && !status.empty()) {
                sql += " AND o.status = '" + status + "'";
            }
            
            if (!start_date.empty()) {
                if (!created_column.empty()) {
                    sql += " AND DATE(o." + created_column + ") >= '" + start_date + "'";
                }
            }
            
            if (!end_date.empty()) {
                if (!created_column.empty()) {
                    sql += " AND DATE(o." + created_column + ") <= '" + end_date + "'";
                }
            }
            
            std::string order_by_column = !created_column.empty() ? "o." + created_column : "o." + id_column;
            sql += " ORDER BY " + order_by_column + " DESC";
            
            // 计算分页
            int offset = (page - 1) * page_size;
            sql += " LIMIT " + std::to_string(page_size) + " OFFSET " + std::to_string(offset);
            
            json result = executeQuery(sql);
            if (result["success"].get<bool>()) {
                // 获取总记录数
                std::string count_sql = "SELECT COUNT(*) as total_count FROM orders o WHERE 1=1";
                if (status != "all" && !status.empty()) {
                    count_sql += " AND o.status = '" + status + "'";
                }
                if (!start_date.empty()) {
                    if (!created_column.empty()) {
                        count_sql += " AND DATE(o." + created_column + ") >= '" + start_date + "'";
                    }
                }
                if (!end_date.empty()) {
                    if (!created_column.empty()) {
                        count_sql += " AND DATE(o." + created_column + ") <= '" + end_date + "'";
                    }
                }
                
                json count_result = executeQuery(count_sql);
                int total_count = 0;
                if (count_result["success"].get<bool>() && !count_result["data"].empty()) {
                    total_count = count_result["data"][0]["total_count"].get<int>();
                }
                
                json response_data;
                response_data["orders"] = result["data"];
                response_data["page"] = page;
                response_data["page_size"] = page_size;
                response_data["total_count"] = total_count;
                response_data["total_pages"] = (total_count + page_size - 1) / page_size;
                response_data["status_filter"] = status;
                
                return createSuccessResponse(response_data, "获取订单列表成功");
            }
            
            return result;
        } catch (const std::exception& e) {
            return createErrorResponse("获取订单列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 订单物流跟踪
json OrderService::trackOrder(long order_id) {
        logInfo("跟踪订单物流，订单ID: " + std::to_string(order_id));
        
        if (order_id <= 0) {
            return createErrorResponse("无效的订单ID", Constants::VALIDATION_ERROR_CODE);
        }
        
        try {
            std::string sql = "SELECT order_id, order_no, status, tracking_number, shipping_method, "
                             "shipped_at, delivered_at FROM orders WHERE order_id = " + 
                             std::to_string(order_id);
            
            json result = executeQuery(sql);
            if (!result["success"].get<bool>() || result["data"].empty()) {
                return createErrorResponse("订单不存在", Constants::VALIDATION_ERROR_CODE);
            }
            
            json order_data = result["data"][0];
            
            // 生成物流跟踪信息
            json tracking_info = json::array();
            
            if (order_data["status"].get<std::string>() == "shipped" || 
                order_data["status"].get<std::string>() == "delivered" ||
                order_data["status"].get<std::string>() == "completed") {
                
                tracking_info.push_back({
                    {"time", order_data["shipped_at"]},
                    {"status", "已发货"},
                    {"description", "商品已从仓库发出"}
                });
                
                if (order_data["status"].get<std::string>() == "delivered" ||
                    order_data["status"].get<std::string>() == "completed") {
                    tracking_info.push_back({
                        {"time", order_data["delivered_at"]},
                        {"status", "已送达"},
                        {"description", "商品已送达收货地址"}
                    });
                }
            }
            
            json response_data;
            response_data["order_id"] = order_id;
            response_data["order_no"] = order_data["order_no"];
            response_data["status"] = order_data["status"];
            response_data["tracking_number"] = order_data["tracking_number"];
            response_data["shipping_method"] = order_data["shipping_method"];
            response_data["tracking_info"] = tracking_info;
            
            return createSuccessResponse(response_data, "获取物流信息成功");
            
        } catch (const std::exception& e) {
            return createErrorResponse("跟踪订单异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
        }
    }
    
    // 审核退款申请(管理员功能)
json OrderService::approveRefund(long refund_id, long admin_id, bool approve, const std::string& admin_reply) {
    logInfo("管理员审核退款，退款ID: " + std::to_string(refund_id) + ", 管理员ID: " + std::to_string(admin_id) + 
            ", 审核结果: " + (approve ? "批准" : "拒绝"));
    
    std::lock_guard<std::mutex> lock(order_mutex_);
    
    if (refund_id <= 0 || admin_id <= 0) {
        return createErrorResponse("无效的退款ID或管理员ID", Constants::VALIDATION_ERROR_CODE);
    }
    
    try {
        executeQuery("BEGIN");
        
        // 获取退款申请信息
        std::string query_sql = "SELECT r.order_id, r.user_id, r.reason, r.refund_amount, r.status, "
                               "o.status as order_status, o.payment_status "
                               "FROM refund_requests r "
                               "JOIN orders o ON r.order_id = o.order_id "
                               "WHERE r.refund_id = " + std::to_string(refund_id) + " FOR UPDATE";
        json query_result = executeQuery(query_sql);
        
        if (!query_result["success"].get<bool>() || query_result["data"].empty()) {
            executeQuery("ROLLBACK");
            return createErrorResponse("退款申请不存在", Constants::VALIDATION_ERROR_CODE);
        }
        
        auto refund_data = query_result["data"][0];
        std::string refund_status = refund_data["status"].get<std::string>();
        
        if (refund_status != "pending") {
            executeQuery("ROLLBACK");
            return createErrorResponse("该退款申请已处理，无法重复审核", Constants::VALIDATION_ERROR_CODE);
        }
        
        long order_id = refund_data["order_id"].get<long>();
        long user_id = refund_data["user_id"].get<long>();
        double refund_amount = refund_data["refund_amount"].get<double>();
        
        if (approve) {
            // 批准退款
            // 更新退款申请状态
            std::string update_refund_sql = "UPDATE refund_requests SET status = 'approved', "
                                           "processed_by = " + std::to_string(admin_id) + ", "
                                           "admin_reply = '" + escapeSQLString(admin_reply) + "', "
                                           "processed_at = NOW() WHERE refund_id = " + std::to_string(refund_id);
            json update_refund_result = executeQuery(update_refund_sql);
            
            if (!update_refund_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("更新退款申请失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 更新订单状态为refunded
            std::string update_order_sql = "UPDATE orders SET status = 'refunded', payment_status = 'refunded', "
                                          "updated_at = NOW() WHERE order_id = " + std::to_string(order_id);
            json update_order_result = executeQuery(update_order_sql);
            
            if (!update_order_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("更新订单状态失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 恢复优惠券（如果订单使用了优惠券）⭐新增⭐
            std::string coupon_check_sql = "SELECT id, coupon_id FROM user_coupons "
                                          "WHERE order_id = " + std::to_string(order_id) + 
                                          " AND status = 'used' LIMIT 1";
            json coupon_check_result = executeQuery(coupon_check_sql);
            
            if (coupon_check_result["success"].get<bool>() && !coupon_check_result["data"].empty()) {
                auto coupon_data = coupon_check_result["data"][0];
                long user_coupon_id = coupon_data["id"].get<long>();
                long coupon_id = coupon_data["coupon_id"].get<long>();
                
                // 恢复优惠券状态为unused
                std::string restore_coupon_sql = "UPDATE user_coupons SET status = 'unused', "
                                                "order_id = NULL, used_at = NULL "
                                                "WHERE id = " + std::to_string(user_coupon_id);
                json restore_coupon_result = executeQuery(restore_coupon_sql);
                
                if (!restore_coupon_result["success"].get<bool>()) {
                    executeQuery("ROLLBACK");
                    logError("恢复优惠券失败: user_coupon_id=" + std::to_string(user_coupon_id));
                    return createErrorResponse("恢复优惠券失败", Constants::DATABASE_ERROR_CODE);
                }
                
                logInfo("退款批准-优惠券已恢复: user_coupon_id=" + std::to_string(user_coupon_id) + 
                       ", coupon_id=" + std::to_string(coupon_id));
            }
            
            // 返还库存
            std::string items_sql = "SELECT product_id, quantity FROM order_items WHERE order_id = " + std::to_string(order_id);
            json items_result = executeQuery(items_sql);
            
            if (items_result["success"].get<bool>() && !items_result["data"].empty()) {
                for (const auto& item : items_result["data"]) {
                    long product_id = item["product_id"].get<long>();
                    int quantity = item["quantity"].get<int>();
                    
                    // 返还库存
                    std::string restore_sql = "UPDATE products SET stock_quantity = stock_quantity + " + 
                                             std::to_string(quantity) + ", updated_at = NOW() "
                                             "WHERE product_id = " + std::to_string(product_id);
                    json restore_result = executeQuery(restore_sql);
                    
                    if (!restore_result["success"].get<bool>()) {
                        executeQuery("ROLLBACK");
                        return createErrorResponse("库存返还失败", Constants::DATABASE_ERROR_CODE);
                    }
                    
                    // 记录库存变动
                    logStockChange(product_id, quantity, "refund_approved", "refund", refund_id, admin_id);
                    
                    logInfo("退款返还库存: 商品ID=" + std::to_string(product_id) + ", 数量=" + std::to_string(quantity));
                }
            }
            
            // 先提交事务
            json commit_result = executeQuery("COMMIT");
            if (!commit_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("提交事务失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 事务提交后创建通知(确保通知不受事务影响)
            try {
                createNotification(user_id, "refund_approved", "退款已批准", 
                                 "您的订单 #" + std::to_string(order_id) + " 退款申请已批准，退款金额: ¥" + 
                                 std::to_string(refund_amount) + "。" + (admin_reply.empty() ? "" : "管理员回复: " + admin_reply), 
                                 order_id);
            } catch (const std::exception& e) {
                logError("创建批准通知失败(不影响退款处理): " + std::string(e.what()));
            }
            
            json response_data;
            response_data["refund_id"] = refund_id;
            response_data["order_id"] = order_id;
            response_data["status"] = "approved";
            response_data["refund_amount"] = refund_amount;
            
            logInfo("退款申请已批准，退款ID: " + std::to_string(refund_id));
            return createSuccessResponse(response_data, "退款申请已批准");
            
        } else {
            // 拒绝退款
            std::string update_refund_sql = "UPDATE refund_requests SET status = 'rejected', "
                                           "processed_by = " + std::to_string(admin_id) + ", "
                                           "admin_reply = '" + escapeSQLString(admin_reply) + "', "
                                           "processed_at = NOW() WHERE refund_id = " + std::to_string(refund_id);
            json update_refund_result = executeQuery(update_refund_sql);
            
            if (!update_refund_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("更新退款申请失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 恢复订单状态(从refunding恢复到之前的状态,这里简单恢复为paid)
            std::string update_order_sql = "UPDATE orders SET status = 'paid', updated_at = NOW() "
                                          "WHERE order_id = " + std::to_string(order_id);
            json update_order_result = executeQuery(update_order_sql);
            
            if (!update_order_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("更新订单状态失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 先提交事务
            json commit_result = executeQuery("COMMIT");
            if (!commit_result["success"].get<bool>()) {
                executeQuery("ROLLBACK");
                return createErrorResponse("提交事务失败", Constants::DATABASE_ERROR_CODE);
            }
            
            // 事务提交后创建通知(确保通知不受事务影响)
            try {
                createNotification(user_id, "refund_rejected", "退款被拒绝", 
                                 "您的订单 #" + std::to_string(order_id) + " 退款申请被拒绝。" + 
                                 (admin_reply.empty() ? "" : "原因: " + admin_reply), order_id);
            } catch (const std::exception& e) {
                logError("创建拒绝通知失败(不影响退款处理): " + std::string(e.what()));
            }
            
            json response_data;
            response_data["refund_id"] = refund_id;
            response_data["order_id"] = order_id;
            response_data["status"] = "rejected";
            
            logInfo("退款申请已拒绝，退款ID: " + std::to_string(refund_id));
            return createSuccessResponse(response_data, "退款申请已拒绝");
        }
        
    } catch (const std::exception& e) {
        try { executeQuery("ROLLBACK"); } catch(...) {}
        return createErrorResponse("审核退款异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 获取退款申请列表(管理员功能)
json OrderService::getRefundRequests(const std::string& status, int page, int page_size) {
    logInfo("获取退款申请列表，状态: " + status + ", 页码: " + std::to_string(page));
    
    try {
        if (page < 1) page = 1;
        if (page_size < 1 || page_size > 100) page_size = 20;
        
        int offset = (page - 1) * page_size;
        
        // 构建WHERE条件
        std::string where_clause = " WHERE 1=1 ";
        if (status != "all") {
            where_clause += " AND r.status = '" + escapeSQLString(status) + "'";
        }
        
        // 查询总数
        std::string count_sql = "SELECT COUNT(*) as total FROM refund_requests r" + where_clause;
        json count_result = executeQuery(count_sql);
        int total = 0;
        if (count_result["success"].get<bool>() && !count_result["data"].empty()) {
            total = count_result["data"][0]["total"].get<int>();
        }
        
        // 查询列表
        std::string query_sql = "SELECT r.refund_id, r.order_id, r.user_id, r.reason, r.refund_amount, "
                               "r.status, r.processed_by as admin_id, r.admin_reply, r.created_at, r.processed_at, "
                               "o.order_no, u.username "
                               "FROM refund_requests r "
                               "JOIN orders o ON r.order_id = o.order_id "
                               "JOIN users u ON r.user_id = u.user_id " +
                               where_clause +
                               " ORDER BY r.created_at DESC LIMIT " + std::to_string(page_size) + 
                               " OFFSET " + std::to_string(offset);
        
        json result = executeQuery(query_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        json response_data;
        response_data["list"] = result["data"];
        response_data["pagination"] = {
            {"page", page},
            {"page_size", page_size},
            {"total", total},
            {"total_pages", (total + page_size - 1) / page_size}
        };
        
        return createSuccessResponse(response_data, "获取退款申请列表成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取退款申请列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 获取用户的退款申请
json OrderService::getUserRefundRequests(long user_id) {
    // 特殊处理：如果 user_id 为负数，则将其绝对值作为 order_id 来查询
    bool query_by_order_id = (user_id < 0);
    long actual_id = query_by_order_id ? -user_id : user_id;
    
    if (query_by_order_id) {
        logInfo("获取订单退款申请，订单ID: " + std::to_string(actual_id));
    } else {
        logInfo("获取用户退款申请，用户ID: " + std::to_string(actual_id));
    }
    
    try {
        std::string query_sql = "SELECT r.refund_id, r.order_id, r.reason, r.refund_amount, "
                               "r.status, r.admin_reply, r.created_at, r.processed_at, o.order_no "
                               "FROM refund_requests r "
                               "JOIN orders o ON r.order_id = o.order_id "
                               "WHERE ";
        
        if (query_by_order_id) {
            query_sql += "r.order_id = " + std::to_string(actual_id);
        } else {
            query_sql += "r.user_id = " + std::to_string(actual_id);
        }
        
        query_sql += " ORDER BY r.created_at DESC";
        
        json result = executeQuery(query_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        return createSuccessResponse(result["data"], "获取退款申请成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取退款申请异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 创建用户通知
json OrderService::createNotification(long user_id, const std::string& type, 
                                     const std::string& title, const std::string& content, long related_id) {
    try {
        std::string insert_sql = "INSERT INTO user_notifications (user_id, type, title, content, related_id, is_read, created_at) "
                                "VALUES (" + std::to_string(user_id) + ", '" + escapeSQLString(type) + "', '" + 
                                escapeSQLString(title) + "', '" + escapeSQLString(content) + "', " + 
                                std::to_string(related_id) + ", FALSE, NOW())";
        
        json result = executeQuery(insert_sql);
        
        if (result["success"].get<bool>()) {
            logInfo("创建通知成功，用户ID: " + std::to_string(user_id) + ", 标题: " + title);
        }
        
        return result;
        
    } catch (const std::exception& e) {
        logError("创建通知异常: " + std::string(e.what()));
        return createErrorResponse("创建通知异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 获取用户通知列表
json OrderService::getNotifications(long user_id, bool unread_only) {
    logInfo("获取用户通知，用户ID: " + std::to_string(user_id) + ", 仅未读: " + (unread_only ? "是" : "否"));
    
    try {
        std::string query_sql = "SELECT notification_id, type, title, content, related_id, is_read, created_at "
                               "FROM user_notifications WHERE user_id = " + std::to_string(user_id);
        
        if (unread_only) {
            query_sql += " AND is_read = FALSE";
        }
        
        query_sql += " ORDER BY created_at DESC LIMIT 50";
        
        json result = executeQuery(query_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        return createSuccessResponse(result["data"], "获取通知列表成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("获取通知列表异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 标记通知为已读
json OrderService::markNotificationRead(long notification_id, long user_id) {
    logInfo("标记通知已读，通知ID: " + std::to_string(notification_id) + ", 用户ID: " + std::to_string(user_id));
    
    try {
        // 先检查通知是否存在
        std::string check_sql = "SELECT notification_id, is_read FROM user_notifications "
                               "WHERE notification_id = " + std::to_string(notification_id) + 
                               " AND user_id = " + std::to_string(user_id);
        json check_result = executeQuery(check_sql);
        
        if (!check_result["success"].get<bool>()) {
            return check_result;
        }
        
        if (check_result["data"].empty()) {
            return createErrorResponse("通知不存在或无权访问", Constants::VALIDATION_ERROR_CODE);
        }
        
        // 执行更新
        std::string update_sql = "UPDATE user_notifications SET is_read = 1 "
                                "WHERE notification_id = " + std::to_string(notification_id) + 
                                " AND user_id = " + std::to_string(user_id);
        
        json result = executeQuery(update_sql);
        
        if (!result["success"].get<bool>()) {
            return result;
        }
        
        logInfo("通知已标记为已读: notification_id=" + std::to_string(notification_id));
        return createSuccessResponse(json(), "标记通知已读成功");
        
    } catch (const std::exception& e) {
        return createErrorResponse("标记通知已读异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 删除通知
json OrderService::deleteNotification(long notification_id, long user_id) {
    logInfo("删除通知，请求通知ID: " + std::to_string(notification_id) + ", 用户ID: " + std::to_string(user_id));

    try {
        std::string check_sql = "SELECT notification_id FROM user_notifications "
                               "WHERE notification_id = " + std::to_string(notification_id) +
                               " AND user_id = " + std::to_string(user_id);
        json check_result = executeQuery(check_sql);

        if (!check_result["success"].get<bool>()) {
            return check_result;
        }

        if (check_result["data"].empty()) {
            return createErrorResponse("通知不存在或无权访问", Constants::VALIDATION_ERROR_CODE);
        }

        std::string delete_sql = "DELETE FROM user_notifications WHERE notification_id = " +
                                 std::to_string(notification_id) + " AND user_id = " + std::to_string(user_id);

        json delete_result = executeQuery(delete_sql);

        if (!delete_result["success"].get<bool>()) {
            return delete_result;
        }

        long affected = 0;
        if (delete_result["data"].is_object() && delete_result["data"].contains("affected_rows")) {
            affected = delete_result["data"]["affected_rows"].get<long>();
        }

        if (affected <= 0) {
            return createErrorResponse("删除通知失败，请稍后重试", Constants::DATABASE_ERROR_CODE);
        }

        json response_data;
        response_data["notification_id"] = notification_id;
        response_data["deleted"] = true;

        logInfo("通知删除成功，通知ID: " + std::to_string(notification_id));
        return createSuccessResponse(response_data, "删除通知成功");

    } catch (const std::exception& e) {
        return createErrorResponse("删除通知异常: " + std::string(e.what()), Constants::DATABASE_ERROR_CODE);
    }
}

// 记录库存变动
bool OrderService::logStockChange(long product_id, int change_qty, const std::string& reason,
                                  const std::string& related_type, long related_id, long operator_id) {
    try {
        // 获取当前库存
        std::string query_sql = "SELECT stock_quantity FROM products WHERE product_id = " + std::to_string(product_id);
        json query_result = executeQuery(query_sql);
        
        int stock_before = 0;
        if (query_result["success"].get<bool>() && !query_result["data"].empty()) {
            stock_before = query_result["data"][0]["stock_quantity"].get<int>();
        }
        
        int stock_after = stock_before + change_qty;
        
        // 插入库存变动记录
        std::string insert_sql = "INSERT INTO stock_logs (product_id, change_quantity, stock_before, stock_after, "
                                "reason, related_type, related_id, operator_id, created_at) "
                                "VALUES (" + std::to_string(product_id) + ", " + std::to_string(change_qty) + ", " + 
                                std::to_string(stock_before) + ", " + std::to_string(stock_after) + ", '" + 
                                escapeSQLString(reason) + "', '" + escapeSQLString(related_type) + "', " + 
                                std::to_string(related_id) + ", " + std::to_string(operator_id) + ", NOW())";
        
        json result = executeQuery(insert_sql);
        
        if (result["success"].get<bool>()) {
            logInfo("记录库存变动: 商品ID=" + std::to_string(product_id) + 
                   ", 变动=" + std::to_string(change_qty) + ", 原因=" + reason);
            return true;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        logError("记录库存变动异常: " + std::string(e.what()));
        return false;
    }
}

    // 获取当前时间戳
std::string OrderService::getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
