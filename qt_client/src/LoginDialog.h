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
#include <QGroupBox>

/**
 * 登录对话框
 * 支持连接到你的原有 EmshopNettyServer.java 服务器
 * 使用TCP协议进行通信
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    QString username() const;
    QString password() const;
    QString serverUrl() const;
    
    // 状态管理
    void setStatusMessage(const QString &message);

public slots:
    void setConnecting(bool connecting);
    void showError(const QString &error);
    void onConnected();  // 连接成功时调用

signals:
    void loginRequested(const QString &username, const QString &password);
    void connectRequested(const QString &host, quint16 port);

private slots:
    void onConnectClicked();
    void onLoginClicked();

private:
    void setupUI();
    void parseServerUrl(const QString &url, QString &host, quint16 &port);
    
    QLineEdit *m_serverUrlEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_connectButton;
    QPushButton *m_loginButton;
    QPushButton *m_cancelButton;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QGroupBox *m_loginGroup;
    
    bool m_connected;
};

#endif // LOGINDIALOG_H