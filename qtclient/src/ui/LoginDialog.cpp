#include "ui/LoginDialog.h"

#include "core/ApplicationContext.h"
#include "network/NetworkClient.h"
#include "utils/JsonUtils.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

namespace {
QString styleForStatus(bool success)
{
    return success ? QStringLiteral("color:#188038;") : QStringLiteral("color:#c5221f;");
}

qint64 extractUserId(const QJsonDocument &doc)
{
    static const QStringList candidatePaths = {
        QStringLiteral("data.user_id"),
        QStringLiteral("data.userInfo.user_id"),
        QStringLiteral("data.user_info.user_id"),
        QStringLiteral("user_id"),
        QStringLiteral("userInfo.user_id"),
        QStringLiteral("user_info.user_id")
    };

    for (const auto &path : candidatePaths) {
        QJsonValue value = JsonUtils::extract(doc, path);
        if (!value.isUndefined()) {
            qlonglong id = JsonUtils::asLongLong(value, -1);
            if (id >= 0) {
                return id;
            }
        }
    }
    return -1;
}

QString extractRole(const QJsonDocument &doc)
{
    static const QStringList candidatePaths = {
        QStringLiteral("data.role"),
        QStringLiteral("data.user_role"),
        QStringLiteral("data.user_info.role"),
        QStringLiteral("data.userInfo.role"),
        QStringLiteral("user_info.role"),
        QStringLiteral("userInfo.role"),
        QStringLiteral("user.role"),
        QStringLiteral("profile.role"),
        QStringLiteral("role"),
        QStringLiteral("roles.0"),
        QStringLiteral("data.roles.0"),
        QStringLiteral("data.user_info.roles.0"),
        QStringLiteral("data.userInfo.roles.0"),
    };

    for (const auto &path : candidatePaths) {
        QJsonValue value = JsonUtils::extract(doc, path);
        if (value.isString()) {
            const QString role = value.toString().trimmed();
            if (!role.isEmpty()) {
                return role;
            }
        }
        if (value.isArray() && !value.toArray().isEmpty()) {
            const QJsonArray array = value.toArray();
            for (const QJsonValue &entry : array) {
                if (entry.isString() && !entry.toString().trimmed().isEmpty()) {
                    return entry.toString().trimmed();
                }
                if (entry.isObject()) {
                    const QJsonValue nested = entry.toObject().value(QStringLiteral("role"));
                    if (nested.isString() && !nested.toString().trimmed().isEmpty()) {
                        return nested.toString().trimmed();
                    }
                }
            }
        }
        if (value.isObject()) {
            const QJsonValue nested = value.toObject().value(QStringLiteral("role"));
            if (nested.isString() && !nested.toString().trimmed().isEmpty()) {
                return nested.toString().trimmed();
            }
        }
    }
    return QStringLiteral("user");
}
} // namespace

LoginDialog::LoginDialog(ApplicationContext &context, QWidget *parent)
    : QDialog(parent)
    , m_context(context)
{
    setWindowTitle(tr("Emshop 客户端登录"));
    setModal(true);
    setMinimumWidth(480);
    setMinimumHeight(550);

    // 设置对话框样式
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
        }
    )");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Logo 和标题区域
    auto *titleLabel = new QLabel(tr("🛒 Emshop"), this);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: white;
            font-size: 32pt;
            font-weight: 700;
            background: transparent;
            border: none;
        }
    )");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    auto *subtitleLabel = new QLabel(tr("欢迎使用电商管理系统"), this);
    subtitleLabel->setStyleSheet(R"(
        QLabel {
            color: rgba(255, 255, 255, 0.9);
            font-size: 11pt;
            background: transparent;
            border: none;
        }
    )");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(10);

    // 白色卡片容器
    auto *cardWidget = new QWidget(this);
    cardWidget->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 16px;
        }
        QLabel {
            color: #2c3e50;
            font-weight: 600;
            background: transparent;
        }
    )");
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(16);

    // 表单布局
    auto *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    formLayout->setHorizontalSpacing(12);
    formLayout->setVerticalSpacing(12);

    // 样式化的输入框 - 增强对比度
    QString inputStyle =
        "QLineEdit, QSpinBox {"
        "  border: 2px solid #9b59b6;"
        "  border-radius: 8px;"
        "  padding: 10px 14px;"
        "  background-color: white;"
        "  color: #2c3e50;"
        "  font-size: 10pt;"
        "  font-weight: 500;"
        "}"
        "QLineEdit:focus, QSpinBox:focus {"
        "  border-color: #667eea;"
        "  border-width: 3px;"
        "  background-color: #ffffff;"
        "}"
        "QLineEdit::placeholder {"
        "  color: #95a5a6;"
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
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            padding: 10px;
            border-radius: 6px;
            background-color: #e8f5e9;
            color: #2e7d32;
            font-size: 9pt;
        }
    )");
    cardLayout->addWidget(m_statusLabel);

    // 按钮样式
    QString primaryBtnStyle =
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #667eea, stop:1 #764ba2);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 24px;"
        "  font-weight: 600;"
        "  font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #5568d3, stop:1 #6a3e91);"
        "}"
        "QPushButton:pressed {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #4d5dbd, stop:1 #5f3681);"
        "}"
        "QPushButton:disabled {"
        "  background: #cccccc;"
        "  color: #999999;"
        "}";

    QString secondaryBtnStyle =
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #667eea;"
        "  border: 2px solid #667eea;"
        "  border-radius: 8px;"
        "  padding: 10px 20px;"
        "  font-weight: 500;"
        "  font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "  background-color: rgba(102, 126, 234, 0.1);"
        "}"
        "QPushButton:pressed {"
        "  background-color: rgba(102, 126, 234, 0.2);"
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
    footerLabel->setStyleSheet(R"(
        QLabel {
            color: rgba(255, 255, 255, 0.7);
            font-size: 8pt;
            background: transparent;
            border: none;
        }
    )");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::attemptRegister);
    connect(m_pingButton, &QPushButton::clicked, this, &LoginDialog::testPing);

    showStatusMessage(tr("✨ 请输入服务器信息和账号"), true);
}

void LoginDialog::setBusy(bool busy)
{
    m_loginButton->setEnabled(!busy);
    m_registerButton->setEnabled(!busy);
    m_pingButton->setEnabled(!busy);
    if (busy) {
        setCursor(Qt::BusyCursor);
    } else {
        unsetCursor();
    }
}

void LoginDialog::showStatusMessage(const QString &message, bool success)
{
    QString style = success ? R"(
        QLabel {
            padding: 10px;
            border-radius: 6px;
            background-color: #e8f5e9;
            color: #2e7d32;
            font-size: 9pt;
            font-weight: 500;
        }
    )" : R"(
        QLabel {
            padding: 10px;
            border-radius: 6px;
            background-color: #ffebee;
            color: #c62828;
            font-size: 9pt;
            font-weight: 500;
        }
    )";
    m_statusLabel->setStyleSheet(style);
    m_statusLabel->setText(message);
}

void LoginDialog::attemptLogin()
{
    const QString host = m_hostEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_portSpin->value());
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (host.isEmpty() || username.isEmpty() || password.isEmpty()) {
        showStatusMessage(tr("请填写完整的登录信息"), false);
        return;
    }

    NetworkClient *client = m_context.networkClient();
    QString errorMessage;
    if (!client->connectToServer(host, port, &errorMessage)) {
        showStatusMessage(tr("连接服务器失败: %1").arg(errorMessage), false);
        return;
    }

    setBusy(true);
    const QString command = QStringLiteral("LOGIN %1 %2").arg(username, password);

    QPointer<LoginDialog> guard(this);
    client->sendCommand(command,
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            handleLoginSuccess(response);
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            setBusy(false);
                            showStatusMessage(tr("登录失败: %1").arg(error), false);
                        });
}

void LoginDialog::handleLoginSuccess(const QString &responsePayload)
{
    bool ok = false;
    QString errorMessage;
    QJsonDocument doc = JsonUtils::parse(responsePayload, &ok, &errorMessage);
    if (!ok) {
        setBusy(false);
        showStatusMessage(tr("服务器响应解析失败: %1").arg(errorMessage), false);
        return;
    }

    if (!JsonUtils::isSuccess(doc)) {
        setBusy(false);
        const QString serverMessage = JsonUtils::message(doc);
        showStatusMessage(serverMessage.isEmpty() ? tr("用户名或密码错误") : serverMessage, false);
        return;
    }

    UserSession session;
    session.loggedIn = true;
    session.username = m_usernameEdit->text().trimmed();
    session.userId = extractUserId(doc);
    session.role = extractRole(doc);
    session.rawLoginResponse = responsePayload;

    if (session.userId < 0) {
        setBusy(false);
        showStatusMessage(tr("登录成功，但未能获取用户ID"), false);
        return;
    }

    m_context.updateSession(session);

    showStatusMessage(tr("登录成功，欢迎 %1 (%2)").arg(session.username, session.role), true);
    setBusy(false);

    QTimer::singleShot(150, this, [this]() {
        accept();
    });
}

void LoginDialog::attemptRegister()
{
    const QString host = m_hostEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_portSpin->value());
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    const QString phone = m_phoneEdit->text().trimmed();

    if (host.isEmpty() || username.isEmpty() || password.isEmpty() || phone.isEmpty()) {
        showStatusMessage(tr("注册需要填写服务器、用户名、密码、手机号"), false);
        return;
    }

    NetworkClient *client = m_context.networkClient();
    QString errorMessage;
    if (!client->connectToServer(host, port, &errorMessage)) {
        showStatusMessage(tr("连接服务器失败: %1").arg(errorMessage), false);
        return;
    }

    setBusy(true);
    const QString command = QStringLiteral("REGISTER %1 %2 %3").arg(username, password, phone);
    QPointer<LoginDialog> guard(this);
    client->sendCommand(command,
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                showStatusMessage(tr("注册响应解析失败: %1").arg(error), false);
                            } else if (JsonUtils::isSuccess(doc)) {
                                showStatusMessage(tr("注册成功，请使用新账号登录"), true);
                            } else {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) {
                                    msg = response;
                                }
                                showStatusMessage(tr("注册失败: %1").arg(msg), false);
                            }
                            setBusy(false);
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            setBusy(false);
                            showStatusMessage(tr("注册失败: %1").arg(error), false);
                        });
}

void LoginDialog::testPing()
{
    const QString host = m_hostEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_portSpin->value());

    if (host.isEmpty()) {
        showStatusMessage(tr("请填写服务器地址"), false);
        return;
    }

    NetworkClient *client = m_context.networkClient();
    QString errorMessage;
    if (!client->connectToServer(host, port, &errorMessage)) {
        showStatusMessage(tr("连接服务器失败: %1").arg(errorMessage), false);
        return;
    }

    setBusy(true);
    QPointer<LoginDialog> guard(this);
    client->sendCommand(QStringLiteral("PING"),
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (ok && JsonUtils::isSuccess(doc)) {
                                showStatusMessage(tr("服务器在线: %1").arg(JsonUtils::message(doc)), true);
                            } else {
                                showStatusMessage(tr("PING 响应: %1").arg(response), ok);
                            }
                            setBusy(false);
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            setBusy(false);
                            showStatusMessage(tr("测试失败: %1").arg(error), false);
                        });
}
