package emshop;
/**
 * Emshop业务服务类
 * 提供所有业务方法的native声明和协议分发功能
 * 与Netty服务器配合使用
 */
public class EmshopServer {
    
    /**
     * 协议分发方法
     * 根据客户端请求内容调用对应的native业务方法
     * @param request 客户端请求字符串
     * @return 业务处理结果
     */
    public static String dispatch(String request) {
        if (request == null || request.trim().isEmpty()) {
            return "ERROR: Empty request";
        }

        String[] parts = request.trim().split("\\s+");
        String command = parts[0].toUpperCase();

        try {
            switch (command) {
                // 用户管理相关命令
                case "LOGIN":
                    if (parts.length >= 3) {
                        return login(parts[1], parts[2]);
                    } else {
                        return "ERROR: LOGIN requires username and password";
                    }

                case "REGISTER":
                    if (parts.length >= 4) {
                        return register(parts[1], parts[2], parts[3]);
                    } else {
                        return "ERROR: REGISTER requires username, password and phone";
                    }

                case "LOGOUT":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return logout(userId);
                    } else {
                        return "ERROR: LOGOUT requires userId";
                    }

                case "GET_USER_INFO":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return getUserInfo(userId);
                    } else {
                        return "ERROR: GET_USER_INFO requires userId";
                    }

                case "UPDATE_USER_INFO":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        // 剩余部分作为JSON信息
                        String jsonInfo = request.substring(request.indexOf(parts[2]));
                        return updateUserInfo(userId, jsonInfo);
                    } else {
                        return "ERROR: UPDATE_USER_INFO requires userId and jsonInfo";
                    }

                // 商品管理相关命令
                case "GET_PRODUCTS":
                    if (parts.length >= 4) {
                        String category = parts[1];
                        int page = Integer.parseInt(parts[2]);
                        int pageSize = Integer.parseInt(parts[3]);
                        return getProductList(category, page, pageSize);
                    } else {
                        return "ERROR: GET_PRODUCTS requires category, page and pageSize";
                    }

                case "GET_PRODUCT_DETAIL":
                    if (parts.length >= 2) {
                        long productId = Long.parseLong(parts[1]);
                        return getProductDetail(productId);
                    } else {
                        return "ERROR: GET_PRODUCT_DETAIL requires productId";
                    }

                case "ADD_PRODUCT":
                    if (parts.length >= 2) {
                        String jsonProduct = request.substring(request.indexOf(parts[1]));
                        return addProduct(jsonProduct);
                    } else {
                        return "ERROR: ADD_PRODUCT requires jsonProduct";
                    }

                case "UPDATE_PRODUCT":
                    if (parts.length >= 3) {
                        long productId = Long.parseLong(parts[1]);
                        String jsonProduct = request.substring(request.indexOf(parts[2]));
                        return updateProduct(productId, jsonProduct);
                    } else {
                        return "ERROR: UPDATE_PRODUCT requires productId and jsonProduct";
                    }

                case "DELETE_PRODUCT":
                    if (parts.length >= 2) {
                        long productId = Long.parseLong(parts[1]);
                        return deleteProduct(productId);
                    } else {
                        return "ERROR: DELETE_PRODUCT requires productId";
                    }

                // 购物车相关命令
                case "ADD_TO_CART":
                    if (parts.length >= 4) {
                        long userId = Long.parseLong(parts[1]);
                        long productId = Long.parseLong(parts[2]);
                        int quantity = Integer.parseInt(parts[3]);
                        return addToCart(userId, productId, quantity);
                    } else {
                        return "ERROR: ADD_TO_CART requires userId, productId and quantity";
                    }

                case "GET_CART":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return getCart(userId);
                    } else {
                        return "ERROR: GET_CART requires userId";
                    }

                case "REMOVE_FROM_CART":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        long productId = Long.parseLong(parts[2]);
                        return removeFromCart(userId, productId);
                    } else {
                        return "ERROR: REMOVE_FROM_CART requires userId and productId";
                    }

                // 结算与订单相关命令
                case "CHECKOUT":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return checkout(userId);
                    } else {
                        return "ERROR: CHECKOUT requires userId";
                    }

                case "GET_ORDERS":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return getOrderList(userId);
                    } else {
                        return "ERROR: GET_ORDERS requires userId";
                    }

                case "GET_ORDER_DETAIL":
                    if (parts.length >= 2) {
                        long orderId = Long.parseLong(parts[1]);
                        return getOrderDetail(orderId);
                    } else {
                        return "ERROR: GET_ORDER_DETAIL requires orderId";
                    }

                case "CANCEL_ORDER":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        long orderId = Long.parseLong(parts[2]);
                        return cancelOrder(userId, orderId);
                    } else {
                        return "ERROR: CANCEL_ORDER requires userId and orderId";
                    }

                // 系统相关命令
                case "GET_SERVER_STATUS":
                    return getServerStatus();

                case "HELP":
                    return getHelpMessage();

                default:
                    return "ERROR: Unknown command: " + command + ". Type HELP for available commands.";
            }
        } catch (NumberFormatException e) {
            return "ERROR: Invalid number format in request";
        } catch (Exception e) {
            return "ERROR: " + e.getMessage();
        }
    }

    /**
     * 获取帮助信息
     */
    private static String getHelpMessage() {
        StringBuilder help = new StringBuilder();
        help.append("Available Commands:\n");
        help.append("User Management:\n");
        help.append("  LOGIN <username> <password>\n");
        help.append("  REGISTER <username> <password> <phone>\n");
        help.append("  LOGOUT <userId>\n");
        help.append("  GET_USER_INFO <userId>\n");
        help.append("\nProduct Management:\n");
        help.append("  GET_PRODUCTS <category> <page> <pageSize>\n");
        help.append("  GET_PRODUCT_DETAIL <productId>\n");
        help.append("\nShopping Cart:\n");
        help.append("  ADD_TO_CART <userId> <productId> <quantity>\n");
        help.append("  GET_CART <userId>\n");
        help.append("  REMOVE_FROM_CART <userId> <productId>\n");
        help.append("\nOrders:\n");
        help.append("  CHECKOUT <userId>\n");
        help.append("  GET_ORDERS <userId>\n");
        help.append("  GET_ORDER_DETAIL <orderId>\n");
        help.append("  CANCEL_ORDER <userId> <orderId>\n");
        help.append("\nSystem:\n");
        help.append("  GET_SERVER_STATUS\n");
        help.append("  HELP\n");
        return help.toString();
    }

    // ================== Native方法声明 ==================
    // 需要在C++中实现这些方法

    // 标记是否使用模拟数据
    private static boolean useMockData = true;

    // 用户管理
    public static String login(String username, String password) {
        if (useMockData) {
            return MockDataProvider.mockLogin(username, password);
        }
        return nativeLogin(username, password);
    }
    
    public static String register(String username, String password, String phone) {
        if (useMockData) {
            return MockDataProvider.mockRegister(username, password, phone);
        }
        return nativeRegister(username, password, phone);
    }
    
    public static String logout(long userId) {
        if (useMockData) {
            return MockDataProvider.mockLogout(userId);
        }
        return nativeLogout(userId);
    }
    
    public static String getUserInfo(long userId) {
        if (useMockData) {
            return MockDataProvider.mockGetUserInfo(userId);
        }
        return nativeGetUserInfo(userId);
    }
    
    public static String updateUserInfo(long userId, String jsonInfo) {
        if (useMockData) {
            return MockDataProvider.mockError("updateUserInfo");
        }
        return nativeUpdateUserInfo(userId, jsonInfo);
    }

    // Native方法声明
    public static native String nativeLogin(String username, String password);
    public static native String nativeRegister(String username, String password, String phone);
    public static native String nativeLogout(long userId);
    public static native String nativeGetUserInfo(long userId);
    public static native String nativeUpdateUserInfo(long userId, String jsonInfo);

    // 商品管理
    public static String getProductList(String category, int page, int pageSize) {
        if (useMockData) {
            return MockDataProvider.mockGetProductList(category, page, pageSize);
        }
        return nativeGetProductList(category, page, pageSize);
    }
    
    public static String getProductDetail(long productId) {
        if (useMockData) {
            return MockDataProvider.mockGetProductDetail(productId);
        }
        return nativeGetProductDetail(productId);
    }
    
    public static String addProduct(String jsonProduct) {
        if (useMockData) {
            return MockDataProvider.mockError("addProduct");
        }
        return nativeAddProduct(jsonProduct);
    }
    
    public static String updateProduct(long productId, String jsonProduct) {
        if (useMockData) {
            return MockDataProvider.mockError("updateProduct");
        }
        return nativeUpdateProduct(productId, jsonProduct);
    }
    
    public static String deleteProduct(long productId) {
        if (useMockData) {
            return MockDataProvider.mockDeleteProduct(productId);
        }
        return nativeDeleteProduct(productId);
    }

    public static native String nativeGetProductList(String category, int page, int pageSize);
    public static native String nativeGetProductDetail(long productId);
    public static native String nativeAddProduct(String jsonProduct);
    public static native String nativeUpdateProduct(long productId, String jsonProduct);
    public static native String nativeDeleteProduct(long productId);

    // 购物车
    public static String addToCart(long userId, long productId, int quantity) {
        if (useMockData) {
            return MockDataProvider.mockAddToCart(userId, productId, quantity);
        }
        return nativeAddToCart(userId, productId, quantity);
    }
    
    public static String getCart(long userId) {
        if (useMockData) {
            return MockDataProvider.mockGetCart(userId);
        }
        return nativeGetCart(userId);
    }
    
    public static String removeFromCart(long userId, long productId) {
        if (useMockData) {
            return MockDataProvider.mockRemoveFromCart(userId, productId);
        }
        return nativeRemoveFromCart(userId, productId);
    }

    public static native String nativeAddToCart(long userId, long productId, int quantity);
    public static native String nativeGetCart(long userId);
    public static native String nativeRemoveFromCart(long userId, long productId);

    // 结算与订单
    public static String checkout(long userId) {
        if (useMockData) {
            return MockDataProvider.mockCheckout(userId);
        }
        return nativeCheckout(userId);
    }
    
    public static String getOrderList(long userId) {
        if (useMockData) {
            return MockDataProvider.mockGetOrderList(userId);
        }
        return nativeGetOrderList(userId);
    }
    
    public static String getOrderDetail(long orderId) {
        if (useMockData) {
            return MockDataProvider.mockGetOrderDetail(orderId);
        }
        return nativeGetOrderDetail(orderId);
    }
    
    public static String cancelOrder(long userId, long orderId) {
        if (useMockData) {
            return MockDataProvider.mockCancelOrder(userId, orderId);
        }
        return nativeCancelOrder(userId, orderId);
    }

    public static native String nativeCheckout(long userId);
    public static native String nativeGetOrderList(long userId);
    public static native String nativeGetOrderDetail(long orderId);
    public static native String nativeCancelOrder(long userId, long orderId);

    // 系统相关
    public static String getServerStatus() {
        if (useMockData) {
            return MockDataProvider.mockGetServerStatus();
        }
        return nativeGetServerStatus();
    }

    public static native String nativeGetServerStatus();

    // 售后服务
    public static native String afterSale(long userId, long orderId, String reason);

    // 促销策略
    public static native String applyPromotion(long userId, String promoCode);

    // UI风格
    public static native String getAvailableThemes();
    public static native String setTheme(long userId, String themeName);

    // 并发与互斥
    public static native String lockProduct(long productId);
    public static native String unlockProduct(long productId);

    // 数据库操作/映射
    public static native String executeSQL(String sql);

    // 日志与监控
    public static native String getSystemLog(int page, int pageSize);

    // 加载本地库
    static {
        try {
            System.loadLibrary("emshop"); // 需要对应的C++ DLL
            useMockData = false; // 成功加载native库，使用真实数据
            System.out.println("Native library 'emshop' loaded successfully.");
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Warning: Native library 'emshop' not found. Using mock responses.");
            useMockData = true; // 使用模拟数据
            // 在开发阶段，如果没有native库，可以使用模拟数据
        }
    }
}
