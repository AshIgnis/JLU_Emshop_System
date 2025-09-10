package emshop;

import java.util.Scanner;

/**
 * 管理员界面 - 专门为管理员提供的控制台界面
 * 提供库存管理、用户管理、订单管理等管理功能
 */
public class AdminConsole {
    
    private static Scanner scanner = new Scanner(System.in);
    private static long currentAdminId = 0;
    private static String currentAdminName = "";
    
    public static void main(String[] args) {
        System.out.println("===============================================");
        System.out.println("    JLU Emshop System - 管理员控制台");
        System.out.println("===============================================");
        
        // 管理员登录
        if (!adminLogin()) {
            System.out.println("登录失败，程序退出。");
            return;
        }
        
        // 主菜单循环
        mainMenu();
    }
    
    /**
     * 管理员登录
     */
    private static boolean adminLogin() {
        System.out.println("\n请输入管理员登录信息：");
        
        for (int attempt = 0; attempt < 3; attempt++) {
            System.out.print("用户名: ");
            String username = scanner.nextLine();
            System.out.print("密码: ");
            String password = scanner.nextLine();
            
            try {
                String result = EmshopNativeInterface.login(username, password);
                if (result.contains("\"success\":true")) {
                    // 检查是否为管理员
                    if (result.contains("\"role\":\"admin\"")) {
                        currentAdminId = extractUserIdFromResponse(result);
                        currentAdminName = username;
                        System.out.println("管理员登录成功！欢迎，" + username);
                        return true;
                    } else {
                        System.out.println("错误：您不是管理员，无法访问管理控制台");
                        return false;
                    }
                } else {
                    System.out.println("登录失败：" + extractMessageFromResponse(result));
                    System.out.println("剩余尝试次数：" + (2 - attempt));
                }
            } catch (Exception e) {
                System.err.println("登录过程中发生错误：" + e.getMessage());
            }
        }
        
        return false;
    }
    
    /**
     * 主菜单
     */
    private static void mainMenu() {
        while (true) {
            System.out.println("\n===============================================");
            System.out.println("管理员控制台 - " + currentAdminName);
            System.out.println("===============================================");
            System.out.println("1. 库存管理");
            System.out.println("2. 商品管理");
            System.out.println("3. 用户管理");
            System.out.println("4. 订单管理");
            System.out.println("5. 系统报告");
            System.out.println("6. 系统设置");
            System.out.println("0. 退出系统");
            System.out.println("===============================================");
            System.out.print("请选择操作 (0-6): ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    inventoryManagement();
                    break;
                case "2":
                    productManagement();
                    break;
                case "3":
                    userManagement();
                    break;
                case "4":
                    orderManagement();
                    break;
                case "5":
                    systemReports();
                    break;
                case "6":
                    systemSettings();
                    break;
                case "0":
                    System.out.println("感谢使用 JLU Emshop 管理系统，再见！");
                    return;
                default:
                    System.out.println("无效选择，请重新输入");
            }
        }
    }
    
    /**
     * 库存管理菜单
     */
    private static void inventoryManagement() {
        while (true) {
            System.out.println("\n--- 库存管理 ---");
            System.out.println("1. 查看商品库存");
            System.out.println("2. 更新库存");
            System.out.println("3. 低库存报告");
            System.out.println("4. 批量库存调整");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择操作: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    checkStock();
                    break;
                case "2":
                    updateStock();
                    break;
                case "3":
                    lowStockReport();
                    break;
                case "4":
                    batchStockUpdate();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 检查库存
     */
    private static void checkStock() {
        System.out.print("请输入商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            String result = EmshopNativeInterface.checkStock(productId);
            System.out.println("库存查询结果：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的商品ID格式");
        }
    }
    
    /**
     * 更新库存
     */
    private static void updateStock() {
        System.out.print("请输入商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            System.out.print("请输入数量: ");
            int quantity = Integer.parseInt(scanner.nextLine());
            System.out.print("请输入操作类型 (add/subtract/set): ");
            String operation = scanner.nextLine();
            
            String result = EmshopNativeInterface.updateStock(productId, quantity, operation);
            System.out.println("库存更新结果：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的数字格式");
        }
    }
    
    /**
     * 低库存报告
     */
    private static void lowStockReport() {
        System.out.print("请输入库存阈值: ");
        try {
            int threshold = Integer.parseInt(scanner.nextLine());
            String result = EmshopNativeInterface.getLowStockProducts(threshold);
            System.out.println("低库存报告：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的阈值格式");
        }
    }
    
    /**
     * 批量库存调整
     */
    private static void batchStockUpdate() {
        System.out.println("批量库存调整功能");
        System.out.print("请输入调整操作 (add/subtract): ");
        String operation = scanner.nextLine();
        System.out.print("请输入调整数量: ");
        try {
            int quantity = Integer.parseInt(scanner.nextLine());
            System.out.print("请输入商品ID列表 (用逗号分隔): ");
            String[] productIds = scanner.nextLine().split(",");
            
            for (String idStr : productIds) {
                try {
                    long productId = Long.parseLong(idStr.trim());
                    String result = EmshopNativeInterface.updateStock(productId, quantity, operation);
                    System.out.println("商品 " + productId + " 库存调整结果：" + 
                                     (result.contains("\"success\":true") ? "成功" : "失败"));
                } catch (NumberFormatException e) {
                    System.out.println("无效的商品ID: " + idStr);
                }
            }
        } catch (NumberFormatException e) {
            System.out.println("无效的数量格式");
        }
    }
    
    /**
     * 商品管理菜单
     */
    private static void productManagement() {
        System.out.println("\n--- 商品管理 ---");
        System.out.println("1. 添加新商品");
        System.out.println("2. 查看所有商品");
        System.out.println("3. 搜索商品");
        System.out.println("4. 编辑商品信息");
        System.out.println("5. 删除商品");
        System.out.println("6. 商品分类管理");
        System.out.println("0. 返回主菜单");
        System.out.print("请选择操作: ");
        
        String choice = scanner.nextLine();
        
        switch (choice) {
            case "1":
                addProduct();
                break;
            case "2":
                viewAllProducts();
                break;
            case "3":
                searchProducts();
                break;
            case "4":
                editProduct();
                break;
            case "5":
                deleteProduct();
                break;
            case "6":
                manageCategoriesMenu();
                break;
            case "0":
                return;
            default:
                System.out.println("无效选择");
        }
    }
    
    /**
     * 添加商品
     */
    private static void addProduct() {
        System.out.println("\n--- 添加商品 ---");
        
        System.out.print("商品名称: ");
        String name = scanner.nextLine();
        System.out.print("商品描述: ");
        String description = scanner.nextLine();
        System.out.print("商品价格: ");
        double price = Double.parseDouble(scanner.nextLine());
        System.out.print("库存数量: ");
        int stock = Integer.parseInt(scanner.nextLine());
        System.out.print("商品分类: ");
        String category = scanner.nextLine();
        
        try {
            // 构建JSON数据
            String productJson = String.format(
                "{\"name\":\"%s\",\"description\":\"%s\",\"price\":%.2f,\"stock_quantity\":%d,\"category\":\"%s\"}",
                name, description, price, stock, category
            );
            
            String result = EmshopNativeInterface.addProduct(productJson);
            System.out.println("添加结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("添加商品失败：" + e.getMessage());
        }
    }
    
    /**
     * 查看所有商品
     */
    private static void viewAllProducts() {
        try {
            String result = EmshopNativeInterface.getProductList("all", 1, 50);
            System.out.println("\n商品列表：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取商品列表失败：" + e.getMessage());
        }
    }
    
    /**
     * 搜索商品
     */
    private static void searchProducts() {
        System.out.print("请输入搜索关键词: ");
        String keyword = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.searchProducts(keyword, 1, 20, "id", 0.0, 999999.0);
            System.out.println("\n搜索结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("搜索商品失败：" + e.getMessage());
        }
    }
    
    /**
     * 编辑商品
     */
    private static void editProduct() {
        System.out.print("请输入要编辑的商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            
            // 先获取商品详情
            String productDetail = EmshopNativeInterface.getProductDetail(productId);
            System.out.println("当前商品信息：");
            printFormattedResponse(productDetail);
            
            System.out.print("新的商品名称 (回车跳过): ");
            String name = scanner.nextLine();
            System.out.print("新的商品描述 (回车跳过): ");
            String description = scanner.nextLine();
            System.out.print("新的价格 (回车跳过): ");
            String priceStr = scanner.nextLine();
            System.out.print("新的分类 (回车跳过): ");
            String category = scanner.nextLine();
            
            // 构建更新JSON
            StringBuilder jsonBuilder = new StringBuilder("{");
            if (!name.isEmpty()) jsonBuilder.append("\"name\":\"").append(name).append("\",");
            if (!description.isEmpty()) jsonBuilder.append("\"description\":\"").append(description).append("\",");
            if (!priceStr.isEmpty()) jsonBuilder.append("\"price\":").append(priceStr).append(",");
            if (!category.isEmpty()) jsonBuilder.append("\"category\":\"").append(category).append("\",");
            
            String updateJson = jsonBuilder.toString();
            if (updateJson.endsWith(",")) {
                updateJson = updateJson.substring(0, updateJson.length() - 1);
            }
            updateJson += "}";
            
            String result = EmshopNativeInterface.updateProduct(productId, updateJson);
            System.out.println("更新结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的商品ID");
        } catch (Exception e) {
            System.err.println("编辑商品失败：" + e.getMessage());
        }
    }
    
    /**
     * 删除商品
     */
    private static void deleteProduct() {
        System.out.print("请输入要删除的商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            
            System.out.print("确认删除商品 " + productId + "? (y/N): ");
            String confirm = scanner.nextLine();
            
            if ("y".equalsIgnoreCase(confirm) || "yes".equalsIgnoreCase(confirm)) {
                String result = EmshopNativeInterface.deleteProduct(productId);
                System.out.println("删除结果：");
                printFormattedResponse(result);
            } else {
                System.out.println("取消删除操作");
            }
            
        } catch (NumberFormatException e) {
            System.out.println("无效的商品ID");
        } catch (Exception e) {
            System.err.println("删除商品失败：" + e.getMessage());
        }
    }
    
    /**
     * 分类管理菜单
     */
    private static void manageCategoriesMenu() {
        while (true) {
            System.out.println("\n--- 分类管理 ---");
            System.out.println("1. 查看所有分类");
            System.out.println("2. 按分类查看商品");
            System.out.println("0. 返回上级菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewCategories();
                    break;
                case "2":
                    viewProductsByCategory();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看所有分类
     */
    private static void viewCategories() {
        try {
            String result = EmshopNativeInterface.getCategories();
            System.out.println("\n商品分类：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取分类失败：" + e.getMessage());
        }
    }
    
    /**
     * 按分类查看商品
     */
    private static void viewProductsByCategory() {
        System.out.print("请输入分类名称: ");
        String category = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.getCategoryProducts(category, 1, 20, "id");
            System.out.println("\n分类商品：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取分类商品失败：" + e.getMessage());
        }
    }
    
    /**
     * 用户管理菜单
     */
    private static void userManagement() {
        while (true) {
            System.out.println("\n--- 用户管理 ---");
            System.out.println("1. 查看所有用户");
            System.out.println("2. 搜索用户");
            System.out.println("3. 用户角色管理");
            System.out.println("4. 封禁用户");
            System.out.println("5. 解封用户");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewAllUsers();
                    break;
                case "2":
                    searchUsers();
                    break;
                case "3":
                    manageUserRoles();
                    break;
                case "4":
                    banUser();
                    break;
                case "5":
                    unbanUser();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看所有用户
     */
    private static void viewAllUsers() {
        try {
            String result = EmshopNativeInterface.getAllUsers(1, 20, "all");
            System.out.println("\n用户列表：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取用户列表失败：" + e.getMessage());
        }
    }
    
    /**
     * 搜索用户
     */
    private static void searchUsers() {
        System.out.print("请输入用户名关键词: ");
        String keyword = scanner.nextLine();
        
        try {
            // 使用分页查询
            String result = EmshopNativeInterface.getAllUsers(1, 50, "all");
            System.out.println("\n搜索结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("搜索用户失败：" + e.getMessage());
        }
    }
    
    /**
     * 用户角色管理
     */
    private static void manageUserRoles() {
        System.out.print("请输入用户ID: ");
        try {
            long userId = Long.parseLong(scanner.nextLine());
            
            // 获取当前用户信息
            String userInfo = EmshopNativeInterface.getUserInfo(userId);
            System.out.println("当前用户信息：");
            printFormattedResponse(userInfo);
            
            System.out.println("\n可用角色：");
            System.out.println("1. admin - 管理员");
            System.out.println("2. user - 普通用户");
            System.out.println("3. vip - VIP用户");
            System.out.print("请选择新角色 (1-3): ");
            
            String roleChoice = scanner.nextLine();
            String newRole = "";
            
            switch (roleChoice) {
                case "1":
                    newRole = "admin";
                    break;
                case "2":
                    newRole = "user";
                    break;
                case "3":
                    newRole = "vip";
                    break;
                default:
                    System.out.println("无效选择");
                    return;
            }
            
            String result = EmshopNativeInterface.setUserRole(userId, newRole);
            System.out.println("角色设置结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的用户ID");
        } catch (Exception e) {
            System.err.println("角色管理失败：" + e.getMessage());
        }
    }
    
    /**
     * 封禁用户
     */
    private static void banUser() {
        System.out.print("请输入要封禁的用户ID: ");
        try {
            long userId = Long.parseLong(scanner.nextLine());
            System.out.print("封禁原因: ");
            String reason = scanner.nextLine();
            
            String result = EmshopNativeInterface.banUser(userId, reason, currentAdminId);
            System.out.println("封禁结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的用户ID");
        } catch (Exception e) {
            System.err.println("封禁用户失败：" + e.getMessage());
        }
    }
    
    /**
     * 解封用户
     */
    private static void unbanUser() {
        System.out.print("请输入要解封的用户ID: ");
        try {
            long userId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.unbanUser(userId, currentAdminId);
            System.out.println("解封结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的用户ID");
        } catch (Exception e) {
            System.err.println("解封用户失败：" + e.getMessage());
        }
    }
    
    /**
     * 订单管理菜单
     */
    private static void orderManagement() {
        while (true) {
            System.out.println("\n--- 订单管理 ---");
            System.out.println("1. 查看所有订单");
            System.out.println("2. 按状态筛选订单");
            System.out.println("3. 查看订单详情");
            System.out.println("4. 更新订单状态");
            System.out.println("5. 发货管理");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewAllOrders();
                    break;
                case "2":
                    viewOrdersByStatus();
                    break;
                case "3":
                    viewOrderDetail();
                    break;
                case "4":
                    updateOrderStatus();
                    break;
                case "5":
                    shipOrderManagement();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看所有订单
     */
    private static void viewAllOrders() {
        try {
            String result = EmshopNativeInterface.getAllOrders("all", 1, 20, "", "");
            System.out.println("\n订单列表：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取订单列表失败：" + e.getMessage());
        }
    }
    
    /**
     * 按状态查看订单
     */
    private static void viewOrdersByStatus() {
        System.out.println("\n订单状态选项：");
        System.out.println("1. pending - 待支付");
        System.out.println("2. paid - 已支付");
        System.out.println("3. shipping - 配送中");
        System.out.println("4. delivered - 已送达");
        System.out.println("5. cancelled - 已取消");
        System.out.print("请选择状态 (1-5): ");
        
        String statusChoice = scanner.nextLine();
        String status = "";
        
        switch (statusChoice) {
            case "1": status = "pending"; break;
            case "2": status = "paid"; break;
            case "3": status = "shipping"; break;
            case "4": status = "delivered"; break;
            case "5": status = "cancelled"; break;
            default:
                System.out.println("无效选择");
                return;
        }
        
        try {
            String result = EmshopNativeInterface.getAllOrders(status, 1, 20, "", "");
            System.out.println("\n" + status + " 状态的订单：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取订单列表失败：" + e.getMessage());
        }
    }
    
    /**
     * 查看订单详情
     */
    private static void viewOrderDetail() {
        System.out.print("请输入订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.getOrderDetail(orderId);
            System.out.println("\n订单详情：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的订单ID");
        } catch (Exception e) {
            System.err.println("获取订单详情失败：" + e.getMessage());
        }
    }
    
    /**
     * 更新订单状态
     */
    private static void updateOrderStatus() {
        System.out.print("请输入订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            
            System.out.println("\n可选状态：");
            System.out.println("1. paid - 已支付");
            System.out.println("2. shipping - 配送中");
            System.out.println("3. delivered - 已送达");
            System.out.println("4. cancelled - 已取消");
            System.out.print("请选择新状态 (1-4): ");
            
            String statusChoice = scanner.nextLine();
            String newStatus = "";
            
            switch (statusChoice) {
                case "1": newStatus = "paid"; break;
                case "2": newStatus = "shipping"; break;
                case "3": newStatus = "delivered"; break;
                case "4": newStatus = "cancelled"; break;
                default:
                    System.out.println("无效选择");
                    return;
            }
            
            String result = EmshopNativeInterface.updateOrderStatus(orderId, newStatus);
            System.out.println("状态更新结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的订单ID");
        } catch (Exception e) {
            System.err.println("更新订单状态失败：" + e.getMessage());
        }
    }
    
    /**
     * 发货管理
     */
    private static void shipOrderManagement() {
        System.out.print("请输入订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            System.out.print("快递单号: ");
            String trackingNumber = scanner.nextLine();
            System.out.print("配送方式: ");
            String shippingMethod = scanner.nextLine();
            
            String result = EmshopNativeInterface.shipOrder(orderId, trackingNumber, shippingMethod);
            System.out.println("发货结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的订单ID");
        } catch (Exception e) {
            System.err.println("发货操作失败：" + e.getMessage());
        }
    }
    
    /**
     * 系统报告菜单
     */
    private static void systemReports() {
        System.out.println("\n--- 系统报告 ---");
        System.out.println("1. 销售统计");
        System.out.println("2. 用户统计");
        System.out.println("3. 库存统计");
        System.out.println("0. 返回主菜单");
        System.out.print("请选择: ");
        
        String choice = scanner.nextLine();
        
        switch (choice) {
            case "1":
                systemStatisticsReport();
                break;
            case "2":
                salesStatistics();
                break;
            case "3":
                inventoryReport();
                break;
            case "0":
                return;
            default:
                System.out.println("无效选择");
        }
    }
    
    /**
     * 系统统计报告
     */
    private static void systemStatisticsReport() {
        System.out.println("\n统计周期选择：");
        System.out.println("1. 今日");
        System.out.println("2. 本周");
        System.out.println("3. 本月");
        System.out.println("4. 本年");
        System.out.print("请选择周期 (1-4): ");
        
        String periodChoice = scanner.nextLine();
        String period = "";
        
        switch (periodChoice) {
            case "1": period = "day"; break;
            case "2": period = "week"; break;
            case "3": period = "month"; break;
            case "4": period = "year"; break;
            default:
                System.out.println("无效选择");
                return;
        }
        
        try {
            String result = EmshopNativeInterface.getSystemStatistics(period);
            System.out.println("\n系统统计报告：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取系统统计失败：" + e.getMessage());
        }
    }
    
    /**
     * 销售统计
     */
    private static void salesStatistics() {
        System.out.print("开始日期 (YYYY-MM-DD): ");
        String startDate = scanner.nextLine();
        System.out.print("结束日期 (YYYY-MM-DD): ");
        String endDate = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.getSalesStatistics(startDate, endDate);
            System.out.println("\n销售统计：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取销售统计失败：" + e.getMessage());
        }
    }
    
    /**
     * 库存报告
     */
    private static void inventoryReport() {
        System.out.print("低库存阈值 (默认10): ");
        String thresholdStr = scanner.nextLine();
        int threshold = thresholdStr.isEmpty() ? 10 : Integer.parseInt(thresholdStr);
        
        try {
            String result = EmshopNativeInterface.getLowStockProducts(threshold);
            System.out.println("\n库存报告：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取库存报告失败：" + e.getMessage());
        }
    }
    
    /**
     * 系统设置菜单
     */
    private static void systemSettings() {
        while (true) {
            System.out.println("\n--- 系统设置 ---");
            System.out.println("1. 查看系统状态");
            System.out.println("2. 查看系统日志");
            System.out.println("3. 查看系统指标");
            System.out.println("4. 清理缓存");
            System.out.println("5. 数据库管理");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewSystemStatus();
                    break;
                case "2":
                    viewSystemLogs();
                    break;
                case "3":
                    viewSystemMetrics();
                    break;
                case "4":
                    clearSystemCache();
                    break;
                case "5":
                    databaseManagement();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看系统状态
     */
    private static void viewSystemStatus() {
        try {
            String result = EmshopNativeInterface.getServerStatus();
            System.out.println("\n系统状态：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取系统状态失败：" + e.getMessage());
        }
    }
    
    /**
     * 查看系统日志
     */
    private static void viewSystemLogs() {
        System.out.println("\n日志级别选择：");
        System.out.println("1. ERROR - 错误日志");
        System.out.println("2. WARN - 警告日志");
        System.out.println("3. INFO - 信息日志");
        System.out.println("4. DEBUG - 调试日志");
        System.out.println("5. ALL - 所有日志");
        System.out.print("请选择日志级别 (1-5): ");
        
        String levelChoice = scanner.nextLine();
        String logLevel = "";
        
        switch (levelChoice) {
            case "1": logLevel = "ERROR"; break;
            case "2": logLevel = "WARN"; break;
            case "3": logLevel = "INFO"; break;
            case "4": logLevel = "DEBUG"; break;
            case "5": logLevel = "ALL"; break;
            default:
                System.out.println("无效选择");
                return;
        }
        
        try {
            String result = EmshopNativeInterface.getSystemLogs(logLevel, 1, 20);
            System.out.println("\n系统日志：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取系统日志失败：" + e.getMessage());
        }
    }
    
    /**
     * 查看系统指标
     */
    private static void viewSystemMetrics() {
        try {
            String result = EmshopNativeInterface.getSystemMetrics();
            System.out.println("\n系统指标：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取系统指标失败：" + e.getMessage());
        }
    }
    
    /**
     * 清理系统缓存
     */
    private static void clearSystemCache() {
        System.out.println("\n缓存类型选择：");
        System.out.println("1. 商品缓存");
        System.out.println("2. 用户缓存");
        System.out.println("3. 订单缓存");
        System.out.println("4. 所有缓存");
        System.out.print("请选择缓存类型 (1-4): ");
        
        String cacheChoice = scanner.nextLine();
        String cacheType = "";
        
        switch (cacheChoice) {
            case "1": cacheType = "product"; break;
            case "2": cacheType = "user"; break;
            case "3": cacheType = "order"; break;
            case "4": cacheType = "all"; break;
            default:
                System.out.println("无效选择");
                return;
        }
        
        try {
            String result = EmshopNativeInterface.clearCache(cacheType);
            System.out.println("清理缓存结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("清理缓存失败：" + e.getMessage());
        }
    }
    
    /**
     * 数据库管理
     */
    private static void databaseManagement() {
        while (true) {
            System.out.println("\n--- 数据库管理 ---");
            System.out.println("1. 查看数据库模式");
            System.out.println("2. 执行查询");
            System.out.println("3. 数据库备份");
            System.out.println("0. 返回上级菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewDatabaseSchema();
                    break;
                case "2":
                    executeQuery();
                    break;
                case "3":
                    System.out.println("数据库备份功能开发中...");
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看数据库模式
     */
    private static void viewDatabaseSchema() {
        try {
            String result = EmshopNativeInterface.getDatabaseSchema();
            System.out.println("\n数据库模式：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取数据库模式失败：" + e.getMessage());
        }
    }
    
    /**
     * 执行数据库查询
     */
    private static void executeQuery() {
        System.out.println("\n注意：请小心执行数据库操作，建议只执行SELECT查询");
        System.out.print("请输入SQL查询: ");
        String sql = scanner.nextLine();
        
        if (sql.trim().toLowerCase().startsWith("select")) {
            try {
                String result = EmshopNativeInterface.executeSelectQuery(sql, "{}");
                System.out.println("\n查询结果：");
                printFormattedResponse(result);
            } catch (Exception e) {
                System.err.println("查询执行失败：" + e.getMessage());
            }
        } else {
            System.out.print("非SELECT查询可能会修改数据，确认执行? (y/N): ");
            String confirm = scanner.nextLine();
            
            if ("y".equalsIgnoreCase(confirm)) {
                try {
                    String result = EmshopNativeInterface.executeDMLQuery(sql, "{}");
                    System.out.println("\n执行结果：");
                    printFormattedResponse(result);
                } catch (Exception e) {
                    System.err.println("查询执行失败：" + e.getMessage());
                }
            } else {
                System.out.println("取消执行");
            }
        }
    }
    
    // 辅助方法
    private static long extractUserIdFromResponse(String response) {
        int userIdIndex = response.indexOf("\"user_id\":");
        if (userIdIndex != -1) {
            int start = userIdIndex + 10;
            int end = response.indexOf(',', start);
            if (end == -1) end = response.indexOf('}', start);
            if (end != -1) {
                String userIdStr = response.substring(start, end).trim();
                return Long.parseLong(userIdStr);
            }
        }
        return 0;
    }
    
    private static String extractMessageFromResponse(String response) {
        int messageIndex = response.indexOf("\"message\":\"");
        if (messageIndex != -1) {
            int start = messageIndex + 11;
            int end = response.indexOf('"', start);
            if (end != -1) {
                return response.substring(start, end);
            }
        }
        return response; // 如果解析失败，返回原始响应
    }
    
    private static void printFormattedResponse(String response) {
        // 简单的格式化输出，实际项目中建议使用JSON库
        if (response.contains("\"success\":true")) {
            System.out.println("✓ 操作成功");
        } else {
            System.out.println("✗ 操作失败");
        }
        System.out.println("详细信息: " + response);
    }
}
