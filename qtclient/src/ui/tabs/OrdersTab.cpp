#include "ui/tabs/OrdersTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "ui/dialogs/PaymentDialog.h"
#include "utils/JsonUtils.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QStringList>

namespace {
QString quoteForCommand(const QString &value)
{
    QString escaped = value;
    escaped.replace("\\", "\\\\");
    escaped.replace('"', "\\\"");
    return QStringLiteral("\"%1\"").arg(escaped);
}

QJsonArray extractOrders(const QJsonDocument &doc)
{
    static const QStringList paths = {
        QStringLiteral("data.orders"),
        QStringLiteral("data.items"),
        QStringLiteral("orders"),
        QStringLiteral("items")
    };

    for (const auto &path : paths) {
        QJsonValue value = JsonUtils::extract(doc, path);
        if (value.isArray()) {
            return value.toArray();
        }
    }
    if (doc.isArray()) {
        return doc.array();
    }
    return {};
}

qlonglong readOrderId(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("order_id"),
        QStringLiteral("id"),
        QStringLiteral("orderId")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            qlonglong id = JsonUtils::asLongLong(obj.value(key), -1);
            if (id >= 0) {
                return id;
            }
        }
    }
    return -1;
}

QString readOrderStatus(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("status"),
        QStringLiteral("order_status")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toString();
        }
    }
    return QObject::tr("未知");
}

double readOrderTotal(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("total_amount"),
        QStringLiteral("total"),
        QStringLiteral("amount"),
        QStringLiteral("final_total")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return JsonUtils::asDouble(obj.value(key), 0.0);
        }
    }
    QJsonValue totalObj = obj.value(QStringLiteral("summary"));
    if (totalObj.isObject()) {
        return JsonUtils::asDouble(totalObj.toObject().value(QStringLiteral("total_amount")), 0.0);
    }
    return 0.0;
}

double readOrderPayable(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("payable_amount"),
        QStringLiteral("final_amount"),
        QStringLiteral("final_total"),
        QStringLiteral("amount_payable")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return JsonUtils::asDouble(obj.value(key), 0.0);
        }
    }
    return readOrderTotal(obj);
}

QString readOrderTime(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("create_time"),
        QStringLiteral("created_at"),
        QStringLiteral("order_time"),
        QStringLiteral("createdTime")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toString();
        }
    }
    return {};
}

} // namespace

OrdersTab::OrdersTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    m_orderTable = new QTableWidget(this);
    m_orderTable->setColumnCount(5);
    m_orderTable->setHorizontalHeaderLabels({tr("订单ID"), tr("状态"), tr("总额"), tr("应付"), tr("创建时间")});
    m_orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(180);

    m_summaryLabel = new QLabel(tr("暂无订单"), this);

    auto *buttonLayout = new QHBoxLayout;
    auto *refreshButton = new QPushButton(tr("刷新订单"), this);
    auto *detailButton = new QPushButton(tr("查看详情"), this);
    auto *payButton = new QPushButton(tr("支付订单"), this);
    auto *cancelButton = new QPushButton(tr("取消订单"), this);
    auto *refundButton = new QPushButton(tr("申请退款"), this);
    auto *trackButton = new QPushButton(tr("物流跟踪"), this);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(detailButton);
    buttonLayout->addWidget(payButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(refundButton);
    buttonLayout->addWidget(trackButton);
    buttonLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_orderTable, 1);
    layout->addWidget(new QLabel(tr("订单详情"), this));
    layout->addWidget(m_detailView);
    layout->addWidget(m_summaryLabel);

    connect(refreshButton, &QPushButton::clicked, this, &OrdersTab::refreshOrders);
    connect(detailButton, &QPushButton::clicked, this, &OrdersTab::viewOrderDetail);
    connect(payButton, &QPushButton::clicked, this, &OrdersTab::payForOrder);
    connect(cancelButton, &QPushButton::clicked, this, &OrdersTab::cancelOrder);
    connect(refundButton, &QPushButton::clicked, this, &OrdersTab::refundOrder);
    connect(trackButton, &QPushButton::clicked, this, &OrdersTab::trackOrder);
    connect(m_orderTable, &QTableWidget::itemSelectionChanged, this, &OrdersTab::updateDetailView);
}

void OrdersTab::handleSessionChanged(const UserSession &session)
{
    m_loggedIn = session.isValid();
    if (m_loggedIn) {
        refreshOrders();
    } else {
        m_orderTable->setRowCount(0);
        m_detailView->clear();
        m_summaryLabel->setText(tr("暂无订单"));
    }
}

void OrdersTab::refreshOrders()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    sendOrderCommand(QStringLiteral("VIEW_ORDERS"), tr("获取订单"), [this](const QJsonDocument &doc) {
        populateOrders(doc);
    });
}

void OrdersTab::viewOrderDetail()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    qlonglong orderId = selectedOrderId();
    if (orderId < 0) {
        emit statusMessage(tr("请选择订单"), false);
        return;
    }

    sendOrderCommand(QStringLiteral("VIEW_ORDER %1").arg(orderId), tr("查看订单"), [this](const QJsonDocument &doc) {
        m_detailView->setPlainText(JsonUtils::pretty(doc));
    });
}

void OrdersTab::payForOrder()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    qlonglong orderId = selectedOrderId();
    if (orderId < 0) {
        emit statusMessage(tr("请选择订单"), false);
        return;
    }

    double amount = selectedOrderAmount();
    PaymentDialog dialog(m_context, orderId, amount, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    PaymentRequest req = dialog.request();
    if (req.method.isEmpty() || req.amount <= 0.0) {
        emit statusMessage(tr("支付方式或金额无效"), false);
        return;
    }

    QString detail = req.accountInfo.isEmpty() ? QStringLiteral("{}") : quoteForCommand(req.accountInfo);
    QString command = QStringLiteral("PROCESS_PAYMENT %1 %2 %3 %4")
                          .arg(orderId)
                          .arg(req.method)
                          .arg(req.amount, 0, 'f', 2)
                          .arg(detail);

    sendOrderCommand(command, tr("支付订单"), [this](const QJsonDocument &doc) {
        populateOrders(doc);
    });
}

void OrdersTab::cancelOrder()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    qlonglong orderId = selectedOrderId();
    if (orderId < 0) {
        emit statusMessage(tr("请选择订单"), false);
        return;
    }
    const UserSession &session = m_context.session();
    QString command = QStringLiteral("CANCEL_ORDER %1 %2")
                          .arg(session.userId)
                          .arg(orderId);
    sendOrderCommand(command, tr("取消订单"), [this](const QJsonDocument &doc) {
        populateOrders(doc);
    });
}

void OrdersTab::refundOrder()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    qlonglong orderId = selectedOrderId();
    if (orderId < 0) {
        emit statusMessage(tr("请选择订单"), false);
        return;
    }

    bool okAmount = false;
    double amount = QInputDialog::getDouble(this, tr("退款金额"), tr("请输入退款金额"), selectedOrderAmount(), 0.01, 1000000.0, 2, &okAmount);
    if (!okAmount) {
        return;
    }
    bool okReason = false;
    QString reason = QInputDialog::getText(this, tr("退款原因"), tr("请输入退款原因"), QLineEdit::Normal, tr("用户申请退款"), &okReason);
    if (!okReason) {
        return;
    }

    QString command = QStringLiteral("REFUND_PAYMENT %1 %2 %3")
                          .arg(orderId)
                          .arg(amount, 0, 'f', 2)
                          .arg(quoteForCommand(reason));
    sendOrderCommand(command, tr("退款申请"), [this](const QJsonDocument &doc) {
        populateOrders(doc);
    });
}

void OrdersTab::trackOrder()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }
    qlonglong orderId = selectedOrderId();
    if (orderId < 0) {
        emit statusMessage(tr("请选择订单"), false);
        return;
    }

    sendOrderCommand(QStringLiteral("TRACK_ORDER %1").arg(orderId), tr("物流跟踪"), [this](const QJsonDocument &doc) {
        m_detailView->setPlainText(JsonUtils::pretty(doc));
    });
}

void OrdersTab::updateDetailView()
{
    QJsonObject obj = selectedOrder();
    if (obj.isEmpty()) {
        m_detailView->clear();
        return;
    }
    m_detailView->setPlainText(JsonUtils::pretty(QJsonDocument(obj)));
}

void OrdersTab::populateOrders(const QJsonDocument &doc)
{
    QJsonArray orders = extractOrders(doc);
    m_orderTable->setRowCount(orders.size());
    double totalAmount = 0.0;

    for (int row = 0; row < orders.size(); ++row) {
        const QJsonObject obj = orders.at(row).toObject();
        const qlonglong orderId = readOrderId(obj);
        const QString status = readOrderStatus(obj);
        const double total = readOrderTotal(obj);
        const double payable = readOrderPayable(obj);
        const QString created = readOrderTime(obj);

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            m_orderTable->setItem(row, column, item);
        };

        setItem(0, QString::number(orderId));
        setItem(1, status);
        setItem(2, QString::number(total, 'f', 2));
        setItem(3, QString::number(payable, 'f', 2));
        setItem(4, created);

        totalAmount += payable;
    }

    if (!orders.isEmpty()) {
        m_orderTable->selectRow(0);
    }

    m_summaryLabel->setText(tr("订单数: %1 | 待支付总额: %2 元")
                                .arg(orders.size())
                                .arg(totalAmount, 0, 'f', 2));

    m_detailView->setPlainText(JsonUtils::pretty(doc));
}

QJsonObject OrdersTab::selectedOrder() const
{
    const int row = m_orderTable->currentRow();
    if (row < 0) {
        return {};
    }
    QTableWidgetItem *item = m_orderTable->item(row, 0);
    if (!item) {
        return {};
    }
    const QString payload = item->data(Qt::UserRole).toString();
    return QJsonDocument::fromJson(payload.toUtf8()).object();
}

qlonglong OrdersTab::selectedOrderId() const
{
    return readOrderId(selectedOrder());
}

double OrdersTab::selectedOrderAmount() const
{
    return readOrderPayable(selectedOrder());
}

void OrdersTab::sendOrderCommand(const QString &command,
                                 const QString &actionName,
                                 std::function<void(const QJsonDocument &)> onSuccess)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        emit statusMessage(tr("未连接服务器"), false);
        return;
    }

    QPointer<OrdersTab> guard(this);
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
