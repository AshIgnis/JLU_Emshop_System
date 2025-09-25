package emshop;

/**
 * 测试登录响应格式的简单程序
 */
public class TestLoginResponse {
    public static void main(String[] args) {
        try {
            // 测试登录并查看返回的JSON格式
            String result = EmshopNativeInterface.login("admin", "admin123");
            System.out.println("登录响应格式:");
            System.out.println(result);
            
            // 格式化打印JSON
            if (result.startsWith("{")) {
                System.out.println("\n格式化的JSON:");
                // 简单的JSON格式化显示
                String formatted = result.replace(",", ",\n  ")
                                         .replace("{", "{\n  ")
                                         .replace("}", "\n}");
                System.out.println(formatted);
            }
            
        } catch (Exception e) {
            System.err.println("测试失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
}