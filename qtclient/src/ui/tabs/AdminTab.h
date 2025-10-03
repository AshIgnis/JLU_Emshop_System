#pragma once

#include <QWidget>
#include <functional>

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QComboBox;
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

private:
    void setupUi();
    void sendCommand(const QString &cmd,
                     std::function<void(const QJsonDocument&)> onSuccess,
                     const QString &actionLabel);

    ApplicationContext &m_context;
    QTabWidget *m_tabs {nullptr};
    // 库存
    QTableWidget *m_lowStockTable {nullptr};
    QLineEdit *m_stockProductId {nullptr};
    QSpinBox *m_stockDelta {nullptr};
    QComboBox *m_stockOp {nullptr};
    QPushButton *m_stockApplyBtn {nullptr};
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
};
