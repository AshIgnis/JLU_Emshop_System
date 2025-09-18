#ifndef ORDERWIDGET_H
#define ORDERWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QJsonObject>
#include "EmshopClient.h"

class OrderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderWidget(QWidget *parent = nullptr);
    
    void setEmshopClient(EmshopClient *client);
    void refreshOrders();

private slots:
    void onOrdersReceived(const QJsonObject &data);
    void onOrderStatusUpdated(qint64 orderId, const QString &status, const QString &message);

private:
    void setupUI();
    void loadOrdersToTree(const QJsonObject &ordersData);
    void filterOrdersByStatus(const QString &status);
    
    EmshopClient *m_client;
    
    QTreeWidget *m_orderTree;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;
};

#endif // ORDERWIDGET_H