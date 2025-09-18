#include "ClientAdapter.h"

ClientAdapter::ClientAdapter(EmshopTcpClient *tcpClient, QObject *parent)
    : QObject(parent), m_tcpClient(tcpClient)
{
    // 转发所有信号
    connect(m_tcpClient, &EmshopTcpClient::connectionStateChanged, this, 
            [this](EmshopTcpClient::ConnectionState state) {
        emit connectionStateChanged(static_cast<ClientAdapter::ConnectionState>(state));
    });
    
    connect(m_tcpClient, &EmshopTcpClient::connected, this, &ClientAdapter::connected);
    connect(m_tcpClient, &EmshopTcpClient::disconnected, this, &ClientAdapter::disconnected);
    connect(m_tcpClient, &EmshopTcpClient::error, this, &ClientAdapter::error);
    
    connect(m_tcpClient, &EmshopTcpClient::authenticated, this, &ClientAdapter::authenticated);
    connect(m_tcpClient, &EmshopTcpClient::authenticationFailed, this, &ClientAdapter::authenticationFailed);
    
    connect(m_tcpClient, &EmshopTcpClient::productsReceived, this, &ClientAdapter::productsReceived);
    connect(m_tcpClient, &EmshopTcpClient::searchResultsReceived, this, &ClientAdapter::searchResultsReceived);
    connect(m_tcpClient, &EmshopTcpClient::cartUpdated, this, &ClientAdapter::cartUpdated);
    connect(m_tcpClient, &EmshopTcpClient::cartReceived, this, &ClientAdapter::cartReceived);
    connect(m_tcpClient, &EmshopTcpClient::ordersReceived, this, &ClientAdapter::ordersReceived);
    connect(m_tcpClient, &EmshopTcpClient::orderDetailReceived, this, &ClientAdapter::orderDetailReceived);
    connect(m_tcpClient, &EmshopTcpClient::addressesReceived, this, &ClientAdapter::addressesReceived);
    connect(m_tcpClient, &EmshopTcpClient::addressAdded, this, &ClientAdapter::addressAdded);
    connect(m_tcpClient, &EmshopTcpClient::orderCreated, this, &ClientAdapter::orderCreated);
    connect(m_tcpClient, &EmshopTcpClient::paymentProcessed, this, &ClientAdapter::paymentProcessed);
    connect(m_tcpClient, &EmshopTcpClient::orderCancelled, this, &ClientAdapter::orderCancelled);
    
    connect(m_tcpClient, &EmshopTcpClient::orderStatusUpdated, this, &ClientAdapter::orderStatusUpdated);
    connect(m_tcpClient, &EmshopTcpClient::stockUpdated, this, &ClientAdapter::stockUpdated);
    connect(m_tcpClient, &EmshopTcpClient::priceUpdated, this, &ClientAdapter::priceUpdated);
    connect(m_tcpClient, &EmshopTcpClient::systemNotificationReceived, this, &ClientAdapter::systemNotificationReceived);
}

void ClientAdapter::getProducts(const QString &category, int page, int pageSize)
{
    m_tcpClient->getProducts(category, page, pageSize);
}

void ClientAdapter::searchProducts(const QString &keyword, int page, int pageSize, 
                                  const QString &sortBy, double minPrice, double maxPrice)
{
    m_tcpClient->searchProducts(keyword, page, pageSize, sortBy, minPrice, maxPrice);
}

void ClientAdapter::addToCart(qint64 productId, int quantity)
{
    m_tcpClient->addToCart(productId, quantity);
}

void ClientAdapter::getCart()
{
    m_tcpClient->getCart();
}

void ClientAdapter::removeFromCart(qint64 productId)
{
    m_tcpClient->removeFromCart(productId);
}

void ClientAdapter::updateCartQuantity(qint64 productId, int quantity)
{
    m_tcpClient->updateCartQuantity(productId, quantity);
}

void ClientAdapter::clearCart()
{
    m_tcpClient->clearCart();
}

void ClientAdapter::getUserOrders()
{
    m_tcpClient->getUserOrders();
}

void ClientAdapter::getOrderDetail(qint64 orderId)
{
    m_tcpClient->getOrderDetail(orderId);
}

void ClientAdapter::getUserAddresses()
{
    m_tcpClient->getUserAddresses();
}

void ClientAdapter::addAddress(const QString &receiverName, const QString &phone, 
                              const QString &province, const QString &city, 
                              const QString &district, const QString &detailAddress,
                              const QString &postalCode, bool isDefault)
{
    m_tcpClient->addAddress(receiverName, phone, province, city, district, detailAddress, postalCode, isDefault);
}

void ClientAdapter::createOrder(qint64 addressId, const QString &couponCode, const QString &remark)
{
    m_tcpClient->createOrder(addressId, couponCode, remark);
}

void ClientAdapter::processPayment(qint64 orderId, const QString &paymentMethod, double amount,
                                  const QJsonObject &paymentDetails)
{
    m_tcpClient->processPayment(orderId, paymentMethod, amount, paymentDetails);
}

void ClientAdapter::cancelOrder(qint64 orderId, const QString &reason)
{
    m_tcpClient->cancelOrder(orderId, reason);
}

ClientAdapter::ConnectionState ClientAdapter::connectionState() const
{
    return static_cast<ClientAdapter::ConnectionState>(m_tcpClient->connectionState());
}

bool ClientAdapter::isAuthenticated() const
{
    return m_tcpClient->isAuthenticated();
}