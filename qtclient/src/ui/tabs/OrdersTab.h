#pragma once

#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>

class QLabel;
class QPlainTextEdit;
class QTableWidget;

class ApplicationContext;
struct UserSession;

class OrdersTab : public QWidget {
    Q_OBJECT
public:
    explicit OrdersTab(ApplicationContext &context, QWidget *parent = nullptr);

signals:
    void statusMessage(const QString &message, bool success);

public slots:
    void refreshOrders();
    void handleSessionChanged(const UserSession &session);
    void viewOrderDetail();

private slots:
    void payForOrder();
    void cancelOrder();
    void refundOrder();
    void trackOrder();
    void deleteOrder();
    void updateDetailView();

private:
    void populateOrders(const QJsonDocument &doc);
    QJsonObject selectedOrder() const;
    qlonglong selectedOrderId() const;
    double selectedOrderAmount() const;
    void sendOrderCommand(const QString &command,
                          const QString &actionName,
                          std::function<void(const QJsonDocument &)> onSuccess = {});

    ApplicationContext &m_context;
    QTableWidget *m_orderTable;
    QPlainTextEdit *m_detailView;
    QLabel *m_summaryLabel;
    bool m_loggedIn = false;
};
