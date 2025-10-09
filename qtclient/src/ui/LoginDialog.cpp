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
    setMinimumWidth(420);

    auto *formLayout = new QFormLayout;

    m_hostEdit = new QLineEdit(this);
    m_hostEdit->setText(QStringLiteral("127.0.0.1"));
    formLayout->addRow(tr("服务器地址"), m_hostEdit);

    m_portSpin = new QSpinBox(this);
    m_portSpin->setRange(1, 65535);
    // 默认端口调整为 8081 (后端当前监听端口)
    m_portSpin->setValue(8081);
    formLayout->addRow(tr("端口"), m_portSpin);

    m_usernameEdit = new QLineEdit(this);
    formLayout->addRow(tr("用户名"), m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(tr("密码"), m_passwordEdit);

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText(tr("注册时需要填写手机号"));
    formLayout->addRow(tr("手机号"), m_phoneEdit);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(styleForStatus(true));
    formLayout->addRow(tr("状态"), m_statusLabel);

    m_loginButton = new QPushButton(tr("登录"), this);
    m_registerButton = new QPushButton(tr("注册"), this);
    m_pingButton = new QPushButton(tr("测试连接"), this);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    buttonBox->addButton(m_loginButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(m_registerButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_pingButton, QDialogButtonBox::ActionRole);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::reject);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::attemptRegister);
    connect(m_pingButton, &QPushButton::clicked, this, &LoginDialog::testPing);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);

    showStatusMessage(tr("请输入服务器信息和账号"), true);
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
    m_statusLabel->setStyleSheet(styleForStatus(success));
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
