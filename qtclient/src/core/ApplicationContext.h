#pragma once

#include <QObject>
#include "core/UserSession.h"

class NetworkClient;

/**
 * @brief ApplicationContext 持有全局共享的网络客户端与会话信息。
 */
class ApplicationContext : public QObject {
    Q_OBJECT
public:
    explicit ApplicationContext(QObject *parent = nullptr);
    ~ApplicationContext() override;

    NetworkClient *networkClient() const;
    UserSession &session();
    const UserSession &session() const;

    void updateSession(const UserSession &session);
    void clearSession();

signals:
    void sessionChanged(const UserSession &session);

private:
    NetworkClient *m_networkClient;
    UserSession m_session;
};
