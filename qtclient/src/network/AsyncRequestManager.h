#pragma once

#include <QObject>
#include <QString>
#include <QQueue>
#include <QMap>
#include <QTimer>
#include <functional>
#include <memory>

class NetworkClient;

/**
 * @brief 异步请求管理器
 * 
 * 功能:
 * 1. 请求队列管理 - 控制并发请求数量
 * 2. 请求优先级 - 支持高优先级请求插队
 * 3. 请求超时 - 自动超时重试
 * 4. 请求去重 - 相同请求合并
 * 5. 批量请求 - 合并多个请求减少网络开销
 */
class AsyncRequestManager : public QObject {
    Q_OBJECT
    
public:
    enum Priority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };
    
    using ResponseCallback = std::function<void(bool success, const QString &response)>;
    
    explicit AsyncRequestManager(NetworkClient *client, QObject *parent = nullptr);
    
    /**
     * @brief 发送异步请求
     * @param command 命令字符串
     * @param callback 响应回调
     * @param priority 优先级
     * @param timeoutMs 超时时间(毫秒)
     * @param allowDuplicate 是否允许重复请求
     * @return 请求ID
     */
    quint64 sendRequest(const QString &command,
                       ResponseCallback callback,
                       Priority priority = Normal,
                       int timeoutMs = 10000,
                       bool allowDuplicate = true);
    
    /**
     * @brief 取消请求
     */
    void cancelRequest(quint64 requestId);
    
    /**
     * @brief 批量发送请求
     * @param commands 命令列表
     * @param callback 所有请求完成后的回调
     */
    void sendBatchRequests(const QStringList &commands,
                          std::function<void(QMap<QString, QString>)> callback);
    
    /**
     * @brief 设置最大并发请求数
     */
    void setMaxConcurrentRequests(int max) { m_maxConcurrent = max; }
    
    /**
     * @brief 获取待处理请求数量
     */
    int pendingRequestCount() const { return m_pendingRequests.size() + m_activeRequests.size(); }
    
    /**
     * @brief 清空所有待处理请求
     */
    void clearPendingRequests();

signals:
    void requestCompleted(quint64 requestId, bool success);
    void queueSizeChanged(int size);
    
private slots:
    void processNextRequest();
    void onRequestTimeout();
    
private:
    struct Request {
        quint64 id;
        QString command;
        ResponseCallback callback;
        Priority priority;
        int timeoutMs;
        QTimer *timer = nullptr;
        
        bool operator<(const Request &other) const {
            return priority > other.priority; // 高优先级排前面
        }
    };
    
    NetworkClient *m_client;
    quint64 m_nextRequestId = 1;
    int m_maxConcurrent = 5; // 默认最多5个并发请求
    
    QList<Request> m_pendingRequests; // 待处理队列(按优先级排序)
    QMap<quint64, Request> m_activeRequests; // 活动请求
    QMap<QString, QList<ResponseCallback>> m_duplicateCallbacks; // 去重合并回调
};
