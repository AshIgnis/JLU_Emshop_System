#include "network/NetworkClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, qOverload<QAbstractSocket::SocketError>(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onSocketError);
}

bool NetworkClient::connectToServer(const QString &host, quint16 port, QString *errorMessage, int timeoutMs)
{
    if (isConnected() && host == m_host && port == m_port) {
        return true;
    }

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(1000);
    }

    m_socket->connectToHost(host, port);
    if (!m_socket->waitForConnected(timeoutMs)) {
        if (errorMessage) {
            *errorMessage = m_socket->errorString();
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
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(500);
        }
    }
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
    flushPendingErrors(m_socket->errorString());
    emit connectionStateChanged(false);
}

void NetworkClient::onDisconnected()
{
    flushPendingErrors(tr("连接已断开"));
    emit connectionStateChanged(false);
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
