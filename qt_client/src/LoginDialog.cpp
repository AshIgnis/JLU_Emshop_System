#include "LoginDialog.h"
#include <QMessageBox>
#include <QApplication>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_connected(false)
{
    setWindowTitle("Emshop 客户端 - 登录");
    setModal(true);
    setupUI();
}

QString LoginDialog::username() const
{
    return m_usernameEdit->text();
}

QString LoginDialog::password() const
{
    return m_passwordEdit->text();
}

QString LoginDialog::serverUrl() const
{
    return m_serverUrlEdit->text();
}

void LoginDialog::setConnecting(bool connecting)
{
    m_connectButton->setEnabled(!connecting);
    m_loginButton->setEnabled(!connecting && m_connected);
    m_serverUrlEdit->setEnabled(!connecting);
    
    if (connecting) {
        m_statusLabel->setText("正在连接...");
        m_statusLabel->setStyleSheet("color: blue;");
        m_progressBar->setVisible(true);
    } else {
        m_progressBar->setVisible(false);
        if (m_connected) {
            m_statusLabel->setText("已连接到服务器，请登录");
            m_statusLabel->setStyleSheet("color: green;");
            m_loginButton->setEnabled(true);
            m_usernameEdit->setFocus();
        } else {
            m_statusLabel->setText("未连接");
            m_statusLabel->setStyleSheet("color: gray;");
        }
    }
}

void LoginDialog::showError(const QString &error)
{
    m_statusLabel->setText(QString("错误: %1").arg(error));
    m_statusLabel->setStyleSheet("color: red;");
    QMessageBox::warning(this, "连接错误", error);
}

void LoginDialog::onConnectClicked()
{
    QString url = m_serverUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入服务器地址");
        return;
    }
    
    // 确保 URL 格式正确
    if (!url.startsWith("ws://") && !url.startsWith("wss://")) {
        // 默认使用 wss
        url = "wss://" + url;
    }
    
    if (!url.endsWith("/ws")) {
        url += "/ws";
    }
    
    m_serverUrlEdit->setText(url);
    m_connected = false;
    emit connectRequested(url);
}

void LoginDialog::onLoginClicked()
{
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    
    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }
    
    emit loginRequested(user, pass);
}

void LoginDialog::setupUI()
{
    setFixedSize(400, 300);
    
    // 创建控件
    m_serverUrlEdit = new QLineEdit(this);
    m_serverUrlEdit->setPlaceholderText("例如: localhost:8081");
    m_serverUrlEdit->setText("localhost:8081");
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("用户名");
    
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("密码");
    
    m_connectButton = new QPushButton("连接", this);
    m_loginButton = new QPushButton("登录", this);
    m_cancelButton = new QPushButton("取消", this);
    
    m_statusLabel = new QLabel("未连接", this);
    m_statusLabel->setStyleSheet("color: gray;");
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_progressBar->setVisible(false);
    
    // 初始状态
    m_loginButton->setEnabled(false);
    
    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 服务器连接区域
    QFormLayout *connectionLayout = new QFormLayout();
    connectionLayout->addRow("服务器地址:", m_serverUrlEdit);
    connectionLayout->addRow("", m_connectButton);
    
    mainLayout->addLayout(connectionLayout);
    mainLayout->addWidget(new QLabel("", this)); // 分隔符
    
    // 登录区域
    QFormLayout *loginLayout = new QFormLayout();
    loginLayout->addRow("用户名:", m_usernameEdit);
    loginLayout->addRow("密码:", m_passwordEdit);
    
    mainLayout->addLayout(loginLayout);
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    
    // 连接信号
    connect(m_connectButton, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 回车键处理
    connect(m_serverUrlEdit, &QLineEdit::returnPressed, this, &LoginDialog::onConnectClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    
    m_serverUrlEdit->setFocus();
}

// 成功连接时调用
void LoginDialog::onConnected()
{
    m_connected = true;
    setConnecting(false);
}