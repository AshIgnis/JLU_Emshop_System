package emshop;

import org.slf4j.MDC;
import java.util.UUID;

/**
 * Trace ID管理工具类
 * 用于在整个请求链路中传递唯一的追踪ID
 */
public class TraceIdUtil {
    
    private static final String TRACE_ID_KEY = "traceId";
    private static final String USER_ID_KEY = "userId";
    private static final String USERNAME_KEY = "username";
    
    /**
     * 生成新的Trace ID
     * @return 32位UUID (去掉-符号)
     */
    public static String generateTraceId() {
        return UUID.randomUUID().toString().replace("-", "");
    }
    
    /**
     * 设置Trace ID到MDC
     * @param traceId 追踪ID
     */
    public static void setTraceId(String traceId) {
        if (traceId != null && !traceId.isEmpty()) {
            MDC.put(TRACE_ID_KEY, traceId);
        }
    }
    
    /**
     * 获取当前Trace ID
     * @return Trace ID
     */
    public static String getTraceId() {
        return MDC.get(TRACE_ID_KEY);
    }
    
    /**
     * 生成并设置新的Trace ID
     * @return 新生成的Trace ID
     */
    public static String initTraceId() {
        String traceId = generateTraceId();
        setTraceId(traceId);
        return traceId;
    }
    
    /**
     * 设置用户ID到MDC
     * @param userId 用户ID
     */
    public static void setUserId(Long userId) {
        if (userId != null && userId > 0) {
            MDC.put(USER_ID_KEY, String.valueOf(userId));
        }
    }
    
    /**
     * 设置用户名到MDC
     * @param username 用户名
     */
    public static void setUsername(String username) {
        if (username != null && !username.isEmpty()) {
            MDC.put(USERNAME_KEY, username);
        }
    }
    
    /**
     * 设置用户上下文信息
     * @param userId 用户ID
     * @param username 用户名
     */
    public static void setUserContext(Long userId, String username) {
        setUserId(userId);
        setUsername(username);
    }
    
    /**
     * 清除所有MDC信息
     */
    public static void clear() {
        MDC.clear();
    }
    
    /**
     * 清除Trace ID
     */
    public static void clearTraceId() {
        MDC.remove(TRACE_ID_KEY);
    }
    
    /**
     * 清除用户上下文
     */
    public static void clearUserContext() {
        MDC.remove(USER_ID_KEY);
        MDC.remove(USERNAME_KEY);
    }
}
