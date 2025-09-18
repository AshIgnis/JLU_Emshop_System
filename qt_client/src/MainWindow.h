#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QJsonObject>
#include "ClientAdapter.h"

class ProductListWidget;
class CartWidget;
class OrderWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void setEmshopClient(ClientAdapter *client);

private slots:
    void onConnectionStateChanged(ClientAdapter::ConnectionState state);
    void onAuthenticated(const QJsonObject &userInfo);
    void onSystemNotificationReceived(const QString &title, const QString &content, const QString &level);
    void onRefreshData();
    void onDisconnect();

private:
    void setupUI();
    void setupMenuBar();
    void updateStatusBar();
    
    ClientAdapter *m_client;
    
    // UI 组件
    QTabWidget *m_tabWidget;
    ProductListWidget *m_productListWidget;
    CartWidget *m_cartWidget;
    OrderWidget *m_orderWidget;
    
    QLabel *m_statusLabel;
    QLabel *m_userLabel;
    QPushButton *m_refreshButton;
    QPushButton *m_disconnectButton;
    
    QJsonObject m_userInfo;
};

#endif // MAINWINDOW_H