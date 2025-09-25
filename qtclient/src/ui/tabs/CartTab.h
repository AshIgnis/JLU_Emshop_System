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

private:
    void sendCartCommand(const QString &command, const QString &successAction,
                         std::function<void(const QJsonDocument &)> onSuccess = {});
    void populateCart(const QJsonDocument &doc);
    void populateAddresses(const QJsonDocument &doc);
    qlonglong selectedProductId() const;
    QJsonObject selectedCartItem() const;
    AddressRecord currentAddress() const;
    void setLoggedIn(bool loggedIn);

    ApplicationContext &m_context;
    QTableWidget *m_cartTable;
    QPlainTextEdit *m_detailView;
    QLabel *m_summaryLabel;
    QComboBox *m_addressCombo;
    QLineEdit *m_couponEdit;
    QLineEdit *m_remarkEdit;
    QSpinBox *m_quantitySpin;

    QVector<AddressRecord> m_addresses;
    bool m_loggedIn = false;
};
