#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QProgressBar>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    QString username() const;
    QString password() const;
    QString serverUrl() const;

public slots:
    void setConnecting(bool connecting);
    void showError(const QString &error);
    void onConnected();  // 连接成功时调用

signals:
    void loginRequested(const QString &username, const QString &password);
    void connectRequested(const QString &serverUrl);

private slots:
    void onConnectClicked();
    void onLoginClicked();

private:
    void setupUI();
    
    QLineEdit *m_serverUrlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_connectButton;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    
    bool m_connected;
};

#endif // LOGINDIALOG_H