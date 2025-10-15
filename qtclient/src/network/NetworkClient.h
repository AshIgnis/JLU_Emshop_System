#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QByteArray>
#include <functional>

class QJsonObject;
class QTimer;

class NetworkClient : public QObject {
    Q_OBJECT
public:
    using ResponseHandler = std::function<void(const QString &)>;
    using ConnectHandler = std::function<void(bool success, const QString &errorMessage)>;

    explicit NetworkClient(QObject *parent = nullptr);

    // 同步连接(向后兼容)
    bool connectToServer(const QString &host, quint16 port, QString *errorMessage = nullptr, int timeoutMs = 5000);
    
    // 异步连接(推荐使用)
    void connectToServerAsync(const QString &host, quint16 port, 
                             ConnectHandler onComplete,
                             int timeoutMs = 5000);
    
    void disconnectFromServer();

    bool isConnected() const;
    QString host() const { return m_host; }
    quint16 port() const { return m_port; }

    void sendCommand(const QString &command,
                     ResponseHandler onSuccess,
                     ResponseHandler onError = {});

    void sendJson(const QJsonObject &object,
                  ResponseHandler onSuccess,
                  ResponseHandler onError = {});

    int pendingRequestCount() const { return m_pendingRequests.size(); }

signals:
    void serverMessageReceived(const QString &message);
    void connectionStateChanged(bool connected);
    void connectCompleted(bool success, const QString &errorMessage);

private slots:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onDisconnected();
    void onConnected();
    void onConnectTimeout();

private:
    struct PendingRequest {
        QString command;
        ResponseHandler success;
        ResponseHandler error;
    };

    void handleResponse(const QString &response);
    void flushPendingErrors(const QString &reason);

    QTcpSocket *m_socket;
    QString m_host;
    quint16 m_port = 0;
    QQueue<PendingRequest> m_pendingRequests;
    QByteArray m_buffer;
    
    // 异步连接相关
    QTimer *m_connectTimer = nullptr;
    ConnectHandler m_connectHandler;
};
