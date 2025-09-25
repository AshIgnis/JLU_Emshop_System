#include "ui/tabs/DashboardTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

DashboardTab::DashboardTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    m_connectionLabel = new QLabel(this);
    m_userLabel = new QLabel(this);

    auto *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_connectionLabel);
    infoLayout->addWidget(m_userLabel);

    m_systemStatusButton = new QPushButton(tr("系统状态"), this);
    m_featureStatusButton = new QPushButton(tr("功能完成度"), this);
    m_initButton = new QPushButton(tr("初始化 JNI 服务"), this);
    m_pingButton = new QPushButton(tr("心跳检测"), this);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_systemStatusButton);
    buttonLayout->addWidget(m_featureStatusButton);
    buttonLayout->addWidget(m_initButton);
    buttonLayout->addWidget(m_pingButton);
    buttonLayout->addStretch();

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setMinimumHeight(260);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(infoLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_output, 1);

    connect(m_systemStatusButton, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("SYSTEM_STATUS"), tr("系统状态"));
    });
    connect(m_featureStatusButton, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("GET_FEATURE_STATUS"), tr("功能完成度"));
    });
    connect(m_initButton, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("INIT"), tr("初始化服务"));
    });
    connect(m_pingButton, &QPushButton::clicked, this, [this]() {
        sendCommand(QStringLiteral("PING"), tr("心跳检测"));
    });

    updateConnectionInfo();
}

void DashboardTab::refreshAll()
{
    sendCommand(QStringLiteral("SYSTEM_STATUS"), tr("系统状态"));
    sendCommand(QStringLiteral("GET_FEATURE_STATUS"), tr("功能完成度"));
}

void DashboardTab::handleSessionChanged(const UserSession &session)
{
    if (session.isValid()) {
        m_userLabel->setText(tr("当前用户: %1 (ID: %2, 角色: %3)")
                                 .arg(session.username)
                                 .arg(session.userId)
                                 .arg(session.role));
    } else {
        m_userLabel->setText(tr("未登录"));
    }
}

void DashboardTab::handleConnectionStateChanged(bool)
{
    updateConnectionInfo();
}

void DashboardTab::updateConnectionInfo()
{
    NetworkClient *client = m_context.networkClient();
    if (client->isConnected()) {
        m_connectionLabel->setText(tr("已连接到服务器: %1:%2")
                                        .arg(client->host())
                                        .arg(client->port()));
    } else {
        m_connectionLabel->setText(tr("未连接到服务器"));
    }
}

void DashboardTab::sendCommand(const QString &command, const QString &friendlyName)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        appendLog(friendlyName, tr("未连接到服务器"), false);
        return;
    }

    appendLog(friendlyName, tr("→ %1").arg(command), true);
    client->sendCommand(command,
                        [this, friendlyName](const QString &response) {
                            appendLog(friendlyName, response, true);
                        },
                        [this, friendlyName](const QString &error) {
                            appendLog(friendlyName, error, false);
                        });
}

void DashboardTab::appendLog(const QString &title, const QString &payload, bool success)
{
    const QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString banner = QStringLiteral("[%1][%2] %3\n%4\n")
                         .arg(time,
                              success ? tr("SUCCESS") : tr("ERROR"),
                              title,
                              payload);
    m_output->appendPlainText(banner);
}
