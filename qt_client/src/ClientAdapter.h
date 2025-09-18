#ifndef CLIENTADAPTER_H
#define CLIENTADAPTER_H

#include <QObject>
#include "EmshopTcpClient.h"

/**
 * 客户端适配器
 * 将 EmshopTcpClient 适配为与现有 MainWindow 兼容的接口
 * 保持向后兼容性，同时使用TCP协议连接到你的原有Netty服务器
 */
class ClientAdapter : public QObject
{
    Q_OBJECT

public:
    // 为了兼容原有代码中的EmshopClient枚举
    enum ConnectionState {
        Disconnected = EmshopTcpClient::Disconnected,
        Connecting = EmshopTcpClient::Connecting,
        Connected = EmshopTcpClient::Connected,
        Authenticated = EmshopTcpClient::Authenticated
    };

    explicit ClientAdapter(EmshopTcpClient *tcpClient, QObject *parent = nullptr);
    
    // 转发到TCP客户端的方法
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
    
    // 状态查询
    ConnectionState connectionState() const;
    bool isAuthenticated() const;

signals:
    // 兼容原有信号接口
    void connectionStateChanged(ClientAdapter::ConnectionState state);
    void connected();
    void disconnected();
    void error(const QString &error);
    
    void authenticated(const QJsonObject &userInfo);
    void authenticationFailed(const QString &error);
    
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
    
    void orderStatusUpdated(qint64 orderId, const QString &status, const QString &message);
    void stockUpdated(qint64 productId, int newStock);
    void priceUpdated(qint64 productId, double newPrice);
    void systemNotificationReceived(const QString &title, const QString &content, const QString &level);

private:
    EmshopTcpClient *m_tcpClient;
};

#endif // CLIENTADAPTER_H