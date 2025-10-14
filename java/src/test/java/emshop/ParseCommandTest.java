package emshop;

/**
 * 测试 parseCommand 函数的转义处理
 */
public class ParseCommandTest {
    
    // 复制 parseCommand 方法用于测试
    private static String[] parseCommand(String command) {
        java.util.List<String> parts = new java.util.ArrayList<>();
        boolean inQuotes = false;
        StringBuilder current = new StringBuilder();
        boolean escaped = false;
        
        for (int i = 0; i < command.length(); i++) {
            char c = command.charAt(i);
            
            if (escaped) {
                // 处理转义字符
                if (c == '\\' || c == '"') {
                    current.append(c);
                } else {
                    // 如果不是有效的转义序列，保留反斜杠
                    current.append('\\').append(c);
                }
                escaped = false;
            } else if (c == '\\') {
                // 遇到反斜杠，标记为转义状态
                escaped = true;
            } else if (c == '"') {
                // 引号切换引用状态（不添加引号本身）
                inQuotes = !inQuotes;
            } else if (c == ' ' && !inQuotes) {
                // 空格且不在引号内，分割参数
                if (current.length() > 0) {
                    parts.add(current.toString());
                    current.setLength(0);
                }
            } else {
                // 普通字符
                current.append(c);
            }
        }
        
        // 添加最后一个参数
        if (current.length() > 0) {
            parts.add(current.toString());
        }
        
        return parts.toArray(new String[0]);
    }
    
    public static void main(String[] args) {
        System.out.println("=== 测试 parseCommand 函数 ===\n");
        
        // 测试1: 模拟 Qt 发送的 CREATE_PROMOTION 命令
        String test1 = "CREATE_PROMOTION \"{\\\"code\\\":\\\"5\\\",\\\"discount_type\\\":\\\"fixed_amount\\\",\\\"discount_value\\\":50,\\\"end_date\\\":\\\"2025-11-14 20:28:35\\\",\\\"min_amount\\\":0,\\\"name\\\":\\\"十一大促\\\",\\\"start_date\\\":\\\"2025-10-14 20:28:35\\\",\\\"status\\\":\\\"active\\\"}\"";
        
        System.out.println("测试1: CREATE_PROMOTION 命令");
        System.out.println("输入: " + test1);
        String[] parts1 = parseCommand(test1);
        System.out.println("parts.length = " + parts1.length);
        for (int i = 0; i < parts1.length; i++) {
            System.out.println("parts[" + i + "] = " + parts1[i]);
        }
        
        // 验证 JSON 是否可以解析
        if (parts1.length >= 2) {
            StringBuilder jsonBuilder = new StringBuilder();
            for (int i = 1; i < parts1.length; i++) {
                if (i > 1) jsonBuilder.append(" ");
                jsonBuilder.append(parts1[i]);
            }
            String json = jsonBuilder.toString();
            System.out.println("\n组合的JSON: " + json);
            
            try {
                com.fasterxml.jackson.databind.ObjectMapper mapper = new com.fasterxml.jackson.databind.ObjectMapper();
                com.fasterxml.jackson.databind.JsonNode node = mapper.readTree(json);
                System.out.println("✅ JSON 解析成功！");
                System.out.println("  name = " + node.get("name").asText());
                System.out.println("  code = " + node.get("code").asText());
                System.out.println("  discount_type = " + node.get("discount_type").asText());
                System.out.println("  start_date = " + node.get("start_date").asText());
            } catch (Exception e) {
                System.out.println("❌ JSON 解析失败: " + e.getMessage());
            }
        }
        
        System.out.println("\n" + "=".repeat(50) + "\n");
        
        // 测试2: 简单的引号和空格
        String test2 = "LOGIN \"user name\" \"pass word\"";
        System.out.println("测试2: 带空格的参数");
        System.out.println("输入: " + test2);
        String[] parts2 = parseCommand(test2);
        System.out.println("parts.length = " + parts2.length);
        for (int i = 0; i < parts2.length; i++) {
            System.out.println("parts[" + i + "] = " + parts2[i]);
        }
        
        System.out.println("\n" + "=".repeat(50) + "\n");
        
        // 测试3: 转义的引号
        String test3 = "TEST \"value with \\\"quotes\\\" inside\"";
        System.out.println("测试3: 包含转义引号的参数");
        System.out.println("输入: " + test3);
        String[] parts3 = parseCommand(test3);
        System.out.println("parts.length = " + parts3.length);
        for (int i = 0; i < parts3.length; i++) {
            System.out.println("parts[" + i + "] = " + parts3[i]);
        }
    }
}
