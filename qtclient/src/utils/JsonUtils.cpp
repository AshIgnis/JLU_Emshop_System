#include "utils/JsonUtils.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QObject>
#include <QStringList>

namespace JsonUtils {

QJsonDocument parse(const QString &payload, bool *ok, QString *errorMessage)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (ok) {
            *ok = false;
        }
        if (errorMessage) {
            *errorMessage = QObject::tr("JSON解析失败: %1").arg(parseError.errorString());
        }
        return QJsonDocument();
    }

    if (ok) {
        *ok = true;
    }
    if (errorMessage) {
        errorMessage->clear();
    }
    return doc;
}

bool isSuccess(const QJsonDocument &doc)
{
    if (!doc.isObject()) {
        return false;
    }
    return doc.object().value(QStringLiteral("success")).toBool(false);
}

QString message(const QJsonDocument &doc)
{
    if (!doc.isObject()) {
        return QString();
    }
    return doc.object().value(QStringLiteral("message")).toString();
}

QJsonValue extract(const QJsonDocument &doc, const QString &path)
{
    if (!doc.isObject() && !doc.isArray()) {
        return {};
    }
    return extract(doc.isObject() ? QJsonValue(doc.object()) : QJsonValue(doc.array()), path);
}

QJsonValue extract(const QJsonValue &value, const QString &path)
{
    QStringList parts = path.split('.', Qt::SkipEmptyParts);
    return extract(value, parts);
}

QJsonValue extract(const QJsonValue &value, const QStringList &path)
{
    if (path.isEmpty()) {
        return value;
    }

    QString key = path.first();
    QStringList remaining = path.mid(1);

    if (value.isObject()) {
        QJsonValue child = value.toObject().value(key);
        return extract(child, remaining);
    }

    bool ok = false;
    int index = key.toInt(&ok);
    if (ok && value.isArray()) {
        const QJsonArray array = value.toArray();
        if (index >= 0 && index < array.size()) {
            return extract(array.at(index), remaining);
        }
    }

    return {};
}

QString pretty(const QJsonDocument &doc)
{
    if (doc.isNull()) {
        return QString();
    }
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

QString pretty(const QJsonValue &value)
{
    QJsonDocument doc;
    if (value.isObject()) {
        doc = QJsonDocument(value.toObject());
    } else if (value.isArray()) {
        doc = QJsonDocument(value.toArray());
    } else {
        QJsonObject obj;
        obj.insert(QStringLiteral("value"), value);
        doc = QJsonDocument(obj);
    }
    return pretty(doc);
}

QString asString(const QJsonValue &value)
{
    if (value.isString()) {
        return value.toString();
    }
    if (value.isBool()) {
        return value.toBool() ? QObject::tr("true") : QObject::tr("false");
    }
    if (value.isDouble()) {
        return QString::number(value.toDouble());
    }
    if (value.isNull()) {
        return QObject::tr("null");
    }
    if (value.isUndefined()) {
        return QObject::tr("undefined");
    }
    return QString::fromUtf8(QJsonDocument::fromVariant(value.toVariant()).toJson(QJsonDocument::Compact));
}

double asDouble(const QJsonValue &value, double defaultValue)
{
    if (value.isDouble()) {
        return value.toDouble();
    }
    bool ok = false;
    double converted = value.toVariant().toDouble(&ok);
    return ok ? converted : defaultValue;
}

qlonglong asLongLong(const QJsonValue &value, qlonglong defaultValue)
{
    if (value.isDouble()) {
        return static_cast<qlonglong>(value.toDouble());
    }
    bool ok = false;
    qlonglong converted = value.toVariant().toLongLong(&ok);
    return ok ? converted : defaultValue;
}

} // namespace JsonUtils
