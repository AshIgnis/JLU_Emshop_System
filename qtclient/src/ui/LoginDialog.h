#pragma once

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QLabel;
class QPushButton;

class ApplicationContext;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(ApplicationContext &context, QWidget *parent = nullptr);

private slots:
    void attemptLogin();
    void attemptRegister();
    void testPing();

private:
    void setBusy(bool busy);
    void showStatusMessage(const QString &message, bool success = true);
    void handleLoginSuccess(const QString &responsePayload);

    ApplicationContext &m_context;
    QLineEdit *m_hostEdit;
    QSpinBox *m_portSpin;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_phoneEdit;
    QLabel *m_statusLabel;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QPushButton *m_pingButton;
};
