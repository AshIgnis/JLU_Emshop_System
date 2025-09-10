package emshop;

/**
 * 权限管理服务类
 * 负责管理用户角色和权限验证
 */
public class PermissionService {
    
    // 用户角色常量
    public static final String ROLE_ADMIN = "admin";
    public static final String ROLE_USER = "user";
    public static final String ROLE_VIP = "vip";
    
    // 权限常量
    public static final String PERMISSION_MANAGE_PRODUCTS = "manage_products";
    public static final String PERMISSION_MANAGE_INVENTORY = "manage_inventory";
    public static final String PERMISSION_MANAGE_USERS = "manage_users";
    public static final String PERMISSION_MANAGE_ORDERS = "manage_orders";
    public static final String PERMISSION_VIEW_REPORTS = "view_reports";
    public static final String PERMISSION_SYSTEM_CONFIG = "system_config";
    
    /**
     * 检查用户是否为管理员
     */
    public static boolean isAdmin(long userId) {
        try {
            String result = EmshopNativeInterface.verifyAdminPermission(userId);
            // 解析JSON响应
            return result.contains("\"success\":true") && result.contains("\"is_admin\":true");
        } catch (Exception e) {
            System.err.println("检查管理员权限失败: " + e.getMessage());
            return false;
        }
    }
    
    /**
     * 检查用户是否有指定权限
     */
    public static boolean hasPermission(long userId, String permission) {
        try {
            String result = EmshopNativeInterface.checkUserPermission(userId, permission);
            return result.contains("\"success\":true") && result.contains("\"has_permission\":true");
        } catch (Exception e) {
            System.err.println("检查用户权限失败: " + e.getMessage());
            return false;
        }
    }
    
    /**
     * 获取用户角色
     */
    public static String getUserRole(long userId) {
        try {
            String result = EmshopNativeInterface.getUserRoles(userId);
            // 简单的JSON解析，实际项目中建议使用JSON库
            if (result.contains("\"role\":\"admin\"")) {
                return ROLE_ADMIN;
            } else if (result.contains("\"role\":\"vip\"")) {
                return ROLE_VIP;
            } else {
                return ROLE_USER;
            }
        } catch (Exception e) {
            System.err.println("获取用户角色失败: " + e.getMessage());
            return ROLE_USER; // 默认为普通用户
        }
    }
    
    /**
     * 验证管理员操作权限
     */
    public static String validateAdminOperation(long userId, String operation) {
        if (!isAdmin(userId)) {
            return createErrorResponse("权限不足：需要管理员权限才能执行此操作");
        }
        return null; // null表示验证通过
    }
    
    /**
     * 验证库存管理权限
     */
    public static String validateInventoryPermission(long userId) {
        if (!hasPermission(userId, PERMISSION_MANAGE_INVENTORY)) {
            return createErrorResponse("权限不足：没有库存管理权限");
        }
        return null;
    }
    
    /**
     * 验证商品管理权限
     */
    public static String validateProductPermission(long userId) {
        if (!hasPermission(userId, PERMISSION_MANAGE_PRODUCTS)) {
            return createErrorResponse("权限不足：没有商品管理权限");
        }
        return null;
    }
    
    /**
     * 验证用户管理权限
     */
    public static String validateUserManagementPermission(long userId) {
        if (!hasPermission(userId, PERMISSION_MANAGE_USERS)) {
            return createErrorResponse("权限不足：没有用户管理权限");
        }
        return null;
    }
    
    /**
     * 创建错误响应
     */
    private static String createErrorResponse(String message) {
        return "{\"success\":false,\"message\":\"" + message + "\",\"error_code\":403}";
    }
    
    /**
     * 获取用户的完整权限信息
     */
    public static String getUserPermissionInfo(long userId) {
        try {
            return EmshopNativeInterface.getUserDetailWithPermissions(userId);
        } catch (Exception e) {
            return createErrorResponse("获取用户权限信息失败: " + e.getMessage());
        }
    }
    
    /**
     * 验证用户权限并返回权限检查结果
     */
    public static PermissionCheckResult checkPermissionWithResult(long userId, String permission) {
        try {
            String result = EmshopNativeInterface.checkUserPermission(userId, permission);
            boolean hasPermission = result.contains("\"success\":true") && result.contains("\"has_permission\":true");
            return new PermissionCheckResult(hasPermission, hasPermission ? "权限验证通过" : "权限不足");
        } catch (Exception e) {
            return new PermissionCheckResult(false, "权限检查失败: " + e.getMessage());
        }
    }
    
    /**
     * 权限检查结果类
     */
    public static class PermissionCheckResult {
        private final boolean hasPermission;
        private final String message;
        
        public PermissionCheckResult(boolean hasPermission, String message) {
            this.hasPermission = hasPermission;
            this.message = message;
        }
        
        public boolean hasPermission() {
            return hasPermission;
        }
        
        public String getMessage() {
            return message;
        }
    }
    
    /**
     * 设置用户角色
     */
    public static String setUserRole(long userId, String role) {
        try {
            // 验证角色有效性
            if (!isValidRole(role)) {
                return createErrorResponse("无效的角色: " + role);
            }
            
            return EmshopNativeInterface.setUserRole(userId, role);
        } catch (Exception e) {
            return createErrorResponse("设置用户角色失败: " + e.getMessage());
        }
    }
    
    /**
     * 验证角色是否有效
     */
    private static boolean isValidRole(String role) {
        return ROLE_ADMIN.equals(role) || ROLE_USER.equals(role) || ROLE_VIP.equals(role);
    }
    
    /**
     * 获取角色的权限列表
     */
    public static String[] getRolePermissions(String role) {
        switch (role) {
            case ROLE_ADMIN:
                return new String[]{
                    PERMISSION_MANAGE_PRODUCTS,
                    PERMISSION_MANAGE_INVENTORY,
                    PERMISSION_MANAGE_USERS,
                    PERMISSION_MANAGE_ORDERS,
                    PERMISSION_VIEW_REPORTS,
                    PERMISSION_SYSTEM_CONFIG
                };
            case ROLE_VIP:
                return new String[]{"shop", "cart_management", "order_management", "vip_discounts"};
            case ROLE_USER:
            default:
                return new String[]{"shop", "cart_management", "order_management"};
        }
    }
    
    /**
     * 检查角色是否拥有指定权限
     */
    public static boolean roleHasPermission(String role, String permission) {
        String[] permissions = getRolePermissions(role);
        for (String p : permissions) {
            if (p.equals(permission)) {
                return true;
            }
        }
        return false;
    }
}
