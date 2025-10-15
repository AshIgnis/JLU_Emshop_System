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
    // 主容器样式 - 标签将使用深色文字确保清晰可见
    setStyleSheet(R"(
        QWidget {
            background-color: #f5f7fa;
        }
        QLabel {
            color: #2c3e50;
            font-weight: 500;
            font-size: 10pt;
        }
    )");

    m_connectionLabel = new QLabel(this);
    m_connectionLabel->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            padding: 16px 20px;
            border-radius: 10px;
            font-size: 11pt;
            font-weight: 500;
        }
    )");

    m_userLabel = new QLabel(this);
    m_userLabel->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f093fb, stop:1 #f5576c);
            color: white;
            padding: 16px 20px;
            border-radius: 10px;
            font-size: 11pt;
            font-weight: 500;
        }
    )");

    auto *infoLayout = new QVBoxLayout;
    infoLayout->setSpacing(12);
    infoLayout->addWidget(m_connectionLabel);
    infoLayout->addWidget(m_userLabel);

    // 按钮样式
    QString btnStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4facfe, stop:1 #00f2fe);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: 600;
            font-size: 10pt;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #43a3ee, stop:1 #00dae6);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3a92d8, stop:1 #00c2cf);
        }
    )";

    m_systemStatusButton = new QPushButton(tr("📊 系统状态"), this);
    m_systemStatusButton->setStyleSheet(btnStyle);
    
    m_featureStatusButton = new QPushButton(tr("✅ 功能完成度"), this);
    m_featureStatusButton->setStyleSheet(btnStyle);
    
    m_initButton = new QPushButton(tr("🔧 初始化 JNI 服务"), this);
    m_initButton->setStyleSheet(btnStyle);
    
    m_pingButton = new QPushButton(tr("💓 心跳检测"), this);
    m_pingButton->setStyleSheet(btnStyle);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(m_systemStatusButton);
    buttonLayout->addWidget(m_featureStatusButton);
    buttonLayout->addWidget(m_initButton);
    buttonLayout->addWidget(m_pingButton);
    buttonLayout->addStretch();

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setMinimumHeight(320);
    m_output->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #2c3e50;
            color: #ecf0f1;
            border: 2px solid #34495e;
            border-radius: 10px;
            padding: 16px;
            font-family: "Consolas", "Courier New", monospace;
            font-size: 9pt;
            line-height: 1.5;
        }
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
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
