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
    m_notificationTable = new QTableWidget(this);
    m_notificationTable->setColumnCount(5);
    m_notificationTable->setHorizontalHeaderLabels({
        tr("ID"), tr("标题"), tr("内容"), tr("状态"), tr("时间")
    });
    m_notificationTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_notificationTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_notificationTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(150);

    m_summaryLabel = new QLabel(tr("暂无通知"), this);

    auto *buttonLayout = new QHBoxLayout;
    auto *refreshButton = new QPushButton(tr("刷新"), this);
    auto *viewDetailButton = new QPushButton(tr("查看详情"), this);
    m_markReadButton = new QPushButton(tr("标记已读"), this);
    m_markAllReadButton = new QPushButton(tr("全部已读"), this);
    auto *deleteButton = new QPushButton(tr("删除"), this);
    
    auto *unreadCheckBox = new QCheckBox(tr("只看未读"), this);
    unreadCheckBox->setChecked(false);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(viewDetailButton);
    buttonLayout->addWidget(m_markReadButton);
    buttonLayout->addWidget(m_markAllReadButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(unreadCheckBox);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_notificationTable, 1);
    layout->addWidget(new QLabel(tr("通知详情"), this));
    layout->addWidget(m_detailView);
    layout->addWidget(m_summaryLabel);

    connect(refreshButton, &QPushButton::clicked, this, [this, unreadCheckBox]() {
        bool unreadOnly = unreadCheckBox->isChecked();
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
    
    connect(unreadCheckBox, &QCheckBox::checkStateChanged, this, [refreshButton]() {
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

    sendNotificationCommand(QStringLiteral("GET_NOTIFICATIONS false"), tr("获取通知"), 
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

    // 注意: 后端可能没有实现删除通知的API,这里预留接口
    emit statusMessage(tr("删除功能暂未实现"), false);
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
    QString createTime = obj.value(QStringLiteral("create_time")).toString();
    
    QString detail = tr("=== 通知详情 ===\n\n");
    detail += tr("标题: %1\n").arg(title);
    detail += tr("类型: %1\n").arg(type);
    detail += tr("内容:\n%1\n\n").arg(content);
    detail += tr("时间: %1\n\n").arg(createTime);
    detail += tr("原始JSON:\n") + JsonUtils::pretty(QJsonDocument(obj));
    
    m_detailView->setPlainText(detail);
    
    // 如果是未读,自动标记为已读
    QString status = obj.value(QStringLiteral("is_read")).toString();
    if (status == "0" || status.toLower() == "false") {
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
        qlonglong notificationId = JsonUtils::asLongLong(obj.value(QStringLiteral("notification_id")), 0);
        QString title = obj.value(QStringLiteral("title")).toString();
        QString content = obj.value(QStringLiteral("content")).toString();
        QString isRead = obj.value(QStringLiteral("is_read")).toString();
        QString createTime = obj.value(QStringLiteral("create_time")).toString();
        
        // 截断过长的内容
        if (content.length() > 30) {
            content = content.left(30) + "...";
        }
        
        QString statusText = (isRead == "1" || isRead.toLower() == "true") ? 
            tr("已读") : tr("未读");
        
        if (statusText == tr("未读")) {
            unreadCount++;
        }

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            
            // 未读通知加粗显示
            QString isRead = obj.value(QStringLiteral("is_read")).toString();
            if (isRead == "0" || isRead.toLower() == "false") {
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
