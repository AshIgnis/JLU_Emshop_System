#ifndef EMSHOPCLIENT_H
#define EMSHOPCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QSslConfiguration>

/**
 * Emshop WebSocket 客户端
 * 提供与 Netty 服务器的 WebSocket 连接和通信功能
 */
class EmshopClient : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Authenticated
    };

    explicit EmshopClient(QObject *parent = nullptr);
    ~EmshopClient();

    // 连接管理
    void connectToServer(const QString &url);
    void disconnectFromServer();
    ConnectionState connectionState() const;
    
    // 认证
    void authenticate(const QString &username, const QString &password);
    bool isAuthenticated() const;
    
    // 订阅管理
    void subscribeTopic(const QString &topic);
    void unsubscribeTopic(const QString &topic);
    
    // 业务操作
    void getProducts(const QString &category = "all", int page = 1, int pageSize = 10);
    void searchProducts(const QString &keyword, int page = 1, int pageSize = 10, 
                       const QString &sortBy = "id", double minPrice = 0.0, double maxPrice = 99999.0);
    void addToCart(qint64 productId, int quantity);
    void getCart();
    void removeFromCart(qint64 productId);
    void getUserOrders();
    void getOrderDetail(qint64 orderId);
    void getUserAddresses();
    void addAddress(const QString &receiverName, const QString &phone, 
                   const QString &province, const QString &city, 
                   const QString &district, const QString &detailAddress,
                   const QString &postalCode = "000000", bool isDefault = false);
    void createOrder(qint64 addressId, const QString &couponCode = QString(), 
                    const QString &remark = QString());
    void processPayment(qint64 orderId, const QString &paymentMethod, double amount,
                       const QJsonObject &paymentDetails = QJsonObject());

signals:
    // 连接状态信号
    void connectionStateChanged(EmshopClient::ConnectionState state);
    void connected();
    void disconnected();
    void error(const QString &error);
    
    // 认证信号
    void authenticated(const QJsonObject &userInfo);
    void authenticationFailed(const QString &error);
    
    // 业务响应信号
    void productsReceived(const QJsonObject &data);
    void searchResultsReceived(const QJsonObject &data);
    void cartUpdated(const QJsonObject &data);
    void cartReceived(const QJsonObject &data);
    void ordersReceived(const QJsonObject &data);
    void orderDetailReceived(const QJsonObject &data);
    void addressesReceived(const QJsonObject &data);
    void addressAdded(const QJsonObject &data);
    void orderCreated(const QJsonObject &data);
    void paymentProcessed(const QJsonObject &data);
    
    // 推送消息信号
    void orderStatusUpdated(qint64 orderId, const QString &status, const QString &message);
    void stockUpdated(qint64 productId, int newStock);
    void priceUpdated(qint64 productId, double newPrice);
    void systemNotificationReceived(const QString &title, const QString &content, const QString &level);
    void promotionNotificationReceived(const QJsonObject &promotion);
    void customerServiceMessageReceived(bool fromAdmin, const QString &message);
    void statisticsReceived(int onlineUsers, int totalConnections);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);
    void sendHeartbeat();

private:
    void sendMessage(const QJsonObject &message);
    void handleResponse(const QJsonObject &response);
    void setupHeartbeat();
    void setupSslConfiguration();
    
    QWebSocket *m_webSocket;
    QTimer *m_heartbeatTimer;
    ConnectionState m_connectionState;
    bool m_authenticated;
    QString m_serverUrl;
    QJsonObject m_userInfo;
    QStringList m_subscribedTopics;
    
    // 心跳配置
    static const int HEARTBEAT_INTERVAL = 30000; // 30 seconds
};

#endif // EMSHOPCLIENT_H