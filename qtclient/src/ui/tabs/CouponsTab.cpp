#include "ui/tabs/CouponsTab.h"

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
#include <QTabWidget>

CouponsTab::CouponsTab(ApplicationContext &context, QWidget *parent)
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
    
    m_couponTable = new QTableWidget(this);
    m_couponTable->setColumnCount(6);
    m_couponTable->setHorizontalHeaderLabels({
        tr("ID"), tr("名称"), tr("类型"), tr("折扣"), tr("状态"), tr("过期时间")
    });
    m_couponTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_couponTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_couponTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_couponTable->setStyleSheet(R"(
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

    m_summaryLabel = new QLabel(tr("暂无优惠券"), this);

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
    auto *refreshButton = new QPushButton(tr("我的优惠券"), this);
    auto *templatesButton = new QPushButton(tr("优惠券模板"), this);
    auto *viewDetailButton = new QPushButton(tr("查看详情"), this);
    
    refreshButton->setStyleSheet(buttonStyle);
    templatesButton->setStyleSheet(buttonStyle);
    viewDetailButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(templatesButton);
    buttonLayout->addWidget(viewDetailButton);
    buttonLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_couponTable, 1);
    layout->addWidget(new QLabel(tr("优惠券详情"), this));
    layout->addWidget(m_detailView);
    layout->addWidget(m_summaryLabel);

    connect(refreshButton, &QPushButton::clicked, this, &CouponsTab::refreshCoupons);
    connect(templatesButton, &QPushButton::clicked, this, &CouponsTab::viewCouponTemplates);
    connect(viewDetailButton, &QPushButton::clicked, this, &CouponsTab::viewCouponDetail);
    connect(m_couponTable, &QTableWidget::itemSelectionChanged, this, &CouponsTab::updateDetailView);
}

void CouponsTab::handleSessionChanged(const UserSession &session)
{
    bool wasLoggedIn = m_loggedIn;
    m_loggedIn = session.isValid();
    
    // 只在登录状态真正改变时才处理
    if (m_loggedIn == wasLoggedIn) {
        return;
    }
    
    if (m_loggedIn) {
        // 登录后不自动刷新,等待用户手动点击
        m_summaryLabel->setText(tr("已登录 - 点击按钮查看优惠券"));
    } else {
        m_couponTable->setRowCount(0);
        m_detailView->clear();
        m_summaryLabel->setText(tr("暂无优惠券"));
        m_showingTemplates = false;
    }
}

void CouponsTab::refreshCoupons()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    m_showingTemplates = false;
    // 调用后端获取当前会话用户的优惠券（服务器实现为 GET_USER_COUPONS）
    sendCouponCommand(QStringLiteral("GET_USER_COUPONS"), tr("获取我的优惠券"),
        [this](const QJsonDocument &doc) {
            // 服务器返回的优惠券可能位于 data、data.coupons、data.list 等位置
            populateCoupons(doc);
        });
}

void CouponsTab::viewCouponTemplates()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    m_showingTemplates = true;
    sendCouponCommand(QStringLiteral("GET_COUPON_TEMPLATES"), tr("获取优惠券模板"), 
        [this](const QJsonDocument &doc) {
            populateCouponTemplates(doc);
        });
}

void CouponsTab::viewCouponDetail()
{
    QJsonObject obj = selectedCoupon();
    if (obj.isEmpty()) {
        emit statusMessage(tr("请选择优惠券"), false);
        return;
    }

    QString detail;
    if (m_showingTemplates) {
        // 显示模板详情
        qlonglong templateId = JsonUtils::asLongLong(obj.value(QStringLiteral("template_id")), 0);
        QString name = obj.value(QStringLiteral("template_name")).toString();
        QString type = obj.value(QStringLiteral("discount_type")).toString();
        double discountValue = JsonUtils::asDouble(obj.value(QStringLiteral("discount_value")), 0.0);
        double minAmount = JsonUtils::asDouble(obj.value(QStringLiteral("min_order_amount")), 0.0);
        double maxDiscount = JsonUtils::asDouble(obj.value(QStringLiteral("max_discount_amount")), 0.0);
        QString description = obj.value(QStringLiteral("description")).toString();
        
        detail = tr("=== 优惠券模板详情 ===\n\n");
        detail += tr("模板ID: %1\n").arg(templateId);
        detail += tr("名称: %1\n").arg(name);
        detail += tr("类型: %1\n").arg(type == "percentage" ? tr("百分比") : tr("固定金额"));
        
        if (type == "percentage") {
            detail += tr("折扣: %1%\n").arg(discountValue);
        } else {
            detail += tr("优惠: ¥%1\n").arg(discountValue, 0, 'f', 2);
        }
        
        detail += tr("最低消费: ¥%1\n").arg(minAmount, 0, 'f', 2);
        if (maxDiscount > 0) {
            detail += tr("最高优惠: ¥%1\n").arg(maxDiscount, 0, 'f', 2);
        }
        detail += tr("\n描述: %1\n\n").arg(description);
        detail += tr("原始JSON:\n") + JsonUtils::pretty(QJsonDocument(obj));
        
    } else {
        // 显示用户优惠券详情
        detail = JsonUtils::pretty(QJsonDocument(obj));
    }
    
    m_detailView->setPlainText(detail);
}

void CouponsTab::updateDetailView()
{
    QJsonObject obj = selectedCoupon();
    if (obj.isEmpty()) {
        m_detailView->clear();
        return;
    }
    
    if (m_showingTemplates) {
        QString name = obj.value(QStringLiteral("template_name")).toString();
        QString description = obj.value(QStringLiteral("description")).toString();
        m_detailView->setPlainText(tr("名称: %1\n描述: %2").arg(name, description));
    } else {
        m_detailView->setPlainText(JsonUtils::pretty(QJsonDocument(obj)));
    }
}

void CouponsTab::populateCoupons(const QJsonDocument &doc)
{
    // 尝试多个可能的路径查找优惠券数组
    static const QStringList couponPaths = {
        QStringLiteral("data.user_coupons"),
        QStringLiteral("data.userCoupons"),
        QStringLiteral("data.coupons"),
        QStringLiteral("data.list"),
        QStringLiteral("data.items"),
        QStringLiteral("data"),
        QStringLiteral("user_coupons"),
        QStringLiteral("coupons")
    };

    QJsonArray coupons;
    for (const QString &path : couponPaths) {
        QJsonValue v = JsonUtils::extract(doc, path);
        if (v.isArray()) {
            coupons = v.toArray();
            break;
        }
    }

    m_couponTable->setRowCount(coupons.size());

    for (int row = 0; row < coupons.size(); ++row) {
        const QJsonObject obj = coupons.at(row).toObject();
        
        // 尝试多个可能的字段名
        auto pickLongLong = [&obj](std::initializer_list<QString> keys, qlonglong defaultVal = 0) -> qlonglong {
            for (const QString &key : keys) {
                if (obj.contains(key)) {
                    return JsonUtils::asLongLong(obj.value(key), defaultVal);
                }
            }
            return defaultVal;
        };
        
        auto pickString = [&obj](std::initializer_list<QString> keys, const QString &defaultVal = QString()) -> QString {
            for (const QString &key : keys) {
                if (obj.contains(key)) {
                    QString val = obj.value(key).toString();
                    if (!val.isEmpty()) return val;
                }
            }
            return defaultVal;
        };
        
        auto pickDouble = [&obj](std::initializer_list<QString> keys, double defaultVal = 0.0) -> double {
            for (const QString &key : keys) {
                if (obj.contains(key)) {
                    return JsonUtils::asDouble(obj.value(key), defaultVal);
                }
            }
            return defaultVal;
        };

        qlonglong couponId = pickLongLong({QStringLiteral("coupon_id"), QStringLiteral("id"), QStringLiteral("couponId")});
        QString name = pickString({QStringLiteral("template_name"), QStringLiteral("name"), QStringLiteral("title"), QStringLiteral("coupon_name")});
        QString type = pickString({QStringLiteral("discount_type"), QStringLiteral("type"), QStringLiteral("discountType")});
        double discountValue = pickDouble({QStringLiteral("discount_value"), QStringLiteral("value"), QStringLiteral("discountValue")});
        QString status = pickString({QStringLiteral("status"), QStringLiteral("state")});
        QString expireTime = pickString({QStringLiteral("expire_time"), QStringLiteral("expireTime"), QStringLiteral("valid_until"), QStringLiteral("validUntil")});

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            m_couponTable->setItem(row, column, item);
        };

        QString typeText = (type == "percentage") ? tr("百分比") : tr("固定金额");
        QString discountText = (type == "percentage") ? 
            QString("%1%").arg(discountValue) : 
            QString("¥%1").arg(discountValue, 0, 'f', 2);
        
        QString statusText;
        if (status == "available") statusText = tr("可用");
        else if (status == "used") statusText = tr("已使用");
        else if (status == "expired") statusText = tr("已过期");
        else statusText = status;

        setItem(0, QString::number(couponId));
        setItem(1, name);
        setItem(2, typeText);
        setItem(3, discountText);
        setItem(4, statusText);
        setItem(5, expireTime);
    }

    if (!coupons.isEmpty()) {
        m_couponTable->selectRow(0);
    }

    m_summaryLabel->setText(tr("优惠券总数: %1").arg(coupons.size()));
    
    // 如果没有数据，显示原始JSON以便调试
    if (coupons.isEmpty()) {
        m_detailView->setPlainText(tr("未找到优惠券数据\n\n原始响应:\n") + JsonUtils::pretty(doc));
    }
}

void CouponsTab::populateCouponTemplates(const QJsonDocument &doc)
{
    QJsonArray templates = JsonUtils::extract(doc, QStringLiteral("data")).toArray();
    m_couponTable->setRowCount(templates.size());

    int activeCount = 0;

    for (int row = 0; row < templates.size(); ++row) {
        const QJsonObject obj = templates.at(row).toObject();
        qlonglong templateId = JsonUtils::asLongLong(obj.value(QStringLiteral("template_id")), 0);
        QString name = obj.value(QStringLiteral("template_name")).toString();
        QString type = obj.value(QStringLiteral("discount_type")).toString();
        double discountValue = JsonUtils::asDouble(obj.value(QStringLiteral("discount_value")), 0.0);
        QString status = obj.value(QStringLiteral("status")).toString();
        QString validUntil = obj.value(QStringLiteral("valid_until")).toString();

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            m_couponTable->setItem(row, column, item);
        };

        QString typeText = (type == "percentage") ? tr("百分比折扣") : tr("固定金额减免");
        QString discountText = (type == "percentage") ? 
            QString("%1% OFF").arg(discountValue) : 
            QString("减¥%1").arg(discountValue, 0, 'f', 2);
        
        QString statusText;
        if (status == "active") {
            statusText = tr("活跃");
            activeCount++;
        } else if (status == "inactive") {
            statusText = tr("停用");
        } else {
            statusText = status;
        }

        setItem(0, QString::number(templateId));
        setItem(1, name);
        setItem(2, typeText);
        setItem(3, discountText);
        setItem(4, statusText);
        setItem(5, validUntil.isEmpty() ? tr("无限期") : validUntil);
    }

    if (!templates.isEmpty()) {
        m_couponTable->selectRow(0);
    }

    m_summaryLabel->setText(tr("优惠券模板总数: %1 | 活跃: %2")
                                .arg(templates.size())
                                .arg(activeCount));

    emit statusMessage(tr("已加载 %1 个优惠券模板").arg(templates.size()), true);
}

QJsonObject CouponsTab::selectedCoupon() const
{
    const int row = m_couponTable->currentRow();
    if (row < 0) {
        return {};
    }
    QTableWidgetItem *item = m_couponTable->item(row, 0);
    if (!item) {
        return {};
    }
    const QString payload = item->data(Qt::UserRole).toString();
    return QJsonDocument::fromJson(payload.toUtf8()).object();
}

qlonglong CouponsTab::selectedCouponId() const
{
    QJsonObject obj = selectedCoupon();
    if (m_showingTemplates) {
        return JsonUtils::asLongLong(obj.value(QStringLiteral("template_id")), -1);
    } else {
        return JsonUtils::asLongLong(obj.value(QStringLiteral("coupon_id")), -1);
    }
}

void CouponsTab::sendCouponCommand(const QString &command,
                                  const QString &actionName,
                                  std::function<void(const QJsonDocument &)> onSuccess)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        emit statusMessage(tr("未连接服务器"), false);
        return;
    }

    QPointer<CouponsTab> guard(this);
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
