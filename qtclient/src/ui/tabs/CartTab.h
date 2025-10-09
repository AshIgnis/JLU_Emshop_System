#pragma once

#include <QWidget>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <functional>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;
class QLabel;
class QTimer;

class ApplicationContext;
struct UserSession;

struct AddressRecord {
    qlonglong id = -1;
    QString display;
    QJsonObject raw;
};

class CartTab : public QWidget {
    Q_OBJECT
public:
    explicit CartTab(ApplicationContext &context, QWidget *parent = nullptr);

signals:
    void statusMessage(const QString &message, bool success);
    void orderCreated();
    void orderCreatedWithStock(const QJsonArray &stockChanges); // 新增：携带库存变动

public slots:
    void refreshCart();
    void refreshAddresses();
    void handleSessionChanged(const UserSession &session);

private slots:
    void updateItemQuantity();
    void removeSelectedItem();
    void clearCart();
    void createOrder();
    void addNewAddress();
    void updateDetailView();
    void handleItemChanged(QTableWidgetItem *item); // 勾选状态改变
    void handleToggleAllSelect(bool select);        // 全选/全不选
    void handleCouponSelectionChanged(int index);   // 优惠券选择切换
    void updateSummaryPreview();                    // 预估优惠与应付预览
    void updateCouponInfoHint();                    // 展示当前优惠券详情

private:
    void sendCartCommand(const QString &command, const QString &successAction,
                         std::function<void(const QJsonDocument &)> onSuccess = {});
    void populateCart(const QJsonDocument &doc);
    void populateAddresses(const QJsonDocument &doc);
    qlonglong selectedProductId() const;
    void refreshUserCoupons(); // 新增：刷新用户优惠券
    QJsonObject selectedCartItem() const;
    AddressRecord currentAddress() const;
    void setLoggedIn(bool loggedIn);

    ApplicationContext &m_context;
    QTableWidget *m_cartTable;
    QPlainTextEdit *m_detailView;
    QLabel *m_summaryLabel;
    QLabel *m_couponInfoLabel {nullptr};
    QLabel *m_warningLabel {nullptr};
    QComboBox *m_addressCombo;
    QComboBox *m_couponCombo {nullptr}; // 新增：优惠券展示与选择
    QLineEdit *m_couponEdit;
    QLineEdit *m_remarkEdit;
    QSpinBox *m_quantitySpin;

    QVector<AddressRecord> m_addresses;
    bool m_loggedIn = false;
    bool m_updatingTable = false; // 避免填充表格时触发 itemChanged
    double m_lastCartTotal = 0.0;  // 最近一次购物车原始合计（未应用客户端预览优惠）
    int m_couponCount = 0;         // 用户拥有的优惠券数量
    QTimer *m_refreshTimer {nullptr};
};
