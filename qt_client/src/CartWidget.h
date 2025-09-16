#ifndef CARTWIDGET_H
#define CARTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QJsonObject>
#include "EmshopClient.h"

class CartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CartWidget(QWidget *parent = nullptr);
    
    void setEmshopClient(EmshopClient *client);
    void refreshCart();

private slots:
    void onCartReceived(const QJsonObject &data);
    void onRemoveFromCartClicked();
    void onCheckoutClicked();

private:
    void setupUI();
    void loadCartItems(const QJsonObject &cartData);
    void updateTotal(double total);
    
    EmshopClient *m_client;
    
    QTreeWidget *m_cartTree;
    QPushButton *m_removeButton;
    QPushButton *m_checkoutButton;
    QPushButton *m_refreshButton;
    QLabel *m_totalLabel;
    QLabel *m_statusLabel;
};

#endif // CARTWIDGET_H