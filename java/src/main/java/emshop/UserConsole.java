package emshop;

import java.util.Scanner;

/**
 * 普通用户界面 - 为普通用户提供的购物界面
 * 提供商品浏览、购物车管理、订单查看等用户功能
 */
public class UserConsole {
    
    private static Scanner scanner = new Scanner(System.in);
    private static long currentUserId = 0;
    private static String currentUsername = "";
    private static boolean isLoggedIn = false;
    
    public static void main(String[] args) {
        System.out.println("===============================================");
        System.out.println("    JLU Emshop System - 用户购物界面");
        System.out.println("===============================================");
        
        // 主菜单循环
        mainMenu();
    }
    
    /**
     * 主菜单
     */
    private static void mainMenu() {
        while (true) {
            if (!isLoggedIn) {
                loginMenu();
            } else {
                shoppingMenu();
            }
        }
    }
    
    /**
     * 登录菜单
     */
    private static void loginMenu() {
        System.out.println("\n===============================================");
        System.out.println("欢迎使用 JLU Emshop 购物系统");
        System.out.println("===============================================");
        System.out.println("1. 用户登录");
        System.out.println("2. 用户注册");
        System.out.println("3. 浏览商品（游客模式）");
        System.out.println("0. 退出系统");
        System.out.println("===============================================");
        System.out.print("请选择操作 (0-3): ");
        
        String choice = scanner.nextLine();
        
        switch (choice) {
            case "1":
                userLogin();
                break;
            case "2":
                userRegister();
                break;
            case "3":
                guestBrowse();
                break;
            case "0":
                System.out.println("感谢使用 JLU Emshop 购物系统，再见！");
                System.exit(0);
                break;
            default:
                System.out.println("无效选择，请重新输入");
        }
    }
    
    /**
     * 购物菜单（已登录用户）
     */
    private static void shoppingMenu() {
        System.out.println("\n===============================================");
        System.out.println("用户购物中心 - " + currentUsername);
        System.out.println("===============================================");
        System.out.println("1. 浏览商品");
        System.out.println("2. 搜索商品");
        System.out.println("3. 购物车管理");
        System.out.println("4. 我的订单");
        System.out.println("5. 个人中心");
        System.out.println("6. 登出");
        System.out.println("0. 退出系统");
        System.out.println("===============================================");
        System.out.print("请选择操作 (0-6): ");
        
        String choice = scanner.nextLine();
        
        switch (choice) {
            case "1":
                browseProducts();
                break;
            case "2":
                searchProducts();
                break;
            case "3":
                cartManagement();
                break;
            case "4":
                orderManagement();
                break;
            case "5":
                userProfile();
                break;
            case "6":
                logout();
                break;
            case "0":
                System.out.println("感谢使用 JLU Emshop 购物系统，再见！");
                System.exit(0);
                break;
            default:
                System.out.println("无效选择，请重新输入");
        }
    }
    
    /**
     * 用户登录
     */
    private static void userLogin() {
        System.out.println("\n--- 用户登录 ---");
        System.out.print("用户名: ");
        String username = scanner.nextLine();
        System.out.print("密码: ");
        String password = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.login(username, password);
            if (result.contains("\"success\":true")) {
                currentUserId = extractUserIdFromResponse(result);
                currentUsername = username;
                isLoggedIn = true;
                System.out.println("登录成功！欢迎，" + username);
            } else {
                System.out.println("登录失败：" + extractMessageFromResponse(result));
            }
        } catch (Exception e) {
            System.err.println("登录过程中发生错误：" + e.getMessage());
        }
    }
    
    /**
     * 用户注册
     */
    private static void userRegister() {
        System.out.println("\n--- 用户注册 ---");
        System.out.print("用户名: ");
        String username = scanner.nextLine();
        System.out.print("密码: ");
        String password = scanner.nextLine();
        System.out.print("手机号: ");
        String phone = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.register(username, password, phone);
            if (result.contains("\"success\":true")) {
                System.out.println("注册成功！请登录使用系统");
            } else {
                System.out.println("注册失败：" + extractMessageFromResponse(result));
            }
        } catch (Exception e) {
            System.err.println("注册过程中发生错误：" + e.getMessage());
        }
    }
    
    /**
     * 游客浏览
     */
    private static void guestBrowse() {
        System.out.println("\n--- 游客浏览模式 ---");
        System.out.println("1. 查看所有商品");
        System.out.println("2. 按分类浏览");
        System.out.println("0. 返回主菜单");
        System.out.print("请选择: ");
        
        String choice = scanner.nextLine();
        switch (choice) {
            case "1":
                viewAllProducts();
                break;
            case "2":
                browseByCategoryGuest();
                break;
            case "0":
                return;
            default:
                System.out.println("无效选择");
        }
    }
    
    /**
     * 浏览商品
     */
    private static void browseProducts() {
        System.out.println("\n--- 商品浏览 ---");
        System.out.println("1. 查看所有商品");
        System.out.println("2. 按分类浏览");
        System.out.println("3. 查看商品详情");
        System.out.println("0. 返回主菜单");
        System.out.print("请选择: ");
        
        String choice = scanner.nextLine();
        
        switch (choice) {
            case "1":
                viewAllProducts();
                break;
            case "2":
                browseByCategory();
                break;
            case "3":
                viewProductDetail();
                break;
            case "0":
                return;
            default:
                System.out.println("无效选择");
        }
    }
    
    /**
     * 查看所有商品
     */
    private static void viewAllProducts() {
        try {
            String result = EmshopNativeInterface.getProductList("all", 1, 20);
            System.out.println("\n商品列表：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取商品列表失败：" + e.getMessage());
        }
    }
    
    /**
     * 购物车管理
     */
    private static void cartManagement() {
        while (true) {
            System.out.println("\n--- 购物车管理 ---");
            System.out.println("1. 查看购物车");
            System.out.println("2. 添加商品到购物车");
            System.out.println("3. 更新商品数量");
            System.out.println("4. 移除商品");
            System.out.println("5. 清空购物车");
            System.out.println("6. 结算购物车");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewCart();
                    break;
                case "2":
                    addToCart();
                    break;
                case "3":
                    updateCartQuantity();
                    break;
                case "4":
                    removeFromCart();
                    break;
                case "5":
                    clearCart();
                    break;
                case "6":
                    checkout();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    /**
     * 查看购物车
     */
    private static void viewCart() {
        try {
            String result = EmshopNativeInterface.getCart(currentUserId);
            System.out.println("\n我的购物车：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取购物车失败：" + e.getMessage());
        }
    }
    
    /**
     * 添加商品到购物车
     */
    private static void addToCart() {
        System.out.print("请输入商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            System.out.print("请输入数量: ");
            int quantity = Integer.parseInt(scanner.nextLine());
            
            String result = EmshopNativeInterface.addToCart(currentUserId, productId, quantity);
            System.out.println("添加结果：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的数字格式");
        }
    }
    
    /**
     * 登出
     */
    private static void logout() {
        currentUserId = 0;
        currentUsername = "";
        isLoggedIn = false;
        System.out.println("已成功登出");
    }
    
    // 其他方法的占位符
    // 搜索商品功能
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
    
    // 订单管理功能
    private static void orderManagement() {
        while (true) {
            System.out.println("\n--- 我的订单 ---");
            System.out.println("1. 查看所有订单");
            System.out.println("2. 按状态查看订单");
            System.out.println("3. 查看订单详情");
            System.out.println("4. 取消订单");
            System.out.println("5. 确认收货");
            System.out.println("6. 申请退款");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewMyOrders();
                    break;
                case "2":
                    viewOrdersByStatus();
                    break;
                case "3":
                    viewOrderDetail();
                    break;
                case "4":
                    cancelOrder();
                    break;
                case "5":
                    confirmDelivery();
                    break;
                case "6":
                    requestRefund();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    private static void viewMyOrders() {
        try {
            String result = EmshopNativeInterface.getOrderList(currentUserId);
            System.out.println("\n我的订单：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取订单列表失败：" + e.getMessage());
        }
    }
    
    private static void viewOrdersByStatus() {
        System.out.println("\n订单状态选择：");
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
            String result = EmshopNativeInterface.getOrdersByStatus(currentUserId, status);
            System.out.println("\n" + status + " 状态的订单：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取订单列表失败：" + e.getMessage());
        }
    }
    
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
    
    private static void cancelOrder() {
        System.out.print("请输入要取消的订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.cancelOrder(currentUserId, orderId);
            System.out.println("取消订单结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的订单ID");
        } catch (Exception e) {
            System.err.println("取消订单失败：" + e.getMessage());
        }
    }
    
    private static void confirmDelivery() {
        System.out.print("请输入要确认收货的订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.completeOrder(orderId);
            System.out.println("确认收货结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的订单ID");
        } catch (Exception e) {
            System.err.println("确认收货失败：" + e.getMessage());
        }
    }
    
    private static void requestRefund() {
        System.out.print("请输入订单ID: ");
        try {
            long orderId = Long.parseLong(scanner.nextLine());
            System.out.print("退款原因: ");
            String reason = scanner.nextLine();
            
            String result = EmshopNativeInterface.requestRefund(orderId, currentUserId, reason);
            System.out.println("申请退款结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的输入格式");
        } catch (Exception e) {
            System.err.println("申请退款失败：" + e.getMessage());
        }
    }
    
    // 个人中心功能
    private static void userProfile() {
        while (true) {
            System.out.println("\n--- 个人中心 ---");
            System.out.println("1. 查看个人信息");
            System.out.println("2. 修改个人信息");
            System.out.println("3. 地址管理");
            System.out.println("4. 优惠券管理");
            System.out.println("5. 我的评价");
            System.out.println("0. 返回主菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewUserInfo();
                    break;
                case "2":
                    updateUserInfo();
                    break;
                case "3":
                    addressManagement();
                    break;
                case "4":
                    couponManagement();
                    break;
                case "5":
                    myReviews();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    private static void viewUserInfo() {
        try {
            String result = EmshopNativeInterface.getUserInfo(currentUserId);
            System.out.println("\n个人信息：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取个人信息失败：" + e.getMessage());
        }
    }
    
    private static void updateUserInfo() {
        System.out.println("个人信息修改功能开发中...");
    }
    
    private static void addressManagement() {
        while (true) {
            System.out.println("\n--- 地址管理 ---");
            System.out.println("1. 查看收货地址");
            System.out.println("2. 添加收货地址");
            System.out.println("3. 修改收货地址");
            System.out.println("4. 删除收货地址");
            System.out.println("5. 设置默认地址");
            System.out.println("0. 返回上级菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewAddresses();
                    break;
                case "2":
                    addAddress();
                    break;
                case "3":
                    updateAddress();
                    break;
                case "4":
                    deleteAddress();
                    break;
                case "5":
                    setDefaultAddress();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    private static void viewAddresses() {
        try {
            String result = EmshopNativeInterface.getUserAddresses(currentUserId);
            System.out.println("\n收货地址：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取地址列表失败：" + e.getMessage());
        }
    }
    
    private static void addAddress() {
        System.out.print("收货人姓名: ");
        String receiverName = scanner.nextLine();
        System.out.print("收货人电话: ");
        String phone = scanner.nextLine();
        System.out.print("省份: ");
        String province = scanner.nextLine();
        System.out.print("城市: ");
        String city = scanner.nextLine();
        System.out.print("区县: ");
        String district = scanner.nextLine();
        System.out.print("详细地址: ");
        String detailAddress = scanner.nextLine();
        System.out.print("邮政编码: ");
        String postalCode = scanner.nextLine();
        System.out.print("是否设为默认地址? (y/N): ");
        boolean isDefault = "y".equalsIgnoreCase(scanner.nextLine());
        
        try {
            String result = EmshopNativeInterface.addUserAddress(currentUserId, receiverName, phone, 
                    province, city, district, detailAddress, postalCode, isDefault);
            System.out.println("添加地址结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("添加地址失败：" + e.getMessage());
        }
    }
    
    private static void updateAddress() {
        System.out.println("修改地址功能开发中...");
    }
    
    private static void deleteAddress() {
        System.out.print("请输入要删除的地址ID: ");
        try {
            long addressId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.deleteUserAddress(addressId);
            System.out.println("删除地址结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的地址ID");
        } catch (Exception e) {
            System.err.println("删除地址失败：" + e.getMessage());
        }
    }
    
    private static void setDefaultAddress() {
        System.out.print("请输入要设为默认的地址ID: ");
        try {
            long addressId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.setDefaultAddress(currentUserId, addressId);
            System.out.println("设置默认地址结果：");
            printFormattedResponse(result);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的地址ID");
        } catch (Exception e) {
            System.err.println("设置默认地址失败：" + e.getMessage());
        }
    }
    
    private static void couponManagement() {
        while (true) {
            System.out.println("\n--- 优惠券管理 ---");
            System.out.println("1. 我的优惠券");
            System.out.println("2. 可领取的优惠券");
            System.out.println("3. 领取优惠券");
            System.out.println("0. 返回上级菜单");
            System.out.print("请选择: ");
            
            String choice = scanner.nextLine();
            
            switch (choice) {
                case "1":
                    viewMyCoupons();
                    break;
                case "2":
                    viewAvailableCoupons();
                    break;
                case "3":
                    claimCoupon();
                    break;
                case "0":
                    return;
                default:
                    System.out.println("无效选择");
            }
        }
    }
    
    private static void viewMyCoupons() {
        try {
            String result = EmshopNativeInterface.getUserCoupons(currentUserId);
            System.out.println("\n我的优惠券：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取优惠券失败：" + e.getMessage());
        }
    }
    
    private static void viewAvailableCoupons() {
        try {
            String result = EmshopNativeInterface.getAvailableCoupons();
            System.out.println("\n可领取的优惠券：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取可用优惠券失败：" + e.getMessage());
        }
    }
    
    private static void claimCoupon() {
        System.out.print("请输入优惠券代码: ");
        String couponCode = scanner.nextLine();
        
        try {
            String result = EmshopNativeInterface.claimCoupon(currentUserId, couponCode);
            System.out.println("领取优惠券结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("领取优惠券失败：" + e.getMessage());
        }
    }
    
    private static void myReviews() {
        try {
            String result = EmshopNativeInterface.getUserReviews(currentUserId, 1, 20);
            System.out.println("\n我的评价：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("获取评价列表失败：" + e.getMessage());
        }
    }
    
    private static void browseByCategoryGuest() {
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
    
    private static void browseByCategory() {
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
    
    private static void viewProductDetail() {
        System.out.print("请输入商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.getProductDetail(productId);
            System.out.println("\n商品详情：");
            printFormattedResponse(result);
            
            // 显示商品评论
            System.out.println("\n--- 商品评论 ---");
            String reviews = EmshopNativeInterface.getProductReviews(productId, 1, 5, "created_at");
            printFormattedResponse(reviews);
            
        } catch (NumberFormatException e) {
            System.out.println("无效的商品ID");
        } catch (Exception e) {
            System.err.println("获取商品详情失败：" + e.getMessage());
        }
    }
    
    private static void updateCartQuantity() {
        System.out.print("请输入商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            System.out.print("请输入新数量: ");
            int quantity = Integer.parseInt(scanner.nextLine());
            
            String result = EmshopNativeInterface.updateCartItemQuantity(currentUserId, productId, quantity);
            System.out.println("更新数量结果：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的数字格式");
        } catch (Exception e) {
            System.err.println("更新数量失败：" + e.getMessage());
        }
    }
    
    private static void removeFromCart() {
        System.out.print("请输入要移除的商品ID: ");
        try {
            long productId = Long.parseLong(scanner.nextLine());
            
            String result = EmshopNativeInterface.removeFromCart(currentUserId, productId);
            System.out.println("移除商品结果：");
            printFormattedResponse(result);
        } catch (NumberFormatException e) {
            System.out.println("无效的商品ID");
        } catch (Exception e) {
            System.err.println("移除商品失败：" + e.getMessage());
        }
    }
    
    private static void clearCart() {
        try {
            String result = EmshopNativeInterface.clearCart(currentUserId);
            System.out.println("清空购物车结果：");
            printFormattedResponse(result);
        } catch (Exception e) {
            System.err.println("清空购物车失败：" + e.getMessage());
        }
    }
    
    private static void checkout() {
        try {
            // 先查看购物车
            String cartResult = EmshopNativeInterface.getCart(currentUserId);
            System.out.println("\n购物车内容：");
            printFormattedResponse(cartResult);
            
            // 选择收货地址
            String addressResult = EmshopNativeInterface.getUserAddresses(currentUserId);
            System.out.println("\n收货地址：");
            printFormattedResponse(addressResult);
            
            System.out.print("请输入收货地址ID: ");
            long addressId = Long.parseLong(scanner.nextLine());
            
            System.out.print("优惠券代码 (可选，直接回车跳过): ");
            String couponCode = scanner.nextLine();
            if (couponCode.trim().isEmpty()) {
                couponCode = null;
            }
            
            System.out.print("订单备注 (可选): ");
            String remark = scanner.nextLine();
            
            // 创建订单
            String orderResult = EmshopNativeInterface.createOrderFromCart(currentUserId, addressId, couponCode, remark);
            System.out.println("\n订单创建结果：");
            printFormattedResponse(orderResult);
            
            // 如果订单创建成功，提供支付选项
            if (orderResult.contains("\"success\":true")) {
                System.out.println("\n支付方式选择：");
                System.out.println("1. 支付宝");
                System.out.println("2. 微信支付");
                System.out.println("3. 银行卡");
                System.out.print("请选择支付方式 (1-3): ");
                
                String paymentChoice = scanner.nextLine();
                String paymentMethod = "";
                
                switch (paymentChoice) {
                    case "1": paymentMethod = "alipay"; break;
                    case "2": paymentMethod = "wechat"; break;
                    case "3": paymentMethod = "bankcard"; break;
                    default:
                        System.out.println("无效选择，取消支付");
                        return;
                }
                
                // 这里应该从订单创建结果中提取订单ID，简化处理
                System.out.println("模拟支付成功！订单已创建，请在订单管理中查看详情。");
            }
            
        } catch (NumberFormatException e) {
            System.out.println("无效的数字格式");
        } catch (Exception e) {
            System.err.println("结算失败：" + e.getMessage());
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
        return response;
    }
    
    private static void printFormattedResponse(String response) {
        if (response.contains("\"success\":true")) {
            System.out.println("✓ 操作成功");
        } else {
            System.out.println("✗ 操作失败");
        }
        System.out.println("详细信息: " + response);
    }
}
