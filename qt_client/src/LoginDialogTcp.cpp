#include "LoginDialog.h"
#include <QMessageBox>
#include <QRegularExpression>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent), m_connected(false)
{
    setupUI();
}

QString LoginDialog::username() const
{
    return m_usernameEdit->text().trimmed();
}

QString LoginDialog::password() const
{
    return m_passwordEdit->text();
}

QString LoginDialog::serverUrl() const
{
    return m_serverUrlEdit->text().trimmed();
}

void LoginDialog::setConnecting(bool connecting)
{
    m_progressBar->setVisible(connecting);
    m_connectButton->setEnabled(!connecting);
    
    if (connecting) {
        setStatusMessage("🔗 正在连接到服务器...");
    }
}

void LoginDialog::showError(const QString &error)
{
    m_progressBar->setVisible(false);
    m_connectButton->setEnabled(true);
    m_loginButton->setEnabled(m_connected);
    m_loginGroup->setEnabled(m_connected);
    
    setStatusMessage("❌ " + error);
    QMessageBox::warning(this, "错误", error);
}

void LoginDialog::onConnected()
{
    m_connected = true;
    m_connectButton->setEnabled(true);
    m_loginButton->setEnabled(true);
    m_loginGroup->setEnabled(true);
    setStatusMessage("✅ 已连接到服务器，请登录");
}

void LoginDialog::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void LoginDialog::onConnectClicked()
{
    QString url = m_serverUrlEdit->text().trimmed();
    if (url.isEmpty()) {
        showError("🚫 请输入服务器地址");
        return;
    }
    
    QString host;
    quint16 port;
    parseServerUrl(url, host, port);
    
    m_connectButton->setEnabled(false);
    setConnecting(true);
    
    emit connectRequested(host, port);
}

void LoginDialog::onLoginClicked()
{
    if (!m_connected) {
        showError("🚫 请先连接到服务器");
        return;
    }
    
    QString user = m_usernameEdit->text().trimmed();
    QString pass = m_passwordEdit->text();
    
    if (user.isEmpty() || pass.isEmpty()) {
        showError("🚫 请输入用户名和密码");
        return;
    }
    
    m_loginButton->setEnabled(false);
    setStatusMessage("🔐 正在登录...");
    m_progressBar->setVisible(true);
    
    emit loginRequested(user, pass);
}

void LoginDialog::parseServerUrl(const QString &url, QString &host, quint16 &port)
{
    // 默认值
    host = "localhost";
    port = 8081;
    
    // 移除协议前缀（如果有的话）
    QString cleanUrl = url;
    if (cleanUrl.startsWith("http://") || cleanUrl.startsWith("ws://")) {
        cleanUrl = cleanUrl.mid(cleanUrl.indexOf("://") + 3);
    } else if (cleanUrl.startsWith("https://") || cleanUrl.startsWith("wss://")) {
        cleanUrl = cleanUrl.mid(cleanUrl.indexOf("://") + 3);
    }
    
    // 解析 host:port 格式
    int colonIndex = cleanUrl.lastIndexOf(':');
    if (colonIndex != -1) {
        host = cleanUrl.left(colonIndex);
        QString portStr = cleanUrl.mid(colonIndex + 1);
        // 移除可能的路径部分
        int slashIndex = portStr.indexOf('/');
        if (slashIndex != -1) {
            portStr = portStr.left(slashIndex);
        }
        bool ok;
        int portNum = portStr.toInt(&ok);
        if (ok && portNum > 0 && portNum <= 65535) {
            port = static_cast<quint16>(portNum);
        }
    } else {
        host = cleanUrl;
        // 移除可能的路径部分
        int slashIndex = host.indexOf('/');
        if (slashIndex != -1) {
            host = host.left(slashIndex);
        }
    }
    
    // 如果host为空，使用默认值
    if (host.isEmpty()) {
        host = "localhost";
    }
}

void LoginDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 顶部logo和标题
    QLabel *logoLabel = new QLabel("🛒", this);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet("font-size: 48px; color: white; margin: 20px;");
    
    QLabel *titleLabel = new QLabel("JLU 电商系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; color: white; font-weight: bold; margin-bottom: 10px;");
    
    QLabel *subtitleLabel = new QLabel("吉林大学 | 在线购物平台", this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("font-size: 14px; color: rgba(255,255,255,0.8); margin-bottom: 20px;");
    
    mainLayout->addWidget(logoLabel);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    
    // 服务器连接组
    QGroupBox *serverGroup = new QGroupBox("🌐 服务器连接", this);
    QVBoxLayout *serverLayout = new QVBoxLayout(serverGroup);
    
    QLabel *serverLabel = new QLabel("服务器地址:", this);
    m_serverUrlEdit = new QLineEdit(this);
    m_serverUrlEdit->setPlaceholderText("例如: localhost:8081");
    m_serverUrlEdit->setText("localhost:8081");
    
    m_connectButton = new QPushButton("🔗 连接服务器", this);
    m_connectButton->setStyleSheet("background-color: #2196F3;");
    
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(m_serverUrlEdit);
    serverLayout->addWidget(m_connectButton);
    
    mainLayout->addWidget(serverGroup);
    
    // 用户登录组
    m_loginGroup = new QGroupBox("👤 用户登录", this);
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginGroup);
    
    QLabel *userLabel = new QLabel("用户名:", this);
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入用户名");
    
    QLabel *passLabel = new QLabel("密码:", this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("请输入密码");
    
    // 快速登录按钮组
    QHBoxLayout *quickLoginLayout = new QHBoxLayout();
    QPushButton *adminLoginButton = new QPushButton("👨‍💼 管理员登录", this);
    adminLoginButton->setStyleSheet("background-color: #ff9800; font-size: 12px; padding: 8px;");
    QPushButton *userLoginButton = new QPushButton("👤 普通用户登录", this);
    userLoginButton->setStyleSheet("background-color: #9c27b0; font-size: 12px; padding: 8px;");
    
    quickLoginLayout->addWidget(adminLoginButton);
    quickLoginLayout->addWidget(userLoginButton);
    
    loginLayout->addWidget(userLabel);
    loginLayout->addWidget(m_usernameEdit);
    loginLayout->addWidget(passLabel);
    loginLayout->addWidget(m_passwordEdit);
    loginLayout->addSpacing(10);
    loginLayout->addLayout(quickLoginLayout);
    
    mainLayout->addWidget(m_loginGroup);
    
    // 操作按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_loginButton = new QPushButton("🚀 登录系统", this);
    m_loginButton->setStyleSheet("background-color: #4CAF50; font-size: 16px; padding: 15px;");
    
    m_cancelButton = new QPushButton("❌ 取消", this);
    m_cancelButton->setStyleSheet("background-color: #f44336; font-size: 14px; padding: 10px;");
    
    buttonLayout->addWidget(m_loginButton, 2);
    buttonLayout->addWidget(m_cancelButton, 1);
    
    mainLayout->addLayout(buttonLayout);
    
    // 状态显示
    m_statusLabel = new QLabel("💡 请先连接到服务器", this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(R"(
        background-color: rgba(255,255,255,0.1);
        border: 1px solid rgba(255,255,255,0.3);
        border-radius: 6px;
        padding: 8px;
        color: white;
        font-weight: bold;
    )");
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_progressBar->setVisible(false);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid rgba(255,255,255,0.3);
            border-radius: 5px;
            text-align: center;
            color: white;
            font-weight: bold;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 3px;
        }
    )");
    
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addStretch();
    
    // 初始状态
    m_loginButton->setEnabled(false);
    m_loginGroup->setEnabled(false);
    
    // 连接信号
    connect(m_connectButton, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // 快速登录功能
    connect(adminLoginButton, &QPushButton::clicked, [this]() {
        m_usernameEdit->setText("admin");
        m_passwordEdit->setText("admin123");
        m_usernameEdit->setFocus();
    });
    
    connect(userLoginButton, &QPushButton::clicked, [this]() {
        m_usernameEdit->setText("user");
        m_passwordEdit->setText("user123");
        m_usernameEdit->setFocus();
    });
    
    // 回车键登录
    connect(m_passwordEdit, &QLineEdit::returnPressed, [this]() {
        if (m_loginButton->isEnabled()) {
            onLoginClicked();
        }
    });
    
    connect(m_serverUrlEdit, &QLineEdit::returnPressed, [this]() {
        if (m_connectButton->isEnabled()) {
            onConnectClicked();
        }
    });
}