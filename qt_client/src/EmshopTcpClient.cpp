#include "EmshopTcpClient.h"
#include <QDebug>
#include <QJsonParseError>
#include <QHostAddress>
#include <QRegularExpression>

EmshopTcpClient::EmshopTcpClient(QObject *parent)
    : QObject(parent)
    , m_tcpSocket(new QTcpSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_connectionState(Disconnected)
    , m_authenticated(false)
    , m_serverPort(8080)
{
    // 设置TCP socket连接
    connect(m_tcpSocket, &QTcpSocket::connected, this, &EmshopTcpClient::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &EmshopTcpClient::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &EmshopTcpClient::onReadyRead);
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &EmshopTcpClient::onError);
    
    // 设置心跳定时器
    m_heartbeatTimer->setSingleShot(false);
    m_heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &EmshopTcpClient::sendHeartbeat);
    
    qDebug() << "EmshopTcpClient initialized";
}

EmshopTcpClient::~EmshopTcpClient()
{
    disconnectFromServer();
}

void EmshopTcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_connectionState != Disconnected) {
        qDebug() << "Already connected or connecting";
        return;
    }
    
    m_serverHost = host;
    m_serverPort = port;
    
    setState(Connecting);
    qDebug() << "Connecting to" << host << ":" << port;
    
    m_tcpSocket->connectToHost(host, port);
}

void EmshopTcpClient::disconnectFromServer()
{
    if (m_connectionState == Disconnected) {
        return;
    }
    
    m_heartbeatTimer->stop();
    m_authenticated = false;
    m_userInfo = QJsonObject();
    
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
        if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            m_tcpSocket->waitForDisconnected(3000);
        }
    }
    
    setState(Disconnected);
    qDebug() << "Disconnected from server";
}

EmshopTcpClient::ConnectionState EmshopTcpClient::connectionState() const
{
    return m_connectionState;
}

void EmshopTcpClient::authenticate(const QString &username, const QString &password)
{
    if (m_connectionState != Connected) {
        emit authenticationFailed("Not connected to server");
        return;
    }
    
    // 发送简单的LOGIN命令格式，与Netty服务器兼容
    QString command = QString("LOGIN %1 %2").arg(username, password);
    sendTextCommand(command);
    qDebug() << "Authentication request sent for user:" << username;
}

bool EmshopTcpClient::isAuthenticated() const
{
    return m_authenticated && m_connectionState == Authenticated;
}

void EmshopTcpClient::getProducts(const QString &category, int page, int pageSize)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    // 发送GET_PRODUCTS命令，与Netty服务器兼容
    QString command = QString("GET_PRODUCTS %1 %2 %3").arg(category).arg(page).arg(pageSize);
    sendTextCommand(command);
}

void EmshopTcpClient::searchProducts(const QString &keyword, int page, int pageSize, 
                                    const QString &sortBy, double minPrice, double maxPrice)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "searchProducts";
    request["keyword"] = keyword;
    request["page"] = page;
    request["pageSize"] = pageSize;
    request["sortBy"] = sortBy;
    request["minPrice"] = minPrice;
    request["maxPrice"] = maxPrice;
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::addToCart(qint64 productId, int quantity)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    // 发送ADD_TO_CART命令，与Netty服务器兼容
    QString command = QString("ADD_TO_CART %1 %2").arg(productId).arg(quantity);
    sendTextCommand(command);
}

void EmshopTcpClient::getCart()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    // 发送GET_CART命令，与Netty服务器兼容
    QString command = "GET_CART";
    sendTextCommand(command);
}

void EmshopTcpClient::removeFromCart(qint64 productId)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "removeFromCart";
    request["productId"] = productId;
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::updateCartQuantity(qint64 productId, int quantity)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "updateCartQuantity";
    request["productId"] = productId;
    request["quantity"] = quantity;
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::clearCart()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "clearCart";
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::getUserOrders()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "getUserOrders";
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::getOrderDetail(qint64 orderId)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "getOrderDetail";
    request["orderId"] = orderId;
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::getUserAddresses()
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "getUserAddresses";
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::addAddress(const QString &receiverName, const QString &phone, 
                                const QString &province, const QString &city, 
                                const QString &district, const QString &detailAddress,
                                const QString &postalCode, bool isDefault)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "addAddress";
    request["receiverName"] = receiverName;
    request["phone"] = phone;
    request["province"] = province;
    request["city"] = city;
    request["district"] = district;
    request["detailAddress"] = detailAddress;
    request["postalCode"] = postalCode;
    request["isDefault"] = isDefault;
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::createOrder(qint64 addressId, const QString &couponCode, const QString &remark)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "createOrder";
    request["addressId"] = addressId;
    if (!couponCode.isEmpty()) {
        request["couponCode"] = couponCode;
    }
    if (!remark.isEmpty()) {
        request["remark"] = remark;
    }
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::processPayment(qint64 orderId, const QString &paymentMethod, double amount,
                                    const QJsonObject &paymentDetails)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "processPayment";
    request["orderId"] = orderId;
    request["paymentMethod"] = paymentMethod;
    request["amount"] = amount;
    if (!paymentDetails.isEmpty()) {
        request["paymentDetails"] = paymentDetails;
    }
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::cancelOrder(qint64 orderId, const QString &reason)
{
    if (!isAuthenticated()) {
        emit error("Not authenticated");
        return;
    }
    
    QJsonObject request;
    request["action"] = "cancelOrder";
    request["orderId"] = orderId;
    if (!reason.isEmpty()) {
        request["reason"] = reason;
    }
    request["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    
    sendMessage(request);
}

void EmshopTcpClient::onConnected()
{
    setState(Connected);
    setupHeartbeat();
    emit connected();
    qDebug() << "Connected to Netty server";
}

void EmshopTcpClient::onDisconnected()
{
    m_heartbeatTimer->stop();
    m_authenticated = false;
    setState(Disconnected);
    emit disconnected();
    qDebug() << "Disconnected from Netty server";
}

void EmshopTcpClient::onReadyRead()
{
    // 读取所有可用数据到缓冲区
    m_receiveBuffer.append(m_tcpSocket->readAll());
    processReceivedData();
}

void EmshopTcpClient::onError(QAbstractSocket::SocketError socketError)
{
    QString errorString;
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        errorString = "Connection refused by server";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorString = "Host not found";
        break;
    case QAbstractSocket::NetworkError:
        errorString = "Network error";
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorString = "Connection timeout";
        break;
    default:
        errorString = m_tcpSocket->errorString();
    }
    
    setState(Disconnected);
    emit error(errorString);
    qDebug() << "Socket error:" << errorString;
}

void EmshopTcpClient::sendHeartbeat()
{
    if (m_connectionState == Connected || m_connectionState == Authenticated) {
        QJsonObject heartbeat;
        heartbeat["action"] = "heartbeat";
        heartbeat["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        sendMessage(heartbeat);
    }
}

void EmshopTcpClient::sendMessage(const QJsonObject &message)
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append(MESSAGE_DELIMITER); // 添加消息分隔符
    
    qint64 bytesWritten = m_tcpSocket->write(data);
    if (bytesWritten == -1) {
        emit error("Failed to send message: " + m_tcpSocket->errorString());
        return;
    }
    
    m_tcpSocket->flush();
    qDebug() << "Message sent:" << data.left(200) << "..."; // 只显示前200字符
}

void EmshopTcpClient::sendTextCommand(const QString &command)
{
    if (m_tcpSocket->state() != QAbstractSocket::ConnectedState) {
        emit error("Not connected to server");
        return;
    }
    
    QByteArray data = command.toUtf8();
    data.append(MESSAGE_DELIMITER); // 添加消息分隔符
    
    qint64 bytesWritten = m_tcpSocket->write(data);
    if (bytesWritten == -1) {
        emit error("Failed to send message: " + m_tcpSocket->errorString());
        return;
    }
    
    m_tcpSocket->flush();
    qDebug() << "Command sent:" << command;
}

void EmshopTcpClient::processReceivedData()
{
    // 处理缓冲区中的完整消息
    while (true) {
        int delimiterIndex = m_receiveBuffer.indexOf(MESSAGE_DELIMITER);
        if (delimiterIndex == -1) {
            // 没有完整的消息，等待更多数据
            break;
        }
        
        // 提取一个完整的消息
        QByteArray messageData = m_receiveBuffer.left(delimiterIndex);
        m_receiveBuffer.remove(0, delimiterIndex + 1);
        
        if (messageData.isEmpty()) {
            continue;
        }
        
        QString message = QString::fromUtf8(messageData);
        qDebug() << "Received message:" << message;
        
        // 尝试解析为JSON，如果失败则当作纯文本处理
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(messageData, &parseError);
        
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            // JSON响应
            QJsonObject response = doc.object();
            handleResponse(response);
        } else {
            // 纯文本响应（来自Netty服务器）
            handleTextResponse(message);
        }
    }
}

void EmshopTcpClient::handleResponse(const QJsonObject &response)
{
    QString action = response["action"].toString();
    QString status = response["status"].toString();
    bool success = (status == "success");
    
    qDebug() << "Received response - Action:" << action << "Status:" << status;
    
    if (action == "login") {
        if (success) {
            m_authenticated = true;
            m_userInfo = response["data"].toObject();
            setState(Authenticated);
            emit authenticated(m_userInfo);
            qDebug() << "Authentication successful for user:" << m_userInfo["username"].toString();
        } else {
            QString errorMsg = response["message"].toString();
            emit authenticationFailed(errorMsg);
            qDebug() << "Authentication failed:" << errorMsg;
        }
    }
    else if (action == "getProducts") {
        if (success) {
            emit productsReceived(response["data"].toObject());
        } else {
            emit error("Failed to get products: " + response["message"].toString());
        }
    }
    else if (action == "searchProducts") {
        if (success) {
            emit searchResultsReceived(response["data"].toObject());
        } else {
            emit error("Search failed: " + response["message"].toString());
        }
    }
    else if (action == "addToCart" || action == "updateCartQuantity" || action == "removeFromCart" || action == "clearCart") {
        if (success) {
            emit cartUpdated(response["data"].toObject());
        } else {
            emit error("Cart operation failed: " + response["message"].toString());
        }
    }
    else if (action == "getCart") {
        if (success) {
            emit cartReceived(response["data"].toObject());
        } else {
            emit error("Failed to get cart: " + response["message"].toString());
        }
    }
    else if (action == "getUserOrders") {
        if (success) {
            emit ordersReceived(response["data"].toObject());
        } else {
            emit error("Failed to get orders: " + response["message"].toString());
        }
    }
    else if (action == "getOrderDetail") {
        if (success) {
            emit orderDetailReceived(response["data"].toObject());
        } else {
            emit error("Failed to get order detail: " + response["message"].toString());
        }
    }
    else if (action == "getUserAddresses") {
        if (success) {
            emit addressesReceived(response["data"].toObject());
        } else {
            emit error("Failed to get addresses: " + response["message"].toString());
        }
    }
    else if (action == "addAddress") {
        if (success) {
            emit addressAdded(response["data"].toObject());
        } else {
            emit error("Failed to add address: " + response["message"].toString());
        }
    }
    else if (action == "createOrder") {
        if (success) {
            emit orderCreated(response["data"].toObject());
        } else {
            emit error("Failed to create order: " + response["message"].toString());
        }
    }
    else if (action == "processPayment") {
        if (success) {
            emit paymentProcessed(response["data"].toObject());
        } else {
            emit error("Payment failed: " + response["message"].toString());
        }
    }
    else if (action == "cancelOrder") {
        if (success) {
            emit orderCancelled(response["data"].toObject());
        } else {
            emit error("Failed to cancel order: " + response["message"].toString());
        }
    }
    // 处理推送消息
    else if (action == "orderStatusUpdate") {
        qint64 orderId = response["orderId"].toVariant().toLongLong();
        QString status = response["status"].toString();
        QString message = response["message"].toString();
        emit orderStatusUpdated(orderId, status, message);
    }
    else if (action == "stockUpdate") {
        qint64 productId = response["productId"].toVariant().toLongLong();
        int newStock = response["stock"].toInt();
        emit stockUpdated(productId, newStock);
    }
    else if (action == "priceUpdate") {
        qint64 productId = response["productId"].toVariant().toLongLong();
        double newPrice = response["price"].toDouble();
        emit priceUpdated(productId, newPrice);
    }
    else if (action == "systemNotification") {
        QString title = response["title"].toString();
        QString content = response["content"].toString();
        QString level = response["level"].toString();
        emit systemNotificationReceived(title, content, level);
    }
    else if (action == "heartbeat") {
        // 心跳响应，无需处理
        qDebug() << "Heartbeat acknowledged";
    }
    else {
        qDebug() << "Unknown action received:" << action;
    }
}

void EmshopTcpClient::handleTextResponse(const QString &response)
{
    qDebug() << "Handling text response:" << response;
    
    // 检查是否是欢迎消息
    if (response.contains("Welcome to Emshop Server")) {
        qDebug() << "Received welcome message from server";
        return;
    }
    
    // 尝试解析为JSON（Netty服务器返回JSON格式的响应）
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject jsonResponse = doc.object();
        bool success = jsonResponse["success"].toBool();
        QString message = jsonResponse["message"].toString();
        
        // 根据响应内容判断操作类型
        if (jsonResponse.contains("user_info") || jsonResponse.contains("role")) {
            // 登录响应
            if (success) {
                m_authenticated = true;
                QJsonObject userInfo;
                if (jsonResponse.contains("user_info")) {
                    userInfo = jsonResponse["user_info"].toObject();
                } else {
                    userInfo["username"] = "user";
                    userInfo["role"] = jsonResponse["role"].toString();
                    userInfo["user_id"] = jsonResponse["user_id"].toInt();
                }
                m_userInfo = userInfo;
                setState(Authenticated);
                emit authenticated(m_userInfo);
                qDebug() << "Authentication successful";
            } else {
                emit authenticationFailed(message);
                qDebug() << "Authentication failed:" << message;
            }
        }
        else if (jsonResponse.contains("products") || jsonResponse.contains("data")) {
            // 产品列表响应
            if (success) {
                emit productsReceived(jsonResponse);
            } else {
                emit error("Failed to get products: " + message);
            }
        }
        else if (jsonResponse.contains("cart")) {
            // 购物车响应
            if (success) {
                emit cartReceived(jsonResponse);
            } else {
                emit error("Cart operation failed: " + message);
            }
        }
        else {
            // 其他通用响应
            if (!success) {
                emit error(message);
            }
        }
    } else {
        // 纯文本响应，可能是错误消息
        if (response.startsWith("ERROR:")) {
            emit error(response.mid(6).trimmed());
        } else {
            qDebug() << "Unknown text response:" << response;
        }
    }
}

void EmshopTcpClient::setupHeartbeat()
{
    m_heartbeatTimer->start();
}

void EmshopTcpClient::setState(ConnectionState newState)
{
    if (m_connectionState != newState) {
        m_connectionState = newState;
        emit connectionStateChanged(newState);
    }
}