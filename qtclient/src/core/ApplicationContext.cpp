#include "core/ApplicationContext.h"

#include "network/NetworkClient.h"

ApplicationContext::ApplicationContext(QObject *parent)
    : QObject(parent)
    , m_networkClient(new NetworkClient(this))
{
}

ApplicationContext::~ApplicationContext() = default;

NetworkClient *ApplicationContext::networkClient() const
{
    return m_networkClient;
}

UserSession &ApplicationContext::session()
{
    return m_session;
}

const UserSession &ApplicationContext::session() const
{
    return m_session;
}

void ApplicationContext::updateSession(const UserSession &session)
{
    m_session = session;
    emit sessionChanged(m_session);
}

void ApplicationContext::clearSession()
{
    m_session.reset();
    emit sessionChanged(m_session);
}
