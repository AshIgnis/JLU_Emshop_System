#include "EmshopClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QSslError>
#include <QNetworkProxy>

EmshopClient::EmshopClient(QObject *parent)
    : QObject(parent)
    , m_webSocket(nullptr)
    , m_heartbeatTimer(new QTimer(this))
    , m_connectionState(Disconnected)
    , m_authenticated(false)
{
    setupHeartbeat();
}

EmshopClient::~EmshopClient()
{
    if (m_webSocket && m_webSocket->isValid()) {
        m_webSocket->close();
    }
}

void EmshopClient::connectToServer(const QString &url)
{
    if (m_webSocket && m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "Already connected or connecting";
        return;
    }

    m_serverUrl = url;
    m_connectionState = Connecting;
    emit connectionStateChanged(m_connectionState);

    // 创建新的 WebSocket 连接
    if (m_webSocket) {
        m_webSocket->deleteLater();
    }
    
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    
    // 设置 SSL 配置
    setupSslConfiguration();
    
    // 连接信号
    connect(m_webSocket, &QWebSocket::connected, this, &EmshopClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &EmshopClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &EmshopClient::onTextMessageReceived);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &EmshopClient::onError);
    connect(m_webSocket, &QWebSocket::sslErrors, this, &EmshopClient::onSslErrors);
    
    qDebug() << "Connecting to:" << url;
    m_webSocket->open(QUrl(url));
}

void EmshopClient::disconnectFromServer()
{
    if (m_webSocket && m_webSocket->isValid()) {
        m_webSocket->close();
    }
    
    m_heartbeatTimer->stop();
    m_authenticated = false;
    m_connectionState = Disconnected;
    emit connectionStateChanged(m_connectionState);
}

EmshopClient::ConnectionState EmshopClient::connectionState() const
{
    return m_connectionState;
}

void EmshopClient::authenticate(const QString &username, const QString &password)
{
    if (m_connectionState != Connected) {
        qWarning() << "Not connected to server";
        return;
    }

    QJsonObject authMessage;
    authMessage["type"] = "auth";
    authMessage["username"] = username;
    authMessage["password"] = password;

    sendMessage(authMessage);
}

bool EmshopClient::isAuthenticated() const
{
    return m_authenticated;
}

void EmshopClient::subscribeTopic(const QString &topic)
{
    if (!m_subscribedTopics.contains(topic)) {
        m_subscribedTopics.append(topic);
        
        QJsonObject subscribeMessage;
        subscribeMessage["type"] = "subscribe";
        subscribeMessage["topic"] = topic;
        
        sendMessage(subscribeMessage);
    }
}

void EmshopClient::unsubscribeTopic(const QString &topic)
{
    if (m_subscribedTopics.removeOne(topic)) {
        QJsonObject unsubscribeMessage;
        unsubscribeMessage["type"] = "unsubscribe";
        unsubscribeMessage["topic"] = topic;
        
        sendMessage(unsubscribeMessage);
    }
}

void EmshopClient::getProducts(const QString &category, int page, int pageSize)
{
    QJsonObject message;
    message["type"] = "get_products";
    message["category"] = category;
    message["page"] = page;
    message["pageSize"] = pageSize;
    
    sendMessage(message);
}

void EmshopClient::searchProducts(const QString &keyword, int page, int pageSize, 
                                 const QString &sortBy, double minPrice, double maxPrice)
{
    QJsonObject message;
    message["type"] = "search_products";
    message["keyword"] = keyword;
    message["page"] = page;
    message["pageSize"] = pageSize;
    message["sortBy"] = sortBy;
    message["minPrice"] = minPrice;
    message["maxPrice"] = maxPrice;
    
    sendMessage(message);
}

void EmshopClient::addToCart(qint64 productId, int quantity)
{
    QJsonObject message;
    message["type"] = "add_to_cart";
    message["productId"] = productId;
    message["quantity"] = quantity;
    
    sendMessage(message);
}

void EmshopClient::getCart()
{
    QJsonObject message;
    message["type"] = "get_cart";
    
    sendMessage(message);
}

void EmshopClient::removeFromCart(qint64 productId)
{
    QJsonObject message;
    message["type"] = "remove_from_cart";
    message["productId"] = productId;
    
    sendMessage(message);
}

void EmshopClient::getUserOrders()
{
    QJsonObject message;
    message["type"] = "get_user_orders";
    
    sendMessage(message);
}

void EmshopClient::getOrderDetail(qint64 orderId)
{
    QJsonObject message;
    message["type"] = "get_order_detail";
    message["orderId"] = orderId;
    
    sendMessage(message);
}

void EmshopClient::getUserAddresses()
{
    QJsonObject message;
    message["type"] = "get_user_addresses";
    
    sendMessage(message);
}

void EmshopClient::addAddress(const QString &receiverName, const QString &phone, 
                             const QString &province, const QString &city, 
                             const QString &district, const QString &detailAddress,
                             const QString &postalCode, bool isDefault)
{
    QJsonObject message;
    message["type"] = "add_address";
    message["receiverName"] = receiverName;
    message["phone"] = phone;
    message["province"] = province;
    message["city"] = city;
    message["district"] = district;
    message["detailAddress"] = detailAddress;
    message["postalCode"] = postalCode;
    message["isDefault"] = isDefault;
    
    sendMessage(message);
}

void EmshopClient::createOrder(qint64 addressId, const QString &couponCode, const QString &remark)
{
    QJsonObject message;
    message["type"] = "create_order";
    message["addressId"] = addressId;
    if (!couponCode.isEmpty()) {
        message["couponCode"] = couponCode;
    }
    if (!remark.isEmpty()) {
        message["remark"] = remark;
    }
    
    sendMessage(message);
}

void EmshopClient::processPayment(qint64 orderId, const QString &paymentMethod, double amount,
                                 const QJsonObject &paymentDetails)
{
    QJsonObject message;
    message["type"] = "process_payment";
    message["orderId"] = orderId;
    message["paymentMethod"] = paymentMethod;
    message["amount"] = amount;
    if (!paymentDetails.isEmpty()) {
        message["paymentDetails"] = paymentDetails;
    }
    
    sendMessage(message);
}

void EmshopClient::onConnected()
{
    qDebug() << "Connected to WebSocket server";
    m_connectionState = Connected;
    emit connectionStateChanged(m_connectionState);
    emit connected();
    
    // 启动心跳
    m_heartbeatTimer->start();
}

void EmshopClient::onDisconnected()
{
    qDebug() << "Disconnected from WebSocket server";
    m_connectionState = Disconnected;
    m_authenticated = false;
    m_heartbeatTimer->stop();
    
    emit connectionStateChanged(m_connectionState);
    emit disconnected();
}

void EmshopClient::onTextMessageReceived(const QString &message)
{
    qDebug() << "Received message:" << message;
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON message:" << error.errorString();
        return;
    }
    
    QJsonObject response = doc.object();
    handleResponse(response);
}

void EmshopClient::onError(QAbstractSocket::SocketError error)
{
    QString errorString;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            errorString = "Connection refused";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorString = "Remote host closed connection";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorString = "Host not found";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorString = "Socket timeout";
            break;
        case QAbstractSocket::NetworkError:
            errorString = "Network error";
            break;
        case QAbstractSocket::SslHandshakeFailedError:
            errorString = "SSL handshake failed";
            break;
        default:
            errorString = QString("Unknown error (%1)").arg(static_cast<int>(error));
    }
    
    qWarning() << "WebSocket error:" << errorString;
    emit this->error(errorString);
}

void EmshopClient::onSslErrors(const QList<QSslError> &errors)
{
    qWarning() << "SSL errors occurred:";
    for (const QSslError &error : errors) {
        qWarning() << "  -" << error.errorString();
    }
    
    // 在开发环境中忽略自签名证书错误
    // 在生产环境中应该移除这行代码
    m_webSocket->ignoreSslErrors();
}

void EmshopClient::sendHeartbeat()
{
    if (m_connectionState == Connected || m_connectionState == Authenticated) {
        QJsonObject pingMessage;
        pingMessage["type"] = "ping";
        sendMessage(pingMessage);
    }
}

void EmshopClient::sendMessage(const QJsonObject &message)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "WebSocket not connected";
        return;
    }
    
    QJsonDocument doc(message);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    
    qDebug() << "Sending message:" << jsonString;
    m_webSocket->sendTextMessage(jsonString);
}

void EmshopClient::handleResponse(const QJsonObject &response)
{
    QString type = response["type"].toString();
    bool success = response["success"].toBool();
    QString message = response["message"].toString();
    QJsonObject data = response["data"].toObject();
    
    if (type == "welcome") {
        qDebug() << "Received welcome message:" << message;
    } else if (type == "auth") {
        if (success) {
            m_authenticated = true;
            m_connectionState = Authenticated;
            m_userInfo = data;
            emit connectionStateChanged(m_connectionState);
            emit authenticated(data);
            
            // 自动订阅一些基础话题
            subscribeTopic("stock_updates");
            subscribeTopic("price_updates");
            subscribeTopic("promotions");
        } else {
            emit authenticationFailed(message);
        }
    } else if (type == "pong") {
        // 心跳响应
        qDebug() << "Received pong";
    } else if (type == "get_products") {
        if (success) {
            emit productsReceived(data);
        }
    } else if (type == "search_products") {
        if (success) {
            emit searchResultsReceived(data);
        }
    } else if (type == "add_to_cart") {
        if (success) {
            emit cartUpdated(data);
        }
    } else if (type == "get_cart") {
        if (success) {
            emit cartReceived(data);
        }
    } else if (type == "remove_from_cart") {
        if (success) {
            emit cartUpdated(data);
        }
    } else if (type == "get_user_orders") {
        if (success) {
            emit ordersReceived(data);
        }
    } else if (type == "get_order_detail") {
        if (success) {
            emit orderDetailReceived(data);
        }
    } else if (type == "get_user_addresses") {
        if (success) {
            emit addressesReceived(data);
        }
    } else if (type == "add_address") {
        if (success) {
            emit addressAdded(data);
        }
    } else if (type == "create_order") {
        if (success) {
            emit orderCreated(data);
        }
    } else if (type == "process_payment") {
        if (success) {
            emit paymentProcessed(data);
        }
    } else if (type == "order_status_update") {
        qint64 orderId = data["order_id"].toVariant().toLongLong();
        QString status = data["status"].toString();
        QString statusMessage = data["message"].toString();
        emit orderStatusUpdated(orderId, status, statusMessage);
    } else if (type == "stock_update") {
        qint64 productId = data["product_id"].toVariant().toLongLong();
        int newStock = data["stock"].toInt();
        emit stockUpdated(productId, newStock);
    } else if (type == "price_update") {
        qint64 productId = data["product_id"].toVariant().toLongLong();
        double newPrice = data["price"].toDouble();
        emit priceUpdated(productId, newPrice);
    } else if (type == "system_notification") {
        QString title = data["title"].toString();
        QString content = data["content"].toString();
        QString level = data["level"].toString();
        emit systemNotificationReceived(title, content, level);
    } else if (type == "promotion_notification") {
        emit promotionNotificationReceived(data);
    } else if (type == "customer_service") {
        bool fromAdmin = data["from_admin"].toBool();
        QString serviceMessage = data["message"].toString();
        emit customerServiceMessageReceived(fromAdmin, serviceMessage);
    } else if (type == "statistics") {
        int onlineUsers = data["online_users"].toInt();
        int totalConnections = data["total_connections"].toInt();
        emit statisticsReceived(onlineUsers, totalConnections);
    } else {
        qDebug() << "Unknown message type:" << type;
    }
    
    // 如果操作失败，输出错误信息
    if (!success && !message.isEmpty()) {
        qWarning() << "Operation failed:" << type << "-" << message;
        emit error(QString("%1: %2").arg(type, message));
    }
}

void EmshopClient::setupHeartbeat()
{
    connect(m_heartbeatTimer, &QTimer::timeout, this, &EmshopClient::sendHeartbeat);
    m_heartbeatTimer->setSingleShot(false);
    m_heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
}

void EmshopClient::setupSslConfiguration()
{
    if (m_webSocket) {
        QSslConfiguration sslConfig = m_webSocket->sslConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 开发环境中忽略证书验证
        m_webSocket->setSslConfiguration(sslConfig);
    }
}