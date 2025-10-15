#include "network/AsyncRequestManager.h"
#include "network/NetworkClient.h"
#include <QDebug>
#include <algorithm>

AsyncRequestManager::AsyncRequestManager(NetworkClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    Q_ASSERT(client);
}

quint64 AsyncRequestManager::sendRequest(const QString &command,
                                         ResponseCallback callback,
                                         Priority priority,
                                         int timeoutMs,
                                         bool allowDuplicate)
{
    // 请求去重
    if (!allowDuplicate && m_duplicateCallbacks.contains(command)) {
        // 该请求已在队列中,添加回调到列表
        m_duplicateCallbacks[command].append(callback);
        qDebug() << "[AsyncRequestManager] 请求去重,合并回调:" << command;
        return 0; // 返回0表示请求被合并
    }
    
    // 创建请求
    Request request;
    request.id = m_nextRequestId++;
    request.command = command;
    request.callback = callback;
    request.priority = priority;
    request.timeoutMs = timeoutMs;
    
    // 如果允许去重,记录
    if (!allowDuplicate) {
        m_duplicateCallbacks[command].append(callback);
    }
    
    // 根据优先级插入队列
    auto it = std::lower_bound(m_pendingRequests.begin(), m_pendingRequests.end(), 
                               request, [](const Request &a, const Request &b) {
                                   return a.priority > b.priority;
                               });
    m_pendingRequests.insert(it, request);
    
    emit queueSizeChanged(pendingRequestCount());
    
    // 尝试立即处理
    QTimer::singleShot(0, this, &AsyncRequestManager::processNextRequest);
    
    return request.id;
}

void AsyncRequestManager::cancelRequest(quint64 requestId)
{
    // 从待处理队列中移除
    for (int i = 0; i < m_pendingRequests.size(); ++i) {
        if (m_pendingRequests[i].id == requestId) {
            m_pendingRequests.removeAt(i);
            emit queueSizeChanged(pendingRequestCount());
            return;
        }
    }
    
    // 从活动请求中移除
    auto it = m_activeRequests.find(requestId);
    if (it != m_activeRequests.end()) {
        if (it->timer) {
            it->timer->stop();
            it->timer->deleteLater();
        }
        m_activeRequests.erase(it);
        emit queueSizeChanged(pendingRequestCount());
    }
}

void AsyncRequestManager::sendBatchRequests(const QStringList &commands,
                                           std::function<void(QMap<QString, QString>)> callback)
{
    if (commands.isEmpty()) {
        callback({});
        return;
    }
    
    // 使用shared_ptr来管理批量请求的状态
    struct BatchState {
        int totalCount;
        int completedCount = 0;
        QMap<QString, QString> results;
        std::function<void(QMap<QString, QString>)> callback;
    };
    
    auto state = std::make_shared<BatchState>();
    state->totalCount = commands.size();
    state->callback = callback;
    
    // 为每个命令发送请求
    for (const QString &command : commands) {
        sendRequest(command, [state, command](bool success, const QString &response) {
            state->results[command] = success ? response : QString("ERROR: %1").arg(response);
            state->completedCount++;
            
            // 所有请求完成后调用回调
            if (state->completedCount == state->totalCount) {
                state->callback(state->results);
            }
        }, Normal, 10000, false); // 批量请求使用去重
    }
}

void AsyncRequestManager::clearPendingRequests()
{
    // 调用所有待处理请求的回调,通知取消
    for (const auto &request : m_pendingRequests) {
        if (request.callback) {
            request.callback(false, tr("请求已取消"));
        }
    }
    
    m_pendingRequests.clear();
    m_duplicateCallbacks.clear();
    emit queueSizeChanged(0);
}

void AsyncRequestManager::processNextRequest()
{
    // 检查是否达到并发上限
    if (m_activeRequests.size() >= m_maxConcurrent) {
        return;
    }
    
    // 检查是否有待处理请求
    if (m_pendingRequests.isEmpty()) {
        return;
    }
    
    // 取出优先级最高的请求
    Request request = m_pendingRequests.takeFirst();
    
    // 创建超时定时器
    request.timer = new QTimer(this);
    request.timer->setSingleShot(true);
    request.timer->setInterval(request.timeoutMs);
    
    connect(request.timer, &QTimer::timeout, this, [this, id = request.id]() {
        onRequestTimeout();
        
        auto it = m_activeRequests.find(id);
        if (it != m_activeRequests.end()) {
            qDebug() << "[AsyncRequestManager] 请求超时:" << it->command;
            
            // 调用回调
            if (it->callback) {
                it->callback(false, tr("请求超时"));
            }
            
            // 处理去重的回调
            auto dupIt = m_duplicateCallbacks.find(it->command);
            if (dupIt != m_duplicateCallbacks.end()) {
                for (const auto &cb : dupIt.value()) {
                    if (cb) cb(false, tr("请求超时"));
                }
                m_duplicateCallbacks.erase(dupIt);
            }
            
            if (it->timer) {
                it->timer->deleteLater();
            }
            m_activeRequests.erase(it);
            
            emit requestCompleted(id, false);
            emit queueSizeChanged(pendingRequestCount());
            
            // 处理下一个请求
            QTimer::singleShot(0, this, &AsyncRequestManager::processNextRequest);
        }
    });
    
    // 添加到活动请求列表
    quint64 requestId = request.id;
    m_activeRequests.insert(requestId, request);
    
    // 发送网络请求
    m_client->sendCommand(
        request.command,
        // 成功回调
        [this, requestId](const QString &response) {
            auto it = m_activeRequests.find(requestId);
            if (it != m_activeRequests.end()) {
                // 停止超时定时器
                if (it->timer) {
                    it->timer->stop();
                    it->timer->deleteLater();
                }
                
                // 调用主回调
                if (it->callback) {
                    it->callback(true, response);
                }
                
                // 处理去重的回调
                auto dupIt = m_duplicateCallbacks.find(it->command);
                if (dupIt != m_duplicateCallbacks.end()) {
                    for (const auto &cb : dupIt.value()) {
                        if (cb) cb(true, response);
                    }
                    m_duplicateCallbacks.erase(dupIt);
                }
                
                m_activeRequests.erase(it);
                
                emit requestCompleted(requestId, true);
                emit queueSizeChanged(pendingRequestCount());
                
                // 处理下一个请求
                QTimer::singleShot(0, this, &AsyncRequestManager::processNextRequest);
            }
        },
        // 错误回调
        [this, requestId](const QString &error) {
            auto it = m_activeRequests.find(requestId);
            if (it != m_activeRequests.end()) {
                // 停止超时定时器
                if (it->timer) {
                    it->timer->stop();
                    it->timer->deleteLater();
                }
                
                // 调用主回调
                if (it->callback) {
                    it->callback(false, error);
                }
                
                // 处理去重的回调
                auto dupIt = m_duplicateCallbacks.find(it->command);
                if (dupIt != m_duplicateCallbacks.end()) {
                    for (const auto &cb : dupIt.value()) {
                        if (cb) cb(false, error);
                    }
                    m_duplicateCallbacks.erase(dupIt);
                }
                
                m_activeRequests.erase(it);
                
                emit requestCompleted(requestId, false);
                emit queueSizeChanged(pendingRequestCount());
                
                // 处理下一个请求
                QTimer::singleShot(0, this, &AsyncRequestManager::processNextRequest);
            }
        }
    );
    
    // 启动超时定时器
    request.timer->start();
    
    emit queueSizeChanged(pendingRequestCount());
    
    // 如果还有空闲槽位,继续处理下一个
    if (m_activeRequests.size() < m_maxConcurrent && !m_pendingRequests.isEmpty()) {
        QTimer::singleShot(0, this, &AsyncRequestManager::processNextRequest);
    }
}

void AsyncRequestManager::onRequestTimeout()
{
    // 超时处理在定时器回调中完成
}
