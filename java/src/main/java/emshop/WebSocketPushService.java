package emshop;

import io.netty.channel.Channel;
import io.netty.channel.group.ChannelGroup;
import io.netty.handler.codec.http.websocketx.TextWebSocketFrame;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;

/**
 * WebSocket 推送管理服务
 * 负责管理实时消息推送、用户通知等功能
 */
public class WebSocketPushService {
    
    private static final ObjectMapper objectMapper = new ObjectMapper();
    private static final ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(5);
    
    // 订阅管理：用户ID -> 订阅的话题集合
    private static final ConcurrentHashMap<Long, Set<String>> userSubscriptions = new ConcurrentHashMap<>();
    
    // 话题管理：话题 -> 订阅该话题的用户ID集合
    private static final ConcurrentHashMap<String, Set<Long>> topicSubscribers = new ConcurrentHashMap<>();
    
    /**
     * 用户订阅话题
     * @param userId 用户ID
     * @param topic 话题名称
     */
    public static void subscribe(Long userId, String topic) {
        if (userId == null || topic == null) return;
        
        // 添加到用户订阅列表
        userSubscriptions.computeIfAbsent(userId, k -> new ConcurrentSkipListSet<>()).add(topic);
        
        // 添加到话题订阅者列表
        topicSubscribers.computeIfAbsent(topic, k -> new ConcurrentSkipListSet<>()).add(userId);
        
        System.out.println("User " + userId + " subscribed to topic: " + topic);
    }
    
    /**
     * 用户取消订阅话题
     * @param userId 用户ID
     * @param topic 话题名称
     */
    public static void unsubscribe(Long userId, String topic) {
        if (userId == null || topic == null) return;
        
        // 从用户订阅列表移除
        Set<String> userTopics = userSubscriptions.get(userId);
        if (userTopics != null) {
            userTopics.remove(topic);
            if (userTopics.isEmpty()) {
                userSubscriptions.remove(userId);
            }
        }
        
        // 从话题订阅者列表移除
        Set<Long> subscribers = topicSubscribers.get(topic);
        if (subscribers != null) {
            subscribers.remove(userId);
            if (subscribers.isEmpty()) {
                topicSubscribers.remove(topic);
            }
        }
        
        System.out.println("User " + userId + " unsubscribed from topic: " + topic);
    }
    
    /**
     * 用户断线时清理订阅
     * @param userId 用户ID
     */
    public static void clearUserSubscriptions(Long userId) {
        if (userId == null) return;
        
        Set<String> userTopics = userSubscriptions.remove(userId);
        if (userTopics != null) {
            for (String topic : userTopics) {
                Set<Long> subscribers = topicSubscribers.get(topic);
                if (subscribers != null) {
                    subscribers.remove(userId);
                    if (subscribers.isEmpty()) {
                        topicSubscribers.remove(topic);
                    }
                }
            }
        }
        
        System.out.println("Cleared all subscriptions for user: " + userId);
    }
    
    /**
     * 向指定话题推送消息
     * @param topic 话题名称
     * @param message 消息内容
     * @param excludeUserId 排除的用户ID（可选，用于避免推送给发送者）
     */
    public static void pushToTopic(String topic, String message, Long excludeUserId) {
        Set<Long> subscribers = topicSubscribers.get(topic);
        if (subscribers == null || subscribers.isEmpty()) {
            return;
        }
        
        for (Long userId : subscribers) {
            if (excludeUserId != null && excludeUserId.equals(userId)) {
                continue; // 跳过排除的用户
            }
            
            ChannelGroup channels = EmshopWebSocketServer.getUserChannels(userId);
            if (channels != null && !channels.isEmpty()) {
                channels.writeAndFlush(new TextWebSocketFrame(message));
            }
        }
        
        System.out.println("Pushed message to topic '" + topic + "', reached " + subscribers.size() + " subscribers");
    }
    
    /**
     * 推送订单状态更新
     * @param userId 用户ID
     * @param orderId 订单ID
     * @param status 新状态
     * @param message 状态描述
     */
    public static void pushOrderStatusUpdate(Long userId, Long orderId, String status, String message) {
        try {
            String pushMessage = String.format(
                "{\"type\":\"order_status_update\",\"data\":{\"order_id\":%d,\"status\":\"%s\",\"message\":\"%s\",\"timestamp\":%d}}",
                orderId, status, message, System.currentTimeMillis());
                
            EmshopWebSocketServer.sendToUser(userId, pushMessage);
            
            System.out.println("Pushed order status update to user " + userId + ": Order " + orderId + " -> " + status);
        } catch (Exception e) {
            System.err.println("Failed to push order status update: " + e.getMessage());
        }
    }
    
    /**
     * 推送库存变更通知
     * @param productId 商品ID
     * @param newStock 新库存数量
     */
    public static void pushStockUpdate(Long productId, int newStock) {
        try {
            String message = String.format(
                "{\"type\":\"stock_update\",\"data\":{\"product_id\":%d,\"stock\":%d,\"timestamp\":%d}}",
                productId, newStock, System.currentTimeMillis());
                
            pushToTopic("stock_updates", message, null);
            
            System.out.println("Pushed stock update: Product " + productId + " -> " + newStock);
        } catch (Exception e) {
            System.err.println("Failed to push stock update: " + e.getMessage());
        }
    }
    
    /**
     * 推送商品价格变更通知
     * @param productId 商品ID
     * @param newPrice 新价格
     */
    public static void pushPriceUpdate(Long productId, double newPrice) {
        try {
            String message = String.format(
                "{\"type\":\"price_update\",\"data\":{\"product_id\":%d,\"price\":%.2f,\"timestamp\":%d}}",
                productId, newPrice, System.currentTimeMillis());
                
            pushToTopic("price_updates", message, null);
            
            System.out.println("Pushed price update: Product " + productId + " -> " + newPrice);
        } catch (Exception e) {
            System.err.println("Failed to push price update: " + e.getMessage());
        }
    }
    
    /**
     * 推送系统通知
     * @param title 通知标题
     * @param content 通知内容
     * @param level 通知级别 (info, warning, error)
     * @param targetUsers 目标用户列表，null 表示推送给所有用户
     */
    public static void pushSystemNotification(String title, String content, String level, Set<Long> targetUsers) {
        try {
            String message = String.format(
                "{\"type\":\"system_notification\",\"data\":{\"title\":\"%s\",\"content\":\"%s\",\"level\":\"%s\",\"timestamp\":%d}}",
                title, content, level, System.currentTimeMillis());
            
            if (targetUsers == null) {
                // 推送给所有用户
                EmshopWebSocketServer.broadcast(message);
                System.out.println("Broadcasted system notification: " + title);
            } else {
                // 推送给指定用户
                for (Long userId : targetUsers) {
                    EmshopWebSocketServer.sendToUser(userId, message);
                }
                System.out.println("Pushed system notification to " + targetUsers.size() + " users: " + title);
            }
        } catch (Exception e) {
            System.err.println("Failed to push system notification: " + e.getMessage());
        }
    }
    
    /**
     * 推送促销活动通知
     * @param promotionId 促销活动ID
     * @param title 活动标题
     * @param description 活动描述
     * @param startTime 开始时间
     * @param endTime 结束时间
     */
    public static void pushPromotionNotification(Long promotionId, String title, String description, 
                                               long startTime, long endTime) {
        try {
            String message = String.format(
                "{\"type\":\"promotion_notification\",\"data\":{\"promotion_id\":%d,\"title\":\"%s\",\"description\":\"%s\",\"start_time\":%d,\"end_time\":%d,\"timestamp\":%d}}",
                promotionId, title, description, startTime, endTime, System.currentTimeMillis());
                
            pushToTopic("promotions", message, null);
            
            System.out.println("Pushed promotion notification: " + title);
        } catch (Exception e) {
            System.err.println("Failed to push promotion notification: " + e.getMessage());
        }
    }
    
    /**
     * 推送客服消息
     * @param userId 目标用户ID
     * @param fromAdmin 是否来自管理员
     * @param message 消息内容
     */
    public static void pushCustomerServiceMessage(Long userId, boolean fromAdmin, String messageContent) {
        try {
            String message = String.format(
                "{\"type\":\"customer_service\",\"data\":{\"from_admin\":%b,\"message\":\"%s\",\"timestamp\":%d}}",
                fromAdmin, messageContent, System.currentTimeMillis());
                
            EmshopWebSocketServer.sendToUser(userId, message);
            
            System.out.println("Pushed customer service message to user " + userId);
        } catch (Exception e) {
            System.err.println("Failed to push customer service message: " + e.getMessage());
        }
    }
    
    /**
     * 定期推送在线统计信息
     */
    public static void startStatisticsPush() {
        scheduler.scheduleAtFixedRate(() -> {
            try {
                int onlineUsers = EmshopWebSocketServer.getOnlineUserCount();
                int totalConnections = EmshopWebSocketServer.getTotalConnectionCount();
                
                String message = String.format(
                    "{\"type\":\"statistics\",\"data\":{\"online_users\":%d,\"total_connections\":%d,\"timestamp\":%d}}",
                    onlineUsers, totalConnections, System.currentTimeMillis());
                    
                pushToTopic("statistics", message, null);
                
            } catch (Exception e) {
                System.err.println("Failed to push statistics: " + e.getMessage());
            }
        }, 30, 30, TimeUnit.SECONDS); // 每30秒推送一次
    }
    
    /**
     * 定期清理空的话题和订阅
     */
    public static void startCleanupTask() {
        scheduler.scheduleAtFixedRate(() -> {
            // 清理空的话题
            topicSubscribers.entrySet().removeIf(entry -> entry.getValue().isEmpty());
            
            // 清理空的用户订阅
            userSubscriptions.entrySet().removeIf(entry -> entry.getValue().isEmpty());
            
        }, 5, 5, TimeUnit.MINUTES); // 每5分钟清理一次
    }
    
    /**
     * 获取用户的订阅信息
     * @param userId 用户ID
     * @return 订阅的话题集合
     */
    public static Set<String> getUserSubscriptions(Long userId) {
        return userSubscriptions.get(userId);
    }
    
    /**
     * 获取话题的订阅者信息
     * @param topic 话题名称
     * @return 订阅该话题的用户ID集合
     */
    public static Set<Long> getTopicSubscribers(String topic) {
        return topicSubscribers.get(topic);
    }
    
    /**
     * 获取系统推送统计信息
     * @return 统计信息字符串
     */
    public static String getPushStatistics() {
        try {
            return String.format(
                "{\"total_users\":%d,\"total_topics\":%d,\"active_subscriptions\":%d}",
                userSubscriptions.size(),
                topicSubscribers.size(),
                userSubscriptions.values().stream().mapToInt(Set::size).sum()
            );
        } catch (Exception e) {
            return "{\"error\":\"Failed to get statistics\"}";
        }
    }
    
    /**
     * 关闭推送服务
     */
    public static void shutdown() {
        scheduler.shutdown();
        try {
            if (!scheduler.awaitTermination(5, TimeUnit.SECONDS)) {
                scheduler.shutdownNow();
            }
        } catch (InterruptedException e) {
            scheduler.shutdownNow();
            Thread.currentThread().interrupt();
        }
        
        userSubscriptions.clear();
        topicSubscribers.clear();
        
        System.out.println("WebSocket Push Service shutdown completed.");
    }
    
    // 初始化推送服务
    static {
        startStatisticsPush();
        startCleanupTask();
        System.out.println("WebSocket Push Service initialized.");
    }
}