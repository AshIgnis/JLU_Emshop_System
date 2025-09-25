#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QLabel;

class ApplicationContext;

struct PaymentRequest {
    QString method;
    double amount = 0.0;
    QString accountInfo;
};

class PaymentDialog : public QDialog {
    Q_OBJECT
public:
    PaymentDialog(ApplicationContext &context, qlonglong orderId, double suggestedAmount, QWidget *parent = nullptr);

    PaymentRequest request() const;

private slots:
    void fetchMethodsFromServer();
    void updateStatus(const QString &message, bool ok = true);

private:
    void populateDefaultMethods();

    ApplicationContext &m_context;
    qlonglong m_orderId;
    QComboBox *m_methodCombo;
    QDoubleSpinBox *m_amountSpin;
    QLineEdit *m_accountInfoEdit;
    QLabel *m_statusLabel;
};
