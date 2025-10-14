#pragma once

#include <QMainWindow>

class QLabel;
class QTabWidget;

class ApplicationContext;
class DashboardTab;
class ProductsTab;
class CartTab;
class OrdersTab;
class AdminTab;
class NotificationsTab;
class CouponsTab;
struct UserSession;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(ApplicationContext &context, QWidget *parent = nullptr);

private slots:
    void updateConnectionStatus(bool connected);
    void handleSessionChanged(const UserSession &session);
    void handleCartChanged();
    void handleStatusMessage(const QString &message, bool success);
    void logout();
    void showAbout();

private:
    void setupUi();
    void setupMenus();
    void updateWindowTitle();

    ApplicationContext &m_context;
    QTabWidget *m_tabWidget;
    DashboardTab *m_dashboardTab;
    ProductsTab *m_productsTab;
    CartTab *m_cartTab;
    OrdersTab *m_ordersTab;
    NotificationsTab *m_notificationsTab;
    CouponsTab *m_couponsTab;
    AdminTab *m_adminTab {nullptr};
    QLabel *m_statusConnectionLabel;
    QLabel *m_statusUserLabel;
};
