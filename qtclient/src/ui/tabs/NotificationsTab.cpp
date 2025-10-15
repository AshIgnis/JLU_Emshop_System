#include "ui/tabs/NotificationsTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "utils/JsonUtils.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QTimer>

NotificationsTab::NotificationsTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    // 设置整体样式，确保标签文字清晰可见
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
    
    m_notificationTable = new QTableWidget(this);
    m_notificationTable->setColumnCount(5);
    m_notificationTable->setHorizontalHeaderLabels({
        tr("ID"), tr("标题"), tr("内容"), tr("状态"), tr("时间")
    });
    m_notificationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_notificationTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_notificationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_notificationTable->setStyleSheet(R"(
        QTableWidget {
            background-color: white;
            color: #2c3e50;
            alternate-background-color: #f8f9fa;
            gridline-color: #ecf0f1;
            border: 2px solid #dfe6e9;
            border-radius: 10px;
            selection-background-color: #3498db;
            selection-color: white;
        }
        QTableWidget::item {
            color: #2c3e50;
            padding: 10px;
            border: none;
        }
        QTableWidget::item:selected {
            background-color: #3498db;
            color: white;
        }
        QTableWidget::item:hover:!selected {
            background-color: #ecf0f1;
            color: #2c3e50;
        }
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #34495e, stop:1 #2c3e50);
            color: white;
            padding: 12px;
            border: none;
            font-weight: 600;
            font-size: 10pt;
        }
    )");

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(150);
    m_detailView->setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #ffffff;
            color: #2c3e50;
            border: 2px solid #e0e6ed;
            border-radius: 10px;
            padding: 14px;
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
            font-size: 11pt;
            font-weight: 500;
            line-height: 1.6;
        }
    )");

    m_summaryLabel = new QLabel(tr("暂无通知"), this);

    // 按钮样式 - 确保文字清晰可见
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4facfe, stop:1 #00f2fe);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: 600;
            font-size: 10pt;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #43a3ee, stop:1 #00dae6);
        }
    )";
    
    auto *buttonLayout = new QHBoxLayout;
    auto *refreshButton = new QPushButton(tr("刷新"), this);
    auto *viewDetailButton = new QPushButton(tr("查看详情"), this);
    m_markReadButton = new QPushButton(tr("标记已读"), this);
    m_markAllReadButton = new QPushButton(tr("全部已读"), this);
    auto *deleteButton = new QPushButton(tr("删除"), this);
    
    refreshButton->setStyleSheet(buttonStyle);
    viewDetailButton->setStyleSheet(buttonStyle);
    m_markReadButton->setStyleSheet(buttonStyle);
    m_markAllReadButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(buttonStyle);
    
    m_unreadCheckBox = new QCheckBox(tr("只看未读"), this);
    m_unreadCheckBox->setChecked(false);
    m_unreadCheckBox->setStyleSheet(R"(
        QCheckBox {
            color: #2c3e50;
            font-size: 10pt;
            font-weight: 500;
            spacing: 6px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #3498db;
            border-radius: 4px;
            background-color: #ffffff;
        }
        QCheckBox::indicator:hover {
            border-color: #667eea;
            background-color: #f0f4ff;
        }
        QCheckBox::indicator:checked {
            background-color: #3498db;
            border-color: #3498db;
        }
    )");

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(viewDetailButton);
    buttonLayout->addWidget(m_markReadButton);
    buttonLayout->addWidget(m_markAllReadButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_unreadCheckBox);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_notificationTable, 1);
    layout->addWidget(new QLabel(tr("通知详情"), this));
    layout->addWidget(m_detailView);
    layout->addWidget(m_summaryLabel);

    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        bool unreadOnly = m_unreadCheckBox->isChecked();
        QString command = unreadOnly ? 
            QStringLiteral("GET_NOTIFICATIONS true") : 
            QStringLiteral("GET_NOTIFICATIONS false");
        
        sendNotificationCommand(command, tr("获取通知"), [this](const QJsonDocument &doc) {
            populateNotifications(doc);
        });
    });
    
    connect(viewDetailButton, &QPushButton::clicked, this, &NotificationsTab::viewNotificationDetail);
    connect(m_markReadButton, &QPushButton::clicked, this, &NotificationsTab::markAsRead);
    connect(m_markAllReadButton, &QPushButton::clicked, this, &NotificationsTab::markAllAsRead);
    connect(deleteButton, &QPushButton::clicked, this, &NotificationsTab::deleteNotification);
    connect(m_notificationTable, &QTableWidget::itemSelectionChanged, this, &NotificationsTab::updateDetailView);
    
    connect(m_unreadCheckBox, &QCheckBox::checkStateChanged, this, [refreshButton]() {
        refreshButton->click();
    });
}

void NotificationsTab::handleSessionChanged(const UserSession &session)
{
    m_loggedIn = session.isValid();
    if (m_loggedIn) {
        refreshNotifications();
    } else {
        m_notificationTable->setRowCount(0);
        m_detailView->clear();
        m_summaryLabel->setText(tr("暂无通知"));
    }
}

void NotificationsTab::refreshNotifications()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    // 根据复选框状态决定获取所有通知还是仅未读通知
    bool unreadOnly = m_unreadCheckBox ? m_unreadCheckBox->isChecked() : false;
    QString command = unreadOnly ? 
        QStringLiteral("GET_NOTIFICATIONS true") : 
        QStringLiteral("GET_NOTIFICATIONS false");
    
    sendNotificationCommand(command, tr("获取通知"), 
        [this](const QJsonDocument &doc) {
            populateNotifications(doc);
        });
}

void NotificationsTab::markAsRead()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    
    qlonglong notificationId = selectedNotificationId();
    if (notificationId < 0) {
        emit statusMessage(tr("请选择通知"), false);
        return;
    }

    QString command = QStringLiteral("MARK_NOTIFICATION_READ %1").arg(notificationId);
    sendNotificationCommand(command, tr("标记已读"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshNotifications();
    });
}

void NotificationsTab::markAllAsRead()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    // 获取所有未读通知的ID并逐个标记
    int rowCount = m_notificationTable->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *statusItem = m_notificationTable->item(row, 3);
        if (statusItem && statusItem->text() == tr("未读")) {
            QTableWidgetItem *idItem = m_notificationTable->item(row, 0);
            if (idItem) {
                qlonglong notificationId = idItem->text().toLongLong();
                QString command = QStringLiteral("MARK_NOTIFICATION_READ %1").arg(notificationId);
                sendNotificationCommand(command, tr("标记已读"), nullptr);
            }
        }
    }
    
    // 延迟刷新以等待所有标记完成
    QTimer::singleShot(500, this, &NotificationsTab::refreshNotifications);
}

void NotificationsTab::deleteNotification()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    
    qlonglong notificationId = selectedNotificationId();
    if (notificationId < 0) {
        emit statusMessage(tr("请选择通知"), false);
        return;
    }

    QString command = QStringLiteral("DELETE_NOTIFICATION %1").arg(notificationId);
    sendNotificationCommand(command, tr("删除通知"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshNotifications();
    });
}

void NotificationsTab::viewNotificationDetail()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    
    QJsonObject obj = selectedNotification();
    if (obj.isEmpty()) {
        emit statusMessage(tr("请选择通知"), false);
        return;
    }

    // 显示详情
    QString title = obj.value(QStringLiteral("title")).toString();
    QString content = obj.value(QStringLiteral("content")).toString();
    QString type = obj.value(QStringLiteral("type")).toString();
    QString createTime = obj.value(QStringLiteral("created_at")).toString();
    const bool isRead = JsonUtils::asBool(obj.value(QStringLiteral("is_read")));
    
    QString detail = tr("=== 通知详情 ===\n\n");
    detail += tr("标题: %1\n").arg(title);
    detail += tr("类型: %1\n").arg(type);
    detail += tr("内容:\n%1\n\n").arg(content);
    detail += tr("时间: %1\n\n").arg(createTime);
    detail += tr("原始JSON:\n") + JsonUtils::pretty(QJsonDocument(obj));
    
    m_detailView->setPlainText(detail);
    
    // 如果是未读,自动标记为已读
    if (!isRead) {
        markAsRead();
    }
}

void NotificationsTab::updateDetailView()
{
    QJsonObject obj = selectedNotification();
    if (obj.isEmpty()) {
        m_detailView->clear();
        return;
    }
    
    QString title = obj.value(QStringLiteral("title")).toString();
    QString content = obj.value(QStringLiteral("content")).toString();
    
    m_detailView->setPlainText(tr("标题: %1\n内容: %2").arg(title, content));
}

void NotificationsTab::populateNotifications(const QJsonDocument &doc)
{
    QJsonArray notifications = JsonUtils::extract(doc, QStringLiteral("data")).toArray();
    m_notificationTable->setRowCount(notifications.size());
    
    int unreadCount = 0;

    for (int row = 0; row < notifications.size(); ++row) {
        const QJsonObject obj = notifications.at(row).toObject();
        
        // 调试: 打印原始JSON对象
        qDebug() << "Notification JSON:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        
        qlonglong notificationId = JsonUtils::asLongLong(obj.value(QStringLiteral("notification_id")), 0);
        QString title = obj.value(QStringLiteral("title")).toString();
        QString content = obj.value(QStringLiteral("content")).toString();
        
        // 获取is_read字段,打印调试信息
        QJsonValue isReadValue = obj.value(QStringLiteral("is_read"));
        qDebug() << "notification_id:" << notificationId 
                 << "is_read type:" << isReadValue.type() 
                 << "value:" << isReadValue;

        const bool readFlag = JsonUtils::asBool(isReadValue);
        QString createTime = obj.value(QStringLiteral("created_at")).toString();
        
        // 截断过长的内容
        if (content.length() > 30) {
            content = content.left(30) + "...";
        }
        
        QString statusText = readFlag ? tr("已读") : tr("未读");

        if (!readFlag) {
            unreadCount++;
        }

        auto setItem = [this, row, obj, readFlag](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            
            // 未读通知加粗显示
            if (!readFlag) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
            
            m_notificationTable->setItem(row, column, item);
        };

        setItem(0, QString::number(notificationId));
        setItem(1, title);
        setItem(2, content);
        setItem(3, statusText);
        setItem(4, createTime);
    }

    if (!notifications.isEmpty()) {
        m_notificationTable->selectRow(0);
    }

    m_summaryLabel->setText(tr("通知总数: %1 | 未读: %2")
                                .arg(notifications.size())
                                .arg(unreadCount));

    emit statusMessage(tr("已加载 %1 条通知").arg(notifications.size()), true);
}

QJsonObject NotificationsTab::selectedNotification() const
{
    const int row = m_notificationTable->currentRow();
    if (row < 0) {
        return {};
    }
    QTableWidgetItem *item = m_notificationTable->item(row, 0);
    if (!item) {
        return {};
    }
    const QString payload = item->data(Qt::UserRole).toString();
    return QJsonDocument::fromJson(payload.toUtf8()).object();
}

qlonglong NotificationsTab::selectedNotificationId() const
{
    QJsonObject obj = selectedNotification();
    return JsonUtils::asLongLong(obj.value(QStringLiteral("notification_id")), -1);
}

void NotificationsTab::sendNotificationCommand(const QString &command,
                                              const QString &actionName,
                                              std::function<void(const QJsonDocument &)> onSuccess)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        emit statusMessage(tr("未连接服务器"), false);
        return;
    }

    QPointer<NotificationsTab> guard(this);
    client->sendCommand(command,
        [this, guard, actionName, onSuccess](const QString &response) {
            if (!guard) {
                return;
            }
            bool ok = false;
            QString error;
            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
            if (!ok) {
                emit statusMessage(tr("%1失败: %2").arg(actionName, error), false);
                return;
            }
            if (!JsonUtils::isSuccess(doc)) {
                QString msg = JsonUtils::message(doc);
                if (msg.isEmpty()) msg = response;
                emit statusMessage(tr("%1失败: %2").arg(actionName, msg), false);
                return;
            }
            emit statusMessage(tr("%1成功").arg(actionName), true);
            if (onSuccess) {
                onSuccess(doc);
            }
        },
        [this, guard, actionName](const QString &error) {
            if (!guard) {
                return;
            }
            emit statusMessage(tr("%1失败: %2").arg(actionName, error), false);
        });
}
