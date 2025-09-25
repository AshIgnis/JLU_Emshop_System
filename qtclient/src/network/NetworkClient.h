#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QByteArray>
#include <functional>

class QJsonObject;

class NetworkClient : public QObject {
    Q_OBJECT
public:
    using ResponseHandler = std::function<void(const QString &)>;

    explicit NetworkClient(QObject *parent = nullptr);

    bool connectToServer(const QString &host, quint16 port, QString *errorMessage = nullptr, int timeoutMs = 5000);
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

private slots:
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onDisconnected();

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
};
