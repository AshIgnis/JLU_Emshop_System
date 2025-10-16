#include "ui/tabs/OrdersTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "ui/dialogs/PaymentDialog.h"
#include "utils/JsonUtils.h"
#include "utils/StyleHelper.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
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
    
    m_orderTable = new QTableWidget(this);
    m_orderTable->setColumnCount(5);
    m_orderTable->setHorizontalHeaderLabels({tr("订单ID"), tr("状态"), tr("总额"), tr("应付"), tr("创建时间")});
    m_orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_orderTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_orderTable->setStyleSheet(R"(
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
    m_detailView->setMinimumHeight(180);
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

    m_summaryLabel = new QLabel(tr("暂无订单"), this);

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
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3a92d8, stop:1 #00c2cf);
        }
    )";
    
    auto *buttonLayout = new QHBoxLayout;
    auto *refreshButton = new QPushButton(tr("刷新订单"), this);
    auto *detailButton = new QPushButton(tr("查看详情"), this);
    auto *payButton = new QPushButton(tr("支付订单"), this);
    auto *cancelButton = new QPushButton(tr("取消订单"), this);
    auto *refundButton = new QPushButton(tr("申请退款"), this);
    auto *viewRefundsButton = new QPushButton(tr("我的退款"), this);  // v1.1.0: 新增
    auto *trackButton = new QPushButton(tr("物流跟踪"), this);
    auto *deleteButton = new QPushButton(tr("删除订单"), this);
    
    refreshButton->setStyleSheet(buttonStyle);
    detailButton->setStyleSheet(buttonStyle);
    payButton->setStyleSheet(buttonStyle);
    cancelButton->setStyleSheet(buttonStyle);
    refundButton->setStyleSheet(buttonStyle);
    viewRefundsButton->setStyleSheet(buttonStyle);
    trackButton->setStyleSheet(buttonStyle);
    deleteButton->setStyleSheet(buttonStyle);

    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(detailButton);
    buttonLayout->addWidget(payButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(refundButton);
    buttonLayout->addWidget(viewRefundsButton);  // v1.1.0: 新增
    buttonLayout->addWidget(trackButton);
    buttonLayout->addWidget(deleteButton);
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
    connect(viewRefundsButton, &QPushButton::clicked, this, &OrdersTab::viewMyRefunds);  // v1.1.0: 新增
    connect(trackButton, &QPushButton::clicked, this, &OrdersTab::trackOrder);
    connect(deleteButton, &QPushButton::clicked, this, &OrdersTab::deleteOrder);
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
        // 如果服务端附加了友好的纯文本摘要(plain_text)，优先展示，随后附上原始JSON以便排查
        QJsonValue plainTextVal = JsonUtils::extract(doc, QStringLiteral("plain_text"));
        if (plainTextVal.isString()) {
            const QString friendly = plainTextVal.toString();
            if (!friendly.trimmed().isEmpty()) {
                m_detailView->setPlainText(friendly + "\n\n" + JsonUtils::pretty(doc));
                return;
            }
        }
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
        Q_UNUSED(doc);
        // 服务器返回的是单个订单处理结果，不包含订单列表
        // 为了避免将表清空，这里改为主动刷新订单列表
        refreshOrders();
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
        Q_UNUSED(doc);
        // 返回为单个订单对象，改为刷新列表以获取最新状态
        refreshOrders();
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

    QInputDialog dialog(this);
    dialog.setWindowTitle(tr("申请退款"));
    dialog.setLabelText(tr("请输入退款原因(必填)"));
    dialog.setTextValue(tr("商品质量问题"));
    dialog.setOkButtonText(tr("提交申请"));
    dialog.setCancelButtonText(tr("取消"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setModal(true);
    dialog.resize(440, 200);

    dialog.setStyleSheet(R"(
        QInputDialog {
            background-color: #f8fafc;
            color: #1f2933;
            border-radius: 12px;
        }
        QLabel {
            color: #1f2933;
            font-size: 11pt;
            font-weight: 600;
        }
        QLineEdit {
            padding: 10px 14px;
            font-size: 10pt;
            border: 2px solid #667eea;
            border-radius: 8px;
            background-color: #ffffff;
        }
        QLineEdit:focus {
            border-color: #4c51bf;
            background-color: #eef2ff;
        }
    )");

    if (auto *lineEdit = dialog.findChild<QLineEdit *>()) {
        lineEdit->setPlaceholderText(tr("请填写退款原因，例如商品损坏或发货错误"));
        lineEdit->selectAll();
    }

    if (auto *buttonBox = dialog.findChild<QDialogButtonBox *>()) {
        if (auto *okButton = buttonBox->button(QDialogButtonBox::Ok)) {
            okButton->setStyleSheet(StyleHelper::primaryButtonStyle());
        }
        if (auto *cancelButton = buttonBox->button(QDialogButtonBox::Cancel)) {
            cancelButton->setStyleSheet(StyleHelper::secondaryButtonStyle());
        }
    }

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    QString reason = dialog.textValue().trimmed();
    if (reason.isEmpty()) {
        return;
    }

    // v1.1.0: 使用新的 REQUEST_REFUND 命令格式
    QString command = QStringLiteral("REQUEST_REFUND %1 %2")
                          .arg(orderId)
                          .arg(quoteForCommand(reason));
    
    sendOrderCommand(command, tr("退款申请"), [this](const QJsonDocument &doc) {
        // 检查是否成功
        int code = JsonUtils::extract(doc, QStringLiteral("code")).toInt(-1);
        if (code == 200) {
            emit statusMessage(tr("退款申请已提交,等待管理员审批"), true);
        }
        // 刷新订单列表以显示最新状态
        refreshOrders();
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

// v1.1.0: 查看我的退款申请
void OrdersTab::viewMyRefunds()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    sendOrderCommand(QStringLiteral("GET_USER_REFUND_REQUESTS"), tr("查看退款申请"), [this](const QJsonDocument &doc) {
        // 显示退款列表
        QString refundText;
        QJsonArray refunds = JsonUtils::extract(doc, QStringLiteral("data")).toArray();
        
        if (refunds.isEmpty()) {
            refundText = tr("暂无退款申请");
        } else {
            refundText = tr("=== 我的退款申请 (%1条) ===\n\n").arg(refunds.size());
            
            for (int i = 0; i < refunds.size(); ++i) {
                QJsonObject refund = refunds.at(i).toObject();
                qlonglong refundId = JsonUtils::asLongLong(refund.value(QStringLiteral("refund_id")), 0);
                qlonglong orderId = JsonUtils::asLongLong(refund.value(QStringLiteral("order_id")), 0);
                QString status = refund.value(QStringLiteral("status")).toString();
                double amount = JsonUtils::asDouble(refund.value(QStringLiteral("refund_amount")), 0.0);
                QString reason = refund.value(QStringLiteral("reason")).toString();
                QString adminReply = refund.value(QStringLiteral("admin_reply")).toString();
                QString requestTime = refund.value(QStringLiteral("request_time")).toString();
                
                // 状态翻译
                QString statusText;
                if (status == "pending") statusText = tr("待审批");
                else if (status == "approved") statusText = tr("已批准");
                else if (status == "rejected") statusText = tr("已拒绝");
                else if (status == "completed") statusText = tr("已完成");
                else statusText = status;
                
                refundText += tr("【退款 #%1】\n").arg(refundId);
                refundText += tr("  订单ID: %1\n").arg(orderId);
                refundText += tr("  退款金额: %1 元\n").arg(amount, 0, 'f', 2);
                refundText += tr("  状态: %1\n").arg(statusText);
                refundText += tr("  申请原因: %1\n").arg(reason);
                if (!adminReply.isEmpty()) {
                    refundText += tr("  管理员回复: %1\n").arg(adminReply);
                }
                refundText += tr("  申请时间: %1\n").arg(requestTime);
                refundText += "\n";
            }
            
            refundText += tr("\n原始JSON:\n") + JsonUtils::pretty(doc);
        }
        
        m_detailView->setPlainText(refundText);
        emit statusMessage(tr("已获取退款列表"), true);
    });
}

void OrdersTab::deleteOrder()
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
    const QJsonObject obj = selectedOrder();
    const QString status = obj.value(QStringLiteral("status")).toString();
    if (status.toLower() != QStringLiteral("cancelled")) {
        emit statusMessage(tr("仅已取消的订单可删除"), false);
        return;
    }
    const QString command = QStringLiteral("DELETE_ORDER %1").arg(orderId);
    sendOrderCommand(command, tr("删除订单"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshOrders();
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
    int unpaidCount = 0;

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

        // 只计算未支付订单的金额(pending和confirmed状态)
        QString statusLower = status.toLower();
        if (statusLower == "pending" || statusLower == "confirmed") {
            totalAmount += payable;
            unpaidCount++;
        }
    }

    if (!orders.isEmpty()) {
        m_orderTable->selectRow(0);
    }

    m_summaryLabel->setText(tr("订单数: %1 | 待支付订单: %2 | 待支付总额: ¥%3")
                                .arg(orders.size())
                                .arg(unpaidCount)
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
