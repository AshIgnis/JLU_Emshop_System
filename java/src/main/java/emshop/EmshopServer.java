package emshop;

/**
 * Emshop业务服务类
 * 提供协议分发功能，直接调用JNI接口连接C++后端
 */
public class EmshopServer {
    
    /**
     * 协议分发方法
     * 根据客户端请求内容调用对应的JNI业务方法
     * @param request 客户端请求字符串
     * @return 业务处理结果
     */
    public static String dispatch(String request) {
        if (request == null || request.trim().isEmpty()) {
            return "{\"error\": \"Empty request\"}";
        }

        String[] parts = request.trim().split("\\s+");
        String command = parts[0].toUpperCase();

        try {
            switch (command) {
                // 用户管理相关命令
                case "LOGIN":
                    if (parts.length >= 3) {
                        return EmshopNativeInterface.login(parts[1], parts[2]);
                    } else {
                        return "{\"error\": \"LOGIN requires username and password\"}";
                    }

                case "REGISTER":
                    if (parts.length >= 4) {
                        return EmshopNativeInterface.register(parts[1], parts[2], parts[3]);
                    } else {
                        return "{\"error\": \"REGISTER requires username, password and phone\"}";
                    }

                case "LOGOUT":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.logout(userId);
                    } else {
                        return "{\"error\": \"LOGOUT requires userId\"}";
                    }

                case "GET_USER_INFO":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.getUserInfo(userId);
                    } else {
                        return "{\"error\": \"GET_USER_INFO requires userId\"}";
                    }

                case "UPDATE_USER_INFO":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        String jsonInfo = request.substring(request.indexOf(parts[2]));
                        return EmshopNativeInterface.updateUserInfo(userId, jsonInfo);
                    } else {
                        return "{\"error\": \"UPDATE_USER_INFO requires userId and jsonInfo\"}";
                    }

                // 商品管理相关命令
                case "GET_PRODUCTS":
                    if (parts.length >= 4) {
                        String category = parts[1];
                        int page = Integer.parseInt(parts[2]);
                        int pageSize = Integer.parseInt(parts[3]);
                        return EmshopNativeInterface.getProductList(category, page, pageSize);
                    } else {
                        return "{\"error\": \"GET_PRODUCTS requires category, page and pageSize\"}";
                    }

                case "GET_PRODUCT_DETAIL":
                    if (parts.length >= 2) {
                        long productId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.getProductDetail(productId);
                    } else {
                        return "{\"error\": \"GET_PRODUCT_DETAIL requires productId\"}";
                    }

                case "ADD_PRODUCT":
                    if (parts.length >= 2) {
                        String jsonProduct = request.substring(request.indexOf(parts[1]));
                        return EmshopNativeInterface.addProduct(jsonProduct);
                    } else {
                        return "{\"error\": \"ADD_PRODUCT requires jsonProduct\"}";
                    }

                case "UPDATE_PRODUCT":
                    if (parts.length >= 3) {
                        long productId = Long.parseLong(parts[1]);
                        String jsonProduct = request.substring(request.indexOf(parts[2]));
                        return EmshopNativeInterface.updateProduct(productId, jsonProduct);
                    } else {
                        return "{\"error\": \"UPDATE_PRODUCT requires productId and jsonProduct\"}";
                    }

                case "DELETE_PRODUCT":
                    if (parts.length >= 2) {
                        long productId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.deleteProduct(productId);
                    } else {
                        return "{\"error\": \"DELETE_PRODUCT requires productId\"}";
                    }

                case "GET_CATEGORIES":
                    return EmshopNativeInterface.getCategories();

                case "SEARCH_PRODUCTS":
                    if (parts.length >= 6) {
                        String keyword = parts[1];
                        int page = Integer.parseInt(parts[2]);
                        int pageSize = Integer.parseInt(parts[3]);
                        String sortBy = parts[4];
                        double minPrice = Double.parseDouble(parts[5]);
                        double maxPrice = parts.length > 6 ? Double.parseDouble(parts[6]) : Double.MAX_VALUE;
                        return EmshopNativeInterface.searchProducts(keyword, page, pageSize, sortBy, minPrice, maxPrice);
                    } else {
                        return "{\"error\": \"SEARCH_PRODUCTS requires keyword, page, pageSize, sortBy, minPrice [maxPrice]\"}";
                    }

                // 购物车管理相关命令
                case "ADD_TO_CART":
                    if (parts.length >= 4) {
                        long userId = Long.parseLong(parts[1]);
                        long productId = Long.parseLong(parts[2]);
                        int quantity = Integer.parseInt(parts[3]);
                        return EmshopNativeInterface.addToCart(userId, productId, quantity);
                    } else {
                        return "{\"error\": \"ADD_TO_CART requires userId, productId and quantity\"}";
                    }

                case "GET_CART":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.getCart(userId);
                    } else {
                        return "{\"error\": \"GET_CART requires userId\"}";
                    }

                case "REMOVE_FROM_CART":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        long productId = Long.parseLong(parts[2]);
                        return EmshopNativeInterface.removeFromCart(userId, productId);
                    } else {
                        return "{\"error\": \"REMOVE_FROM_CART requires userId and productId\"}";
                    }

                case "CLEAR_CART":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.clearCart(userId);
                    } else {
                        return "{\"error\": \"CLEAR_CART requires userId\"}";
                    }

                // 订单管理相关命令
                case "CHECKOUT":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.checkout(userId);
                    } else {
                        return "{\"error\": \"CHECKOUT requires userId\"}";
                    }

                case "GET_ORDERS":
                    if (parts.length >= 2) {
                        long userId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.getOrderList(userId);
                    } else {
                        return "{\"error\": \"GET_ORDERS requires userId\"}";
                    }

                case "GET_ORDER_DETAIL":
                    if (parts.length >= 2) {
                        long orderId = Long.parseLong(parts[1]);
                        return EmshopNativeInterface.getOrderDetail(orderId);
                    } else {
                        return "{\"error\": \"GET_ORDER_DETAIL requires orderId\"}";
                    }

                case "CANCEL_ORDER":
                    if (parts.length >= 3) {
                        long userId = Long.parseLong(parts[1]);
                        long orderId = Long.parseLong(parts[2]);
                        return EmshopNativeInterface.cancelOrder(userId, orderId);
                    } else {
                        return "{\"error\": \"CANCEL_ORDER requires userId and orderId\"}";
                    }

                // 系统状态命令
                case "STATUS":
                    return EmshopNativeInterface.getServerStatus();

                default:
                    return "{\"error\": \"Unknown command: " + command + "\"}";
            }
        } catch (NumberFormatException e) {
            return "{\"error\": \"Invalid number format: " + e.getMessage() + "\"}";
        } catch (UnsatisfiedLinkError e) {
            return "{\"error\": \"JNI library not loaded: " + e.getMessage() + "\"}";
        } catch (Exception e) {
            return "{\"error\": \"Internal error: " + e.getMessage() + "\"}";
        }
    }
}
