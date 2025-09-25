#pragma once

#include <QString>

struct UserSession {
    qint64 userId = -1;
    QString username;
    QString role = QStringLiteral("user");
    bool loggedIn = false;
    QString rawLoginResponse;

    bool isValid() const { return loggedIn && userId >= 0; }
    bool isAdmin() const { return role.compare(QStringLiteral("admin"), Qt::CaseInsensitive) == 0; }

    void reset()
    {
        userId = -1;
        username.clear();
        role = QStringLiteral("user");
        loggedIn = false;
        rawLoginResponse.clear();
    }
};
