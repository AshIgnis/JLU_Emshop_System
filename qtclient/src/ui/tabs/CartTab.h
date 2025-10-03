#pragma once

#include <QWidget>
#include <QVector>
#include <QJsonObject>
#include <functional>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;
class QTableWidgetItem;
class QLabel;

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
    QComboBox *m_addressCombo;
    QComboBox *m_couponCombo {nullptr}; // 新增：优惠券展示与选择
    QLineEdit *m_couponEdit;
    QLineEdit *m_remarkEdit;
    QSpinBox *m_quantitySpin;

    QVector<AddressRecord> m_addresses;
    bool m_loggedIn = false;
    bool m_updatingTable = false; // 避免填充表格时触发 itemChanged
};
