#ifndef EMSHOPTCPCLIENT_H
#define EMSHOPTCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDataStream>

/**
 * Emshop TCP 客户端
 * 与你的原有 EmshopNettyServer.java 兼容
 * 使用基于分隔符的文本协议进行通信
 */
class EmshopTcpClient : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Authenticated
    };

    explicit EmshopTcpClient(QObject *parent = nullptr);
    ~EmshopTcpClient();

    // 连接管理
    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    ConnectionState connectionState() const;
    
    // 认证
    void authenticate(const QString &username, const QString &password);
    bool isAuthenticated() const;
    
    // 业务操作
    void getProducts(const QString &category = "all", int page = 1, int pageSize = 10);
    void searchProducts(const QString &keyword, int page = 1, int pageSize = 10, 
                       const QString &sortBy = "id", double minPrice = 0.0, double maxPrice = 99999.0);
    void addToCart(qint64 productId, int quantity);
    void getCart();
    void removeFromCart(qint64 productId);
    void updateCartQuantity(qint64 productId, int quantity);
    void clearCart();
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
    void cancelOrder(qint64 orderId, const QString &reason = QString());

signals:
    // 连接状态信号
    void connectionStateChanged(EmshopTcpClient::ConnectionState state);
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
    void orderCancelled(const QJsonObject &data);
    
    // 推送消息信号
    void orderStatusUpdated(qint64 orderId, const QString &status, const QString &message);
    void stockUpdated(qint64 productId, int newStock);
    void priceUpdated(qint64 productId, double newPrice);
    void systemNotificationReceived(const QString &title, const QString &content, const QString &level);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void sendHeartbeat();

private:
    void sendMessage(const QJsonObject &message);
    void sendTextCommand(const QString &command);
    void processReceivedData();
    void handleResponse(const QJsonObject &response);
    void handleTextResponse(const QString &response);
    void setupHeartbeat();
    void setState(ConnectionState newState);
    
    QTcpSocket *m_tcpSocket;
    QTimer *m_heartbeatTimer;
    ConnectionState m_connectionState;
    bool m_authenticated;
    QString m_serverHost;
    quint16 m_serverPort;
    QJsonObject m_userInfo;
    
    // 数据接收缓冲
    QByteArray m_receiveBuffer;
    
    // 心跳配置
    static const int HEARTBEAT_INTERVAL = 30000; // 30 seconds
    static const char MESSAGE_DELIMITER = '\n';  // 与Netty服务器的分隔符一致
};

#endif // EMSHOPTCPCLIENT_H