#pragma once

#include <QJsonDocument>
#include <QJsonValue>
#include <QString>

namespace JsonUtils {

QJsonDocument parse(const QString &payload, bool *ok = nullptr, QString *errorMessage = nullptr);

bool isSuccess(const QJsonDocument &doc);
QString message(const QJsonDocument &doc);

QJsonValue extract(const QJsonDocument &doc, const QString &path);
QJsonValue extract(const QJsonValue &value, const QStringList &path);
QJsonValue extract(const QJsonValue &value, const QString &path);

QString pretty(const QJsonDocument &doc);
QString pretty(const QJsonValue &value);

QString asString(const QJsonValue &value);

double asDouble(const QJsonValue &value, double defaultValue = 0.0);
qlonglong asLongLong(const QJsonValue &value, qlonglong defaultValue = 0);
bool asBool(const QJsonValue &value, bool defaultValue = false);

} // namespace JsonUtils
