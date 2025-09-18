#include "LoginDialog.h"
#include <QMessageBox>
#include <QApplication>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_connected(false)
{
    setWindowTitle("JLU Emshop 电商系统 - 登录");
    setModal(true);
    setFixedSize(450, 550);
    
    // 设置窗口样式
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #667eea, stop: 1 #764ba2);
        }
        QLabel {
            color: white;
            font-weight: bold;
        }
        QLineEdit {
            border: 2px solid #ddd;
            border-radius: 8px;
            padding: 10px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #4CAF50;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        QGroupBox {
            color: white;
            font-weight: bold;
            font-size: 14px;
            border: 2px solid rgba(255,255,255,0.3);
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
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
    QString address = m_serverUrlEdit->text().trimmed();
    if (address.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入服务器地址");
        return;
    }
    
    // 解析主机和端口
    QString host;
    quint16 port = 8080;  // 默认端口
    
    if (address.contains(":")) {
        QStringList parts = address.split(":");
        host = parts[0];
        if (parts.size() > 1) {
            bool ok;
            port = parts[1].toUShort(&ok);
            if (!ok) {
                QMessageBox::warning(this, "输入错误", "端口格式不正确");
                return;
            }
        }
    } else {
        host = address;
    }
    
    m_connected = false;
    emit connectRequested(host, port);
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
    m_serverUrlEdit->setPlaceholderText("例如: localhost:8080");
    m_serverUrlEdit->setText("localhost:8080");
    
    m_connectButton = new QPushButton("🔗 连接服务器", this);
    m_connectButton->setStyleSheet("background-color: #2196F3;");
    
    serverLayout->addWidget(serverLabel);
    serverLayout->addWidget(m_serverUrlEdit);
    serverLayout->addWidget(m_connectButton);
    
    mainLayout->addWidget(serverGroup);
    
    // 用户登录组
    QGroupBox *loginGroup = new QGroupBox("👤 用户登录", this);
    QVBoxLayout *loginLayout = new QVBoxLayout(loginGroup);
    
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
    
    mainLayout->addWidget(loginGroup);
    
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
    loginGroup->setEnabled(false);
    
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