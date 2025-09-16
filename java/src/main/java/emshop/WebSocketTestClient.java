package emshop;

import java.io.IOException;
import java.net.URI;
import java.util.concurrent.CountDownLatch;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.JsonNode;

/**
 * WebSocket 连接测试工具
 * 用于测试 Emshop WebSocket 服务器连接和基本功能
 */
public class WebSocketTestClient extends WebSocketClient {
    
    private static final ObjectMapper objectMapper = new ObjectMapper();
    private CountDownLatch connectionLatch = new CountDownLatch(1);
    private boolean authenticated = false;
    
    public WebSocketTestClient(URI serverURI) {
        super(serverURI);
    }
    
    @Override
    public void onOpen(ServerHandshake handshake) {
        System.out.println("✓ 连接到 WebSocket 服务器成功");
        System.out.println("  状态码: " + handshake.getHttpStatus());
        System.out.println("  状态描述: " + handshake.getHttpStatusMessage());
        connectionLatch.countDown();
    }
    
    @Override
    public void onMessage(String message) {
        System.out.println("← 收到消息: " + message);
        
        try {
            JsonNode jsonNode = objectMapper.readTree(message);
            String type = jsonNode.get("type").asText();
            boolean success = jsonNode.has("success") ? jsonNode.get("success").asBoolean() : true;
            
            if ("auth".equals(type) && success) {
                authenticated = true;
                System.out.println("✓ 认证成功");
            } else if ("welcome".equals(type)) {
                System.out.println("✓ 收到欢迎消息");
            } else if ("pong".equals(type)) {
                System.out.println("✓ 心跳响应正常");
            }
            
        } catch (Exception e) {
            System.err.println("✗ 解析消息失败: " + e.getMessage());
        }
    }
    
    @Override
    public void onClose(int code, String reason, boolean remote) {
        System.out.println("✗ WebSocket 连接关闭");
        System.out.println("  关闭码: " + code);
        System.out.println("  关闭原因: " + reason);
        System.out.println("  远程关闭: " + (remote ? "是" : "否"));
    }
    
    @Override
    public void onError(Exception ex) {
        System.err.println("✗ WebSocket 连接错误: " + ex.getMessage());
        ex.printStackTrace();
        connectionLatch.countDown();
    }
    
    public boolean waitForConnection(long timeoutMs) {
        try {
            return connectionLatch.await(timeoutMs, java.util.concurrent.TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return false;
        }
    }
    
    public void sendAuthRequest(String username, String password) {
        try {
            String authMessage = String.format(
                "{\"type\":\"auth\",\"username\":\"%s\",\"password\":\"%s\"}", 
                username, password);
            System.out.println("→ 发送认证请求: " + authMessage);
            send(authMessage);
        } catch (Exception e) {
            System.err.println("✗ 发送认证请求失败: " + e.getMessage());
        }
    }
    
    public void sendPingRequest() {
        try {
            String pingMessage = "{\"type\":\"ping\"}";
            System.out.println("→ 发送心跳: " + pingMessage);
            send(pingMessage);
        } catch (Exception e) {
            System.err.println("✗ 发送心跳失败: " + e.getMessage());
        }
    }
    
    public void sendGetProductsRequest() {
        try {
            String productsMessage = "{\"type\":\"get_products\",\"category\":\"all\",\"page\":1,\"pageSize\":10}";
            System.out.println("→ 发送获取商品请求: " + productsMessage);
            send(productsMessage);
        } catch (Exception e) {
            System.err.println("✗ 发送获取商品请求失败: " + e.getMessage());
        }
    }
    
    public boolean isAuthenticated() {
        return authenticated;
    }
    
    public static void main(String[] args) {
        String serverUrl = args.length > 0 ? args[0] : "wss://localhost:8081/ws";
        String username = args.length > 1 ? args[1] : "admin";
        String password = args.length > 2 ? args[2] : "admin123";
        
        System.out.println("=== Emshop WebSocket 测试工具 ===");
        System.out.println("服务器地址: " + serverUrl);
        System.out.println("用户名: " + username);
        System.out.println();
        
        try {
            URI serverURI = new URI(serverUrl);
            WebSocketTestClient client = new WebSocketTestClient(serverURI);
            
            // 忽略 SSL 证书错误（仅用于开发测试）
            if (serverUrl.startsWith("wss://")) {
                client.setTcpNoDelay(true);
            }
            
            System.out.println("正在连接到服务器...");
            client.connect();
            
            // 等待连接建立
            if (!client.waitForConnection(10000)) {
                System.err.println("✗ 连接超时");
                return;
            }
            
            // 等待一秒让欢迎消息到达
            Thread.sleep(1000);
            
            // 测试认证
            System.out.println("\n--- 测试用户认证 ---");
            client.sendAuthRequest(username, password);
            Thread.sleep(2000);
            
            if (!client.isAuthenticated()) {
                System.err.println("✗ 认证失败，停止测试");
                client.close();
                return;
            }
            
            // 测试心跳
            System.out.println("\n--- 测试心跳功能 ---");
            client.sendPingRequest();
            Thread.sleep(1000);
            
            // 测试业务功能
            System.out.println("\n--- 测试业务功能 ---");
            client.sendGetProductsRequest();
            Thread.sleep(2000);
            
            // 保持连接一段时间以观察推送消息
            System.out.println("\n--- 等待推送消息 (10秒) ---");
            Thread.sleep(10000);
            
            // 关闭连接
            System.out.println("\n--- 关闭连接 ---");
            client.close();
            
            System.out.println("\n✓ 测试完成");
            
        } catch (Exception e) {
            System.err.println("✗ 测试失败: " + e.getMessage());
            e.printStackTrace();
        }
    }
}