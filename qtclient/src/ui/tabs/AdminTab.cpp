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
#include <QCheckBox>
#include <QTimer>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QStringList>
#include <QAbstractItemView>

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

    // 用户管理页
    auto *userPage = new QWidget(this);
    auto *userLayout = new QVBoxLayout(userPage);
    auto *userToolbar = new QHBoxLayout();
    m_userSearchEdit = new QLineEdit(userPage);
    m_userSearchEdit->setPlaceholderText(tr("搜索用户名/ID"));
    auto *userSearchBtn = new QPushButton(tr("搜索"), userPage);
    auto *userRefreshBtn = new QPushButton(tr("刷新"), userPage);
    m_userRoleCombo = new QComboBox(userPage);
    m_userRoleCombo->addItems({tr("user"), tr("admin"), tr("suspended")});
    auto *userRoleBtn = new QPushButton(tr("调整角色"), userPage);
    auto *userToggleBtn = new QPushButton(tr("启禁用"), userPage);
    userToolbar->addWidget(m_userSearchEdit);
    userToolbar->addWidget(userSearchBtn);
    userToolbar->addWidget(userRefreshBtn);
    userToolbar->addStretch();
    userToolbar->addWidget(new QLabel(tr("角色"), userPage));
    userToolbar->addWidget(m_userRoleCombo);
    userToolbar->addWidget(userRoleBtn);
    userToolbar->addWidget(userToggleBtn);
    userLayout->addLayout(userToolbar);

    m_usersTable = new QTableWidget(userPage);
    m_usersTable->setColumnCount(5);
    m_usersTable->setHorizontalHeaderLabels({tr("用户ID"), tr("用户名"), tr("角色"), tr("状态"), tr("创建时间")});
    m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_usersTable->setSelectionMode(QAbstractItemView::SingleSelection);

    auto *userSplitter = new QSplitter(Qt::Horizontal, userPage);
    userSplitter->addWidget(m_usersTable);

    auto *detailWidget = new QWidget(userSplitter);
    auto *detailLayout = new QVBoxLayout(detailWidget);
    m_userDetailLabel = new QLabel(tr("请选择一个用户"), detailWidget);
    m_userDetailLabel->setWordWrap(true);
    detailLayout->addWidget(m_userDetailLabel);

    auto *couponLayout = new QHBoxLayout();
    m_couponIssueEdit = new QLineEdit(detailWidget);
    m_couponIssueEdit->setPlaceholderText(tr("优惠券代码"));
    auto *issueBtn = new QPushButton(tr("发放优惠券"), detailWidget);
    couponLayout->addWidget(m_couponIssueEdit);
    couponLayout->addWidget(issueBtn);

    auto *ordersBtn = new QPushButton(tr("查看订单"), detailWidget);
    auto *couponsBtn = new QPushButton(tr("查看优惠券"), detailWidget);
    auto *detailBtnRow = new QHBoxLayout();
    detailBtnRow->addWidget(ordersBtn);
    detailBtnRow->addWidget(couponsBtn);
    detailBtnRow->addStretch();

    m_userOrdersView = new QPlainTextEdit(detailWidget);
    m_userOrdersView->setPlaceholderText(tr("用户订单信息"));
    m_userOrdersView->setReadOnly(true);
    m_userCouponsView = new QPlainTextEdit(detailWidget);
    m_userCouponsView->setPlaceholderText(tr("用户优惠券信息"));
    m_userCouponsView->setReadOnly(true);

    detailLayout->addLayout(detailBtnRow);
    detailLayout->addLayout(couponLayout);
    detailLayout->addWidget(m_userOrdersView);
    detailLayout->addWidget(m_userCouponsView);
    userSplitter->addWidget(detailWidget);
    userSplitter->setStretchFactor(0, 3);
    userSplitter->setStretchFactor(1, 2);

    userLayout->addWidget(userSplitter);

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
    m_lowStockThreshold = new QSpinBox(invPage); m_lowStockThreshold->setRange(1, 100000); m_lowStockThreshold->setValue(10);
    auto *invRefresh = new QPushButton(tr("刷新低库存"), invPage);
    m_stockApplyBtn = new QPushButton(tr("应用库存调整"), invPage);
    invCtl->addWidget(m_stockProductId);
    invCtl->addWidget(m_stockDelta);
    invCtl->addWidget(m_stockOp);
    invCtl->addWidget(new QLabel(tr("阈值"), invPage));
    invCtl->addWidget(m_lowStockThreshold);
    invCtl->addWidget(invRefresh);
    m_autoRefreshLowStock = new QCheckBox(tr("自动刷新"), invPage);
    invCtl->addWidget(m_autoRefreshLowStock);
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

    m_tabs->addTab(userPage, tr("用户"));
    m_tabs->addTab(invPage, tr("库存"));
    m_tabs->addTab(ordersPage, tr("订单"));
    m_tabs->addTab(promoPage, tr("促销/优惠券"));

    auto *root = new QVBoxLayout(this);
    root->addWidget(m_tabs);

    connect(userRefreshBtn, &QPushButton::clicked, this, &AdminTab::refreshUsers);
    connect(userSearchBtn, &QPushButton::clicked, this, &AdminTab::refreshUsers);
    connect(m_userSearchEdit, &QLineEdit::returnPressed, this, &AdminTab::refreshUsers);
    connect(m_usersTable, &QTableWidget::itemSelectionChanged, this, &AdminTab::showSelectedUserDetail);
    connect(userRoleBtn, &QPushButton::clicked, this, &AdminTab::applyUserRole);
    connect(userToggleBtn, &QPushButton::clicked, this, &AdminTab::toggleUserStatus);
    connect(issueBtn, &QPushButton::clicked, this, &AdminTab::issueCouponToUser);
    connect(ordersBtn, &QPushButton::clicked, this, &AdminTab::fetchUserOrders);
    connect(couponsBtn, &QPushButton::clicked, this, &AdminTab::fetchUserCoupons);

    connect(invRefresh, &QPushButton::clicked, this, &AdminTab::refreshLowStock);
    connect(m_stockApplyBtn, &QPushButton::clicked, this, &AdminTab::applyStockChange);
    connect(ordersRefresh, &QPushButton::clicked, this, &AdminTab::refreshAllOrders);
    connect(m_prevPage, &QPushButton::clicked, this, &AdminTab::prevPage);
    connect(m_nextPage, &QPushButton::clicked, this, &AdminTab::nextPage);
    connect(promoRefresh, &QPushButton::clicked, this, &AdminTab::refreshPromotions);
    connect(m_promoCreateBtn, &QPushButton::clicked, this, &AdminTab::createPromotion);

    // 低库存定时刷新
    m_lowStockTimer = new QTimer(this);
    m_lowStockTimer->setInterval(10000); // 10s
    connect(m_lowStockTimer, &QTimer::timeout, this, &AdminTab::refreshLowStock);
    connect(m_autoRefreshLowStock, &QCheckBox::toggled, this, [this](bool on){ if (on) { refreshLowStock(); m_lowStockTimer->start(); } else m_lowStockTimer->stop(); });
}

void AdminTab::handleSessionChanged(const UserSession &session)
{
    setEnabled(true);
    Q_UNUSED(session);
    refreshUsers();
    refreshLowStock();
    refreshAllOrders();
    refreshPromotions();
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

qlonglong AdminTab::selectedUserId() const
{
    if (!m_usersTable) return -1;
    int row = m_usersTable->currentRow();
    if (row < 0 && !m_usersTable->selectedItems().isEmpty()) {
        row = m_usersTable->selectedItems().first()->row();
    }
    if (row < 0) return -1;
    QTableWidgetItem *item = m_usersTable->item(row, 0);
    if (!item) return -1;
    bool ok = false;
    qlonglong id = item->text().toLongLong(&ok);
    return ok ? id : -1;
}

void AdminTab::refreshUsers()
{
    if (!m_usersTable) return;
    QString keyword = m_userSearchEdit ? m_userSearchEdit->text().trimmed() : QString();
    QString cmd = keyword.isEmpty() ? QStringLiteral("GET_ALL_USERS")
                                    : QStringLiteral("GET_ALL_USERS %1").arg(keyword);
    sendCommand(cmd,
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.users");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }
            m_usersTable->setRowCount(arr.size());
            for (int i = 0; i < arr.size(); ++i) {
                QJsonObject o = arr.at(i).toObject();
                const qlonglong uid = JsonUtils::asLongLong(o.value("user_id"), -1);
                const QString username = o.value(QStringLiteral("username")).toString();
                QString role = o.value(QStringLiteral("role")).toString();
                if (role.isEmpty()) role = o.value(QStringLiteral("user_role")).toString();
                QString status = o.value(QStringLiteral("status")).toString();
                if (status.isEmpty() && o.contains(QStringLiteral("enabled"))) {
                    status = o.value(QStringLiteral("enabled")).toBool(true) ? tr("启用") : tr("禁用");
                }
                if (status.isEmpty()) status = tr("未知");
                QString created = o.value(QStringLiteral("created_at")).toString();
                if (created.isEmpty()) created = o.value(QStringLiteral("create_time")).toString();
                if (created.isEmpty()) created = o.value(QStringLiteral("register_time")).toString();

                m_usersTable->setItem(i, 0, new QTableWidgetItem(QString::number(uid)));
                m_usersTable->setItem(i, 1, new QTableWidgetItem(username));
                m_usersTable->setItem(i, 2, new QTableWidgetItem(role));
                m_usersTable->setItem(i, 3, new QTableWidgetItem(status));
                m_usersTable->setItem(i, 4, new QTableWidgetItem(created));
            }
            if (!arr.isEmpty()) {
                m_usersTable->selectRow(0);
            } else {
                showSelectedUserDetail();
            }
            emit statusMessage(tr("已刷新用户列表"), true);
        },
        tr("刷新用户"));
}

void AdminTab::showSelectedUserDetail()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0) {
        if (m_userDetailLabel) m_userDetailLabel->setText(tr("请选择一个用户"));
        if (m_userOrdersView) m_userOrdersView->clear();
        if (m_userCouponsView) m_userCouponsView->clear();
        return;
    }
    int row = m_usersTable->currentRow();
    if (row < 0 && !m_usersTable->selectedItems().isEmpty()) row = m_usersTable->selectedItems().first()->row();
    QString username = m_usersTable->item(row, 1) ? m_usersTable->item(row, 1)->text() : QString();
    QString role = m_usersTable->item(row, 2) ? m_usersTable->item(row, 2)->text() : QString();
    QString status = m_usersTable->item(row, 3) ? m_usersTable->item(row, 3)->text() : QString();
    if (m_userDetailLabel) {
        m_userDetailLabel->setText(tr("用户 %1 (ID:%2)\n角色:%3  状态:%4")
                                       .arg(username)
                                       .arg(uid)
                                       .arg(role)
                                       .arg(status));
    }
    fetchUserOrders();
    fetchUserCoupons();
}

void AdminTab::applyUserRole()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0 || !m_userRoleCombo) return;
    QString role = m_userRoleCombo->currentText().trimmed();
    if (role.isEmpty()) return;
    QString cmd = QStringLiteral("SET_USER_ROLE %1 %2").arg(uid).arg(role);
    sendCommand(cmd,
        [this](const QJsonDocument &){
            emit statusMessage(tr("用户角色已更新"), true);
            refreshUsers();
        },
        tr("调整用户角色"));
}

void AdminTab::toggleUserStatus()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0) return;
    int row = m_usersTable->currentRow();
    QString status = (row >= 0 && m_usersTable->item(row, 3)) ? m_usersTable->item(row, 3)->text().toLower() : QString();
    const bool currentlyEnabled = !(status.contains(QStringLiteral("禁")) || status.contains(QStringLiteral("disable")));
    QString cmd = QStringLiteral("SET_USER_STATUS %1 %2").arg(uid).arg(currentlyEnabled ? QStringLiteral("disable") : QStringLiteral("enable"));
    sendCommand(cmd,
        [this, currentlyEnabled](const QJsonDocument &){
            emit statusMessage(currentlyEnabled ? tr("已禁用用户") : tr("已启用用户"), true);
            refreshUsers();
        },
        currentlyEnabled ? tr("禁用用户") : tr("启用用户"));
}

void AdminTab::issueCouponToUser()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0 || !m_couponIssueEdit) return;
    QString code = m_couponIssueEdit->text().trimmed();
    if (code.isEmpty()) return;
    QString escaped = code;
    escaped.replace("\\", "\\\\");
    escaped.replace('"', "\\\"");
    QString cmd = QStringLiteral("ASSIGN_COUPON %1 \"%2\"").arg(uid).arg(escaped);
    sendCommand(cmd,
        [this](const QJsonDocument &){
            emit statusMessage(tr("优惠券已发放"), true);
            if (m_couponIssueEdit) m_couponIssueEdit->clear();
            fetchUserCoupons();
        },
        tr("发放优惠券"));
}

void AdminTab::fetchUserOrders()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0) return;
    QString cmd = QStringLiteral("GET_USER_ORDERS %1").arg(uid);
    sendCommand(cmd,
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.orders");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }
            QStringList lines;
            for (const QJsonValue &val : arr) {
                QJsonObject o = val.toObject();
                const qlonglong orderId = JsonUtils::asLongLong(o.value("order_id"), -1);
                const QString status = o.value(QStringLiteral("status")).toString();
                double amount = JsonUtils::asDouble(o.value("final_amount"), JsonUtils::asDouble(o.value("total_amount"), 0.0));
                QString created = o.value(QStringLiteral("created_at")).toString();
                lines << tr("订单#%1 [%2] 金额:%3 创建:%4").arg(orderId).arg(status, created).arg(amount, 0, 'f', 2);
            }
            if (m_userOrdersView) {
                m_userOrdersView->setPlainText(lines.isEmpty() ? tr("暂无订单") : lines.join('\n'));
            }
        },
        tr("获取用户订单"));
}

void AdminTab::fetchUserCoupons()
{
    const qlonglong uid = selectedUserId();
    if (uid < 0) return;
    QString cmd = QStringLiteral("GET_USER_COUPONS %1").arg(uid);
    sendCommand(cmd,
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.coupons");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }
            QStringList lines;
            for (const QJsonValue &val : arr) {
                QJsonObject o = val.toObject();
                const QString code = o.value(QStringLiteral("code")).toString();
                const QString name = o.value(QStringLiteral("name")).toString(code);
                const QString expire = o.value(QStringLiteral("expire_at")).toString();
                const QString status = o.value(QStringLiteral("status")).toString();
                lines << tr("%1 (%2) 状态:%3 %4")
                             .arg(code, name, status, expire.isEmpty() ? QString() : tr("到期:%1").arg(expire));
            }
            if (m_userCouponsView) {
                m_userCouponsView->setPlainText(lines.isEmpty() ? tr("暂无优惠券") : lines.join('\n'));
            }
        },
        tr("获取用户优惠券"));
}

void AdminTab::refreshLowStock()
{
    // 默认阈值 10，可根据需要提供输入框
    int threshold = m_lowStockThreshold? m_lowStockThreshold->value() : 10;
    sendCommand(QString("GET_LOW_STOCK_PRODUCTS %1").arg(threshold),
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data");
            if (v.isArray()) arr = v.toArray();
            m_lowStockTable->setRowCount(arr.size());
            for (int i=0;i<arr.size();++i){
                QJsonObject o = arr[i].toObject();
                m_lowStockTable->setItem(i,0,new QTableWidgetItem(QString::number(JsonUtils::asLongLong(o.value("product_id"),-1))));
                m_lowStockTable->setItem(i,1,new QTableWidgetItem(o.value("name").toString()));
                int stock = (int)JsonUtils::asLongLong(o.value("stock"),0);
                auto *stockItem = new QTableWidgetItem(QString::number(stock));
                if (stock < 5) {
                    stockItem->setForeground(QBrush(QColor(200,0,0)));
                    stockItem->setToolTip(tr("库存告急 (<5)"));
                }
                m_lowStockTable->setItem(i,2,stockItem);
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
