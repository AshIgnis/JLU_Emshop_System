#pragma once

#include <QWidget>
#include <functional>
#include <QCheckBox>
#include <QTimer>

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;
class QCheckBox;
class QTimer;
class QPlainTextEdit;
class QLabel;
class QDateEdit;
class QJsonDocument;

class ApplicationContext;
struct UserSession;

class AdminTab : public QWidget {
    Q_OBJECT
public:
    explicit AdminTab(ApplicationContext &context, QWidget *parent = nullptr);
    void handleSessionChanged(const UserSession &session);

signals:
    void statusMessage(const QString &message, bool success);

private slots:
    // 库存
    void refreshLowStock();
    void applyStockChange();
    // 订单
    void refreshAllOrders();
    void prevPage();
    void nextPage();
    void changeOrderStatus(qlonglong orderId);
    void viewOrderDetail(qlonglong orderId);
    void refundOrder(qlonglong orderId);
    // 优惠券/促销
    void refreshPromotions();
    void createPromotion();
    // 用户管理
    void refreshUsers();
    void showSelectedUserDetail();
    void applyUserRole();
    void toggleUserStatus();
    void issueCouponToUser();
    void refreshAvailableCoupons();
    void fetchUserOrders();
    void fetchUserCoupons();

private:
    void setupUi();
    void sendCommand(const QString &cmd,
                     std::function<void(const QJsonDocument&)> onSuccess,
                     const QString &actionLabel);
    qlonglong selectedUserId() const;

    ApplicationContext &m_context;
    QTabWidget *m_tabs {nullptr};
    // 库存
    QTableWidget *m_lowStockTable {nullptr};
    QSpinBox *m_lowStockThreshold {nullptr};
    QLineEdit *m_stockProductId {nullptr};
    QSpinBox *m_stockDelta {nullptr};
    QComboBox *m_stockOp {nullptr};
    QPushButton *m_stockApplyBtn {nullptr};
    QCheckBox *m_autoRefreshLowStock {nullptr};
    QTimer *m_lowStockTimer {nullptr};
    // 订单
    QTableWidget *m_ordersTable {nullptr};
    QComboBox *m_orderStatus {nullptr};
    QDateEdit *m_startDate {nullptr};
    QDateEdit *m_endDate {nullptr};
    QSpinBox *m_page {nullptr};
    QSpinBox *m_pageSize {nullptr};
    QPushButton *m_prevPage {nullptr};
    QPushButton *m_nextPage {nullptr};
    // 优惠券/促销
    QTableWidget *m_promotionsTable {nullptr};
    QLineEdit *m_promoName {nullptr};
    QLineEdit *m_promoCode {nullptr};
    QLineEdit *m_promoJson {nullptr};
    QPushButton *m_promoCreateBtn {nullptr};
    // 用户管理
    QTableWidget *m_usersTable {nullptr};
    QLineEdit *m_userSearchEdit {nullptr};
    QComboBox *m_userRoleCombo {nullptr};
    QLabel *m_userDetailLabel {nullptr};
    QPlainTextEdit *m_userOrdersView {nullptr};
    QPlainTextEdit *m_userCouponsView {nullptr};
    QComboBox *m_couponIssueCombo {nullptr};
    QPushButton *m_couponRefreshBtn {nullptr};
};
