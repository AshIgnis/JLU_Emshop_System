#ifndef NOTIFICATIONSTAB_H
#define NOTIFICATIONSTAB_H

#include <QWidget>
#include <QJsonDocument>
#include <functional>

class ApplicationContext;
class QTableWidget;
class QPlainTextEdit;
class QLabel;
class QPushButton;

class NotificationsTab : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationsTab(ApplicationContext &context, QWidget *parent = nullptr);

signals:
    void statusMessage(const QString &message, bool isSuccess);

public slots:
    void handleSessionChanged(const class UserSession &session);
    void refreshNotifications();
    void markAsRead();
    void markAllAsRead();
    void deleteNotification();
    void viewNotificationDetail();

private:
    void populateNotifications(const QJsonDocument &doc);
    QJsonObject selectedNotification() const;
    qlonglong selectedNotificationId() const;
    void updateDetailView();
    void sendNotificationCommand(const QString &command,
                                const QString &actionName,
                                std::function<void(const QJsonDocument &)> onSuccess = nullptr);

    ApplicationContext &m_context;
    QTableWidget *m_notificationTable = nullptr;
    QPlainTextEdit *m_detailView = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QPushButton *m_markReadButton = nullptr;
    QPushButton *m_markAllReadButton = nullptr;
    class QCheckBox *m_unreadCheckBox = nullptr;  // 只看未读复选框
    bool m_loggedIn = false;
};

#endif // NOTIFICATIONSTAB_H
