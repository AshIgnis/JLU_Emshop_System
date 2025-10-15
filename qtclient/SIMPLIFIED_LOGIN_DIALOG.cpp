// 简化版LoginDialog构造函数 - 无R"()"原始字符串

LoginDialog::LoginDialog(ApplicationContext &context, QWidget *parent)
    : QDialog(parent)
    , m_context(context)
{
    setWindowTitle(tr("Emshop 客户端登录"));
    setModal(true);
    setMinimumWidth(480);
    setMinimumHeight(550);

    // 简化的对话框样式（无渐变）
    setStyleSheet("QDialog { background-color: #f5f7fa; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Logo 和标题区域
    auto *titleLabel = new QLabel(tr("🛒 Emshop"), this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "  color: #2c3e50;"
        "  font-size: 28pt;"
        "  font-weight: 700;"
        "  background: transparent;"
        "  border: none;"
        "}");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel(tr("欢迎使用电商管理系统"), this);
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "  color: #7f8c8d;"
        "  font-size: 11pt;"
        "  background: transparent;"
        "  border: none;"
        "}");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(10);

    // 白色卡片容器
    auto *cardWidget = new QWidget(this);
    cardWidget->setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border-radius: 12px;"
        "}");
    
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(16);

    // 表单布局
    auto *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(12);

    // 样式化的输入框
    QString inputStyle =
        "QLineEdit, QSpinBox {"
        "  border: 2px solid #dfe6e9;"
        "  border-radius: 8px;"
        "  padding: 10px 14px;"
        "  background-color: white;"
        "  font-size: 10pt;"
        "}"
        "QLineEdit:focus, QSpinBox:focus {"
        "  border-color: #3498db;"
        "  background-color: #f8f9fa;"
        "}";

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText(QStringLiteral("127.0.0.1"));
    m_hostEdit->setStyleSheet(inputStyle);
    m_hostEdit->setPlaceholderText(tr("服务器IP地址"));
    formLayout->addRow(tr("🌐 服务器"), m_hostEdit);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(8081);
    m_portSpin->setStyleSheet(inputStyle);
    formLayout->addRow(tr("🔌 端口"), m_portSpin);

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setStyleSheet(inputStyle);
    m_usernameEdit->setPlaceholderText(tr("请输入用户名"));
    formLayout->addRow(tr("👤 用户名"), m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet(inputStyle);
    m_passwordEdit->setPlaceholderText(tr("请输入密码"));
    formLayout->addRow(tr("🔒 密码"), m_passwordEdit);

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setStyleSheet(inputStyle);
    m_phoneEdit->setPlaceholderText(tr("注册时需要填写手机号"));
    formLayout->addRow(tr("📱 手机号"), m_phoneEdit);

    cardLayout->addLayout(formLayout);

    // 状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "  padding: 10px;"
        "  border-radius: 6px;"
        "  background-color: #e8f5e9;"
        "  color: #2e7d32;"
        "  font-size: 9pt;"
        "}");
    cardLayout->addWidget(m_statusLabel);

    // 按钮样式（简化版 - 无渐变）
    QString primaryBtnStyle =
        "QPushButton {"
        "  background-color: #3498db;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 24px;"
        "  font-weight: 600;"
        "  font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #999999;"
        "}";

    QString secondaryBtnStyle =
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #3498db;"
        "  border: 2px solid #3498db;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 500;"
        "  font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(52, 152, 219, 0.1);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(52, 152, 219, 0.2);"
        "}";

    m_loginButton = new QPushButton(tr("🚀 登录"), this);
    m_loginButton->setStyleSheet(primaryBtnStyle);
    m_loginButton->setMinimumHeight(44);
    cardLayout->addWidget(m_loginButton);

    auto *actionButtonLayout = new QHBoxLayout;
    actionButtonLayout->setSpacing(12);

    m_registerButton = new QPushButton(tr("📝 注册新账号"), this);
    m_registerButton->setStyleSheet(secondaryBtnStyle);
    actionButtonLayout->addWidget(m_registerButton);

    m_pingButton = new QPushButton(tr("🔍 测试连接"), this);
    m_pingButton->setStyleSheet(secondaryBtnStyle);
    actionButtonLayout->addWidget(m_pingButton);

    cardLayout->addLayout(actionButtonLayout);

    mainLayout->addWidget(cardWidget);

    // 底部信息
    auto *footerLabel = new QLabel(tr("© 2025 Emshop - 吉林大学项目"), this);
    footerLabel->setStyleSheet(
        "QLabel {"
        "  color: #7f8c8d;"
        "  font-size: 8pt;"
        "  background: transparent;"
        "  border: none;"
        "}");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::attemptRegister);
    connect(m_pingButton, &QPushButton::clicked, this, &LoginDialog::testPing);

    showStatusMessage(tr("✨ 请输入服务器信息和账号"), true);
}

// showStatusMessage 简化版
void LoginDialog::showStatusMessage(const QString &message, bool success)
{
    QString style = success ?
        "QLabel {"
        "  padding: 10px;"
        "  border-radius: 6px;"
        "  background-color: #e8f5e9;"
        "  color: #2e7d32;"
        "  font-size: 9pt;"
        "  font-weight: 500;"
        "}" :
        "QLabel {"
        "  padding: 10px;"
        "  border-radius: 6px;"
        "  background-color: #ffebee;"
        "  color: #c62828;"
        "  font-size: 9pt;"
        "  font-weight: 500;"
        "}";
    m_statusLabel->setStyleSheet(style);
    m_statusLabel->setText(message);
}
