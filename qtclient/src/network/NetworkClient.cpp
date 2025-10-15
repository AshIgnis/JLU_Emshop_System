#include "network/NetworkClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_connectTimer(new QTimer(this))
{
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onSocketError);
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout, this, &NetworkClient::onConnectTimeout);
}

bool NetworkClient::connectToServer(const QString &host, quint16 port, QString *errorMessage, int timeoutMs)
{
    if (isConnected() && host == m_host && port == m_port) {
        return true;
    }

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        // 移除阻塞调用,让断开异步进行
    }

    m_socket->connectToHost(host, port);
    
    // 使用QEventLoop代替阻塞等待,允许事件处理,避免UI卡顿
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    
    // 连接成功或失败时退出事件循环
    connect(m_socket, &QTcpSocket::connected, &loop, &QEventLoop::quit);
    connect(m_socket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred), 
            &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    timer.start(timeoutMs);
    loop.exec(); // 处理事件,不阻塞UI线程
    
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        if (errorMessage) {
            *errorMessage = timer.isActive() ? m_socket->errorString() : tr("连接超时");
        }
        return false;
    }

    m_host = host;
    m_port = port;
    emit connectionStateChanged(true);
    return true;
}

void NetworkClient::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        // 移除阻塞调用,让断开连接异步进行,避免UI卡顿
    }
}

// 异步连接方法(完全非阻塞)
void NetworkClient::connectToServerAsync(const QString &host, quint16 port, 
                                         ConnectHandler onComplete,
                                         int timeoutMs)
{
    // 如果已经连接到相同的服务器,直接成功
    if (isConnected() && host == m_host && port == m_port) {
        if (onComplete) {
            onComplete(true, QString());
        }
        return;
    }

    // 保存回调
    m_connectHandler = std::move(onComplete);

    // 如果socket正在使用,先断开
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }

    // 启动超时定时器
    m_connectTimer->start(timeoutMs);

    // 发起异步连接
    m_socket->connectToHost(host, port);
    
    // 连接成功/失败会触发onConnected/onSocketError槽函数
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::sendCommand(const QString &command,
                                ResponseHandler onSuccess,
                                ResponseHandler onError)
{
    if (!isConnected()) {
        if (onError) {
            onError(tr("未连接到服务器"));
        }
        return;
    }

    QString payload = command;
    if (!payload.endsWith('\n')) {
        payload.append('\n');
    }

    PendingRequest request;
    request.command = command;
    request.success = std::move(onSuccess);
    request.error = std::move(onError);
    m_pendingRequests.enqueue(std::move(request));

    QByteArray utf8 = payload.toUtf8();
    qint64 bytesWritten = m_socket->write(utf8);
    if (bytesWritten == -1) {
        PendingRequest failedRequest = m_pendingRequests.dequeue();
        if (failedRequest.error) {
            failedRequest.error(m_socket->errorString());
        }
    } else {
        m_socket->flush();
    }
}

void NetworkClient::sendJson(const QJsonObject &object,
                             ResponseHandler onSuccess,
                             ResponseHandler onError)
{
    QJsonDocument doc(object);
    QString jsonString = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    sendCommand(jsonString, std::move(onSuccess), std::move(onError));
}

void NetworkClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    int newlineIndex = -1;
    while ((newlineIndex = m_buffer.indexOf('\n')) != -1) {
        QByteArray rawLine = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);

        QString response = QString::fromUtf8(rawLine).trimmed();
        if (response.isEmpty()) {
            continue;
        }
        handleResponse(response);
    }
}

void NetworkClient::handleResponse(const QString &response)
{
    if (!m_pendingRequests.isEmpty()) {
        PendingRequest request = m_pendingRequests.dequeue();
        if (request.success) {
            request.success(response);
        }
    } else {
        emit serverMessageReceived(response);
    }
}

void NetworkClient::onSocketError(QAbstractSocket::SocketError)
{
    QString errorMsg = m_socket->errorString();
    
    // 如果是连接过程中的错误,调用连接回调
    if (m_connectHandler) {
        m_connectTimer->stop();
        auto handler = std::move(m_connectHandler);
        m_connectHandler = nullptr;
        handler(false, errorMsg);
        emit connectCompleted(false, errorMsg);
    }
    
    flushPendingErrors(errorMsg);
    emit connectionStateChanged(false);
}

void NetworkClient::onDisconnected()
{
    // 如果是连接过程中断开,调用连接回调
    if (m_connectHandler) {
        m_connectTimer->stop();
        auto handler = std::move(m_connectHandler);
        m_connectHandler = nullptr;
        handler(false, tr("连接已断开"));
        emit connectCompleted(false, tr("连接已断开"));
    }
    
    flushPendingErrors(tr("连接已断开"));
    emit connectionStateChanged(false);
}

void NetworkClient::onConnected()
{
    m_connectTimer->stop();
    
    m_host = m_socket->peerAddress().toString();
    m_port = m_socket->peerPort();
    
    emit connectionStateChanged(true);
    
    // 调用连接回调
    if (m_connectHandler) {
        auto handler = std::move(m_connectHandler);
        m_connectHandler = nullptr;
        handler(true, QString());
        emit connectCompleted(true, QString());
    }
}

void NetworkClient::onConnectTimeout()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->abort(); // 强制关闭连接
        
        if (m_connectHandler) {
            auto handler = std::move(m_connectHandler);
            m_connectHandler = nullptr;
            handler(false, tr("连接超时"));
            emit connectCompleted(false, tr("连接超时"));
        }
    }
}

void NetworkClient::flushPendingErrors(const QString &reason)
{
    while (!m_pendingRequests.isEmpty()) {
        PendingRequest request = m_pendingRequests.dequeue();
        if (request.error) {
            request.error(reason);
        }
    }
    m_buffer.clear();
}
