#include "ui/tabs/AdminTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "utils/JsonUtils.h"

#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QPointer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QDate>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace {
QString pretty(const QJsonDocument &doc) { return JsonUtils::pretty(doc); }
}

AdminTab::AdminTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent), m_context(context)
{
    setupUi();
}

void AdminTab::setupUi()
{
    m_tabs = new QTabWidget(this);

    // 库存页
    auto *invPage = new QWidget(this);
    auto *invLayout = new QVBoxLayout(invPage);
    m_lowStockTable = new QTableWidget(invPage);
    m_lowStockTable->setColumnCount(3);
    m_lowStockTable->setHorizontalHeaderLabels({tr("商品ID"), tr("名称"), tr("库存")});
    m_lowStockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    auto *invCtl = new QHBoxLayout();
    m_stockProductId = new QLineEdit(invPage); m_stockProductId->setPlaceholderText(tr("商品ID"));
    m_stockDelta = new QSpinBox(invPage); m_stockDelta->setRange(-100000, 100000); m_stockDelta->setValue(1);
    m_stockOp = new QComboBox(invPage); m_stockOp->addItems({tr("add"), tr("subtract"), tr("set")});
    auto *invRefresh = new QPushButton(tr("刷新低库存"), invPage);
    m_stockApplyBtn = new QPushButton(tr("应用库存调整"), invPage);
    invCtl->addWidget(m_stockProductId);
    invCtl->addWidget(m_stockDelta);
    invCtl->addWidget(m_stockOp);
    invCtl->addWidget(invRefresh);
    invCtl->addWidget(m_stockApplyBtn);
    invCtl->addStretch();
    invLayout->addLayout(invCtl);
    invLayout->addWidget(m_lowStockTable);

    // 订单页
    auto *ordersPage = new QWidget(this);
    auto *ordersLayout = new QVBoxLayout(ordersPage);
    m_ordersTable = new QTableWidget(ordersPage);
    m_ordersTable->setColumnCount(7);
    m_ordersTable->setHorizontalHeaderLabels({tr("订单ID"), tr("用户ID"), tr("状态"), tr("总额"), tr("优惠"), tr("应付"), tr("操作")});
    m_ordersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto *ordersCtl = new QHBoxLayout();
    m_orderStatus = new QComboBox(ordersPage);
    m_orderStatus->addItem(tr("全部"), "all");
    for (const QString &st : {"pending","paid","shipped","delivered","cancelled","refunded"}) m_orderStatus->addItem(st, st);
    m_startDate = new QDateEdit(QDate::currentDate().addMonths(-1), ordersPage); m_startDate->setCalendarPopup(true);
    m_endDate = new QDateEdit(QDate::currentDate(), ordersPage); m_endDate->setCalendarPopup(true);
    m_page = new QSpinBox(ordersPage); m_page->setRange(1, 100000); m_page->setValue(1);
    m_pageSize = new QSpinBox(ordersPage); m_pageSize->setRange(1, 500); m_pageSize->setValue(20);
    auto *ordersRefresh = new QPushButton(tr("刷新订单"), ordersPage);
    m_prevPage = new QPushButton(tr("上一页"), ordersPage);
    m_nextPage = new QPushButton(tr("下一页"), ordersPage);
    ordersCtl->addWidget(m_orderStatus);
    ordersCtl->addWidget(new QLabel(tr("开始"), ordersPage)); ordersCtl->addWidget(m_startDate);
    ordersCtl->addWidget(new QLabel(tr("结束"), ordersPage)); ordersCtl->addWidget(m_endDate);
    ordersCtl->addWidget(new QLabel(tr("页码"), ordersPage)); ordersCtl->addWidget(m_page);
    ordersCtl->addWidget(new QLabel(tr("每页"), ordersPage)); ordersCtl->addWidget(m_pageSize);
    ordersCtl->addWidget(m_prevPage); ordersCtl->addWidget(m_nextPage);
    ordersCtl->addWidget(ordersRefresh); ordersCtl->addStretch();
    ordersLayout->addLayout(ordersCtl);
    ordersLayout->addWidget(m_ordersTable);

    // 促销/优惠券页
    auto *promoPage = new QWidget(this);
    auto *promoLayout = new QVBoxLayout(promoPage);
    m_promotionsTable = new QTableWidget(promoPage);
    m_promotionsTable->setColumnCount(3);
    m_promotionsTable->setHorizontalHeaderLabels({tr("名称"), tr("代码"), tr("详情")});
    m_promotionsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto *promoCtl = new QFormLayout();
    m_promoName = new QLineEdit(promoPage); m_promoName->setPlaceholderText(tr("活动名称"));
    m_promoCode = new QLineEdit(promoPage); m_promoCode->setPlaceholderText(tr("活动/优惠券代码"));
    m_promoJson = new QLineEdit(promoPage); m_promoJson->setPlaceholderText(tr("JSON 配置，例如 {\"discount\":10}"));
    m_promoCreateBtn = new QPushButton(tr("创建促销/优惠券"), promoPage);
    promoCtl->addRow(tr("名称"), m_promoName);
    promoCtl->addRow(tr("代码"), m_promoCode);
    promoCtl->addRow(tr("配置"), m_promoJson);
    promoLayout->addLayout(promoCtl);
    auto *promoBtns = new QHBoxLayout();
    auto *promoRefresh = new QPushButton(tr("刷新促销"), promoPage);
    promoBtns->addWidget(promoRefresh);
    promoBtns->addWidget(m_promoCreateBtn);
    promoBtns->addStretch();
    promoLayout->addLayout(promoBtns);
    promoLayout->addWidget(m_promotionsTable);

    m_tabs->addTab(invPage, tr("库存"));
    m_tabs->addTab(ordersPage, tr("订单"));
    m_tabs->addTab(promoPage, tr("促销/优惠券"));

    auto *root = new QVBoxLayout(this);
    root->addWidget(m_tabs);

    connect(invRefresh, &QPushButton::clicked, this, &AdminTab::refreshLowStock);
    connect(m_stockApplyBtn, &QPushButton::clicked, this, &AdminTab::applyStockChange);
    connect(ordersRefresh, &QPushButton::clicked, this, &AdminTab::refreshAllOrders);
    connect(m_prevPage, &QPushButton::clicked, this, &AdminTab::prevPage);
    connect(m_nextPage, &QPushButton::clicked, this, &AdminTab::nextPage);
    connect(promoRefresh, &QPushButton::clicked, this, &AdminTab::refreshPromotions);
    connect(m_promoCreateBtn, &QPushButton::clicked, this, &AdminTab::createPromotion);
}

void AdminTab::handleSessionChanged(const UserSession &session)
{
    setEnabled(true);
    Q_UNUSED(session);
}

void AdminTab::sendCommand(const QString &cmd,
                           std::function<void(const QJsonDocument&)> onSuccess,
                           const QString &actionLabel)
{
    auto *client = m_context.networkClient();
    if (!client->isConnected()) return;
    QPointer<AdminTab> guard(this);
    client->sendCommand(cmd,
        [this, guard, onSuccess, actionLabel](const QString &resp){
            if (!guard) return;
            bool ok=false; QString err;
            QJsonDocument doc = JsonUtils::parse(resp, &ok, &err);
            if (!ok || !JsonUtils::isSuccess(doc)) {
                // 简单提示，可扩展到状态栏
                qWarning("%s失败: %s", qPrintable(actionLabel), qPrintable(ok? JsonUtils::message(doc): err));
                return;
            }
            if (onSuccess) onSuccess(doc);
        },
        [actionLabel](const QString &e){ qWarning("%s失败: %s", qPrintable(actionLabel), qPrintable(e)); });
}

void AdminTab::refreshLowStock()
{
    // 默认阈值 10，可根据需要提供输入框
    sendCommand("GET_LOW_STOCK_PRODUCTS 10",
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data");
            if (v.isArray()) arr = v.toArray();
            m_lowStockTable->setRowCount(arr.size());
            for (int i=0;i<arr.size();++i){
                QJsonObject o = arr[i].toObject();
                m_lowStockTable->setItem(i,0,new QTableWidgetItem(QString::number(JsonUtils::asLongLong(o.value("product_id"),-1))));
                m_lowStockTable->setItem(i,1,new QTableWidgetItem(o.value("name").toString()));
                m_lowStockTable->setItem(i,2,new QTableWidgetItem(QString::number(JsonUtils::asLongLong(o.value("stock"),0))));
            }
        },
        tr("刷新低库存"));
}

void AdminTab::applyStockChange()
{
    const qlonglong id = m_stockProductId->text().trimmed().toLongLong();
    const int delta = m_stockDelta->value();
    const QString op = m_stockOp->currentText();
    if (id<=0) return;
    QString cmd = QString("UPDATE_STOCK %1 %2 %3").arg(id).arg(delta).arg(op);
    sendCommand(cmd, [this](const QJsonDocument &){ refreshLowStock(); }, tr("更新库存"));
}

void AdminTab::refreshAllOrders()
{
    const QString st = m_orderStatus->currentData().toString();
    const int page = m_page->value();
    const int pageSize = m_pageSize->value();
    const QString s = m_startDate->date().toString("yyyy-MM-dd");
    const QString e = m_endDate->date().toString("yyyy-MM-dd");
    // 管理员：GET_ALL_ORDERS status page pageSize start end
    QString cmd = QString("GET_ALL_ORDERS %1 %2 %3 %4 %5")
                      .arg(st.isEmpty()?"all":st)
                      .arg(page)
                      .arg(pageSize)
                      .arg(s)
                      .arg(e);
    sendCommand(cmd,
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.orders");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) { v = JsonUtils::extract(doc, "data"); if (v.isArray()) arr = v.toArray(); }
            m_ordersTable->setRowCount(arr.size());
            for (int i=0;i<arr.size();++i){
                QJsonObject o = arr[i].toObject();
                auto asD = [&](const QString &k){ return JsonUtils::asDouble(o.value(k), 0.0); };
                const qlonglong orderId = JsonUtils::asLongLong(o.value("order_id"),-1);
                m_ordersTable->setItem(i,0,new QTableWidgetItem(QString::number(orderId)));
                m_ordersTable->setItem(i,1,new QTableWidgetItem(QString::number(JsonUtils::asLongLong(o.value("user_id"),-1))));
                m_ordersTable->setItem(i,2,new QTableWidgetItem(o.value("status").toString()));
                m_ordersTable->setItem(i,3,new QTableWidgetItem(QString::number(asD("total_amount"),'f',2)));
                m_ordersTable->setItem(i,4,new QTableWidgetItem(QString::number(asD("discount_amount"),'f',2)));
                m_ordersTable->setItem(i,5,new QTableWidgetItem(QString::number(asD("final_amount"),'f',2)));
                // 操作列：改状态/详情/退款 按钮文本
                QWidget *ops = new QWidget(m_ordersTable);
                auto *opsLayout = new QHBoxLayout(ops); opsLayout->setContentsMargins(0,0,0,0);
                auto *btnStatus = new QPushButton(tr("改状态"), ops);
                auto *btnDetail = new QPushButton(tr("详情"), ops);
                auto *btnRefund = new QPushButton(tr("退款"), ops);
                opsLayout->addWidget(btnStatus); opsLayout->addWidget(btnDetail); opsLayout->addWidget(btnRefund); opsLayout->addStretch();
                m_ordersTable->setCellWidget(i, 6, ops);
                connect(btnStatus, &QPushButton::clicked, this, [this, orderId]{ changeOrderStatus(orderId); });
                connect(btnDetail, &QPushButton::clicked, this, [this, orderId]{ viewOrderDetail(orderId); });
                connect(btnRefund, &QPushButton::clicked, this, [this, orderId]{ refundOrder(orderId); });
            }
            emit statusMessage(tr("已刷新订单列表"), true);
        },
        tr("刷新订单"));
}

void AdminTab::prevPage(){ if (m_page->value()>1){ m_page->setValue(m_page->value()-1); refreshAllOrders(); } }
void AdminTab::nextPage(){ m_page->setValue(m_page->value()+1); refreshAllOrders(); }

void AdminTab::changeOrderStatus(qlonglong orderId)
{
    // 简化：循环到下一个常见状态（仅演示，可替换为对话框选择）
    QStringList order = {"pending","paid","shipped","delivered","cancelled","refunded"};
    int row = m_ordersTable->currentRow();
    QString cur = row>=0? (m_ordersTable->item(row,2)? m_ordersTable->item(row,2)->text() : QString()) : QString();
    int idx = order.indexOf(cur); QString next = order[(idx+1) % order.size()];
    QString cmd = QString("UPDATE_ORDER_STATUS %1 %2").arg(orderId).arg(next);
    sendCommand(cmd, [this](const QJsonDocument&){ emit statusMessage(tr("订单状态已更新"), true); refreshAllOrders(); }, tr("更新订单状态"));
}

void AdminTab::viewOrderDetail(qlonglong orderId)
{
    QString cmd = QString("VIEW_ORDER %1").arg(orderId);
    sendCommand(cmd, [this](const QJsonDocument &doc){
        QString msg;
        QJsonValue v = JsonUtils::extract(doc, "plain_text");
        if (v.isString()) msg = v.toString();
        if (msg.isEmpty()) msg = JsonUtils::pretty(doc);
        emit statusMessage(tr("订单详情已获取"), true);
        // 这里简单用消息弹窗或复制到剪贴板，先用 qDebug 输出
        qInfo().noquote() << msg;
    }, tr("查看订单详情"));
}

void AdminTab::refundOrder(qlonglong orderId)
{
    QString cmd = QString("REFUND_PAYMENT %1 %2 %3").arg(orderId).arg(0).arg("自动退款");
    sendCommand(cmd, [this](const QJsonDocument&){ emit statusMessage(tr("退款申请已提交"), true); refreshAllOrders(); }, tr("申请退款"));
}

void AdminTab::refreshPromotions()
{
    sendCommand("GET_ACTIVE_PROMOTIONS",
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data");
            if (v.isArray()) arr = v.toArray();
            m_promotionsTable->setRowCount(arr.size());
            for (int i=0;i<arr.size();++i){
                QJsonObject o = arr[i].toObject();
                const QString name = o.value("name").toString();
                const QString code = o.value("code").toString();
                m_promotionsTable->setItem(i,0,new QTableWidgetItem(name));
                m_promotionsTable->setItem(i,1,new QTableWidgetItem(code));
                m_promotionsTable->setItem(i,2,new QTableWidgetItem(QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact))));
            }
        },
        tr("刷新促销"));
}

void AdminTab::createPromotion()
{
    const QString name = m_promoName->text().trimmed();
    const QString code = m_promoCode->text().trimmed();
    const QString json = m_promoJson->text().trimmed();
    if (name.isEmpty() || code.isEmpty()) return;
    // 采用 JSON 透传，后端解析
    QJsonObject obj; obj["name"]=name; obj["code"]=code;
    if (!json.isEmpty()) {
        bool ok=false; QString err; auto extra = JsonUtils::parse(json,&ok,&err);
        if (ok && extra.isObject()) {
            for (auto it=extra.object().begin(); it!=extra.object().end(); ++it) obj[it.key()]=it.value();
        }
    }
    QString payload = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    // 对 JSON 进行简单转义并用双引号包裹，避免服务端按空格拆分
    payload.replace("\\", "\\\\");
    payload.replace("\"", "\\\"");
    QString cmd = QString("CREATE_PROMOTION \"%1\"").arg(payload);
    sendCommand(cmd, [this](const QJsonDocument &){ refreshPromotions(); }, tr("创建促销"));
}
