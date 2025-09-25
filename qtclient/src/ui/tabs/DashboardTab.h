#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QPlainTextEdit;

class ApplicationContext;
struct UserSession;

class DashboardTab : public QWidget {
    Q_OBJECT
public:
    explicit DashboardTab(ApplicationContext &context, QWidget *parent = nullptr);

public slots:
    void refreshAll();
    void handleSessionChanged(const UserSession &session);
    void handleConnectionStateChanged(bool connected);

private:
    void sendCommand(const QString &command, const QString &friendlyName);
    void appendLog(const QString &title, const QString &payload, bool success);
    void updateConnectionInfo();

    ApplicationContext &m_context;
    QLabel *m_connectionLabel;
    QLabel *m_userLabel;
    QPlainTextEdit *m_output;
    QPushButton *m_systemStatusButton;
    QPushButton *m_featureStatusButton;
    QPushButton *m_initButton;
    QPushButton *m_pingButton;
};
