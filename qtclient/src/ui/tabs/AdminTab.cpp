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
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTime>
#include <QDateTimeEdit>
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
#include <QSignalBlocker>

namespace {
QString pretty(const QJsonDocument &doc) { return JsonUtils::pretty(doc); }
QString quoteForCommand(const QString &value)
{
    QString escaped = value;
    escaped.replace("\\", "\\\\");
    escaped.replace('"', "\\\"");
    return QStringLiteral("\"%1\"").arg(escaped);
}
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
    m_couponIssueCombo = new QComboBox(detailWidget);
    m_couponIssueCombo->setEditable(true);
    if (auto *line = m_couponIssueCombo->lineEdit()) {
        line->setPlaceholderText(tr("选择或输入优惠券代码"));
    }
    m_couponRefreshBtn = new QPushButton(tr("刷新可发放列表"), detailWidget);
    auto *issueBtn = new QPushButton(tr("发放优惠券"), detailWidget);
    couponLayout->addWidget(m_couponIssueCombo);
    couponLayout->addWidget(m_couponRefreshBtn);
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
    auto *invRefresh = new QPushButton(tr("刷新库存"), invPage);
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
    m_promoType = new QComboBox(promoPage);
    m_promoType->addItem(tr("固定减免"), QStringLiteral("fixed_amount"));
    m_promoType->addItem(tr("百分比折扣"), QStringLiteral("percentage"));
    m_promoType->addItem(tr("免运费"), QStringLiteral("free_shipping"));
    m_promoValue = new QDoubleSpinBox(promoPage);
    m_promoValue->setDecimals(2);
    m_promoValue->setMinimum(0.0);
    m_promoValue->setMaximum(1000000.0);
    m_promoValue->setSuffix(tr(" 元"));
    m_promoValue->setValue(50.0);
    m_promoMinAmount = new QDoubleSpinBox(promoPage);
    m_promoMinAmount->setDecimals(2);
    m_promoMinAmount->setMinimum(0.0);
    m_promoMinAmount->setMaximum(1000000.0);
    m_promoMinAmount->setSuffix(tr(" 元"));
    m_promoMinAmount->setValue(0.0);
    m_promoStart = new QDateTimeEdit(QDateTime::currentDateTime(), promoPage);
    m_promoStart->setCalendarPopup(true);
    m_promoEnd = new QDateTimeEdit(QDateTime::currentDateTime().addMonths(1), promoPage);
    m_promoEnd->setCalendarPopup(true);
    m_promoDescription = new QPlainTextEdit(promoPage);
    m_promoDescription->setPlaceholderText(tr("活动描述或限制条件（可选）"));
    m_promoDescription->setMaximumHeight(60);
    m_promoCreateBtn = new QPushButton(tr("创建促销/优惠券"), promoPage);
    promoCtl->addRow(tr("名称"), m_promoName);
    promoCtl->addRow(tr("代码"), m_promoCode);
    promoCtl->addRow(tr("类型"), m_promoType);
    promoCtl->addRow(tr("折扣值"), m_promoValue);
    promoCtl->addRow(tr("最低消费"), m_promoMinAmount);
    promoCtl->addRow(tr("开始时间"), m_promoStart);
    promoCtl->addRow(tr("结束时间"), m_promoEnd);
    promoCtl->addRow(tr("描述"), m_promoDescription);
    promoLayout->addLayout(promoCtl);

    if (m_promoType && m_promoValue) {
        auto adjustValueField = [this]() {
            if (!m_promoType || !m_promoValue) {
                return;
            }
            const QString typeValue = m_promoType->currentData().toString();
            const QSignalBlocker blocker(m_promoValue);
            if (typeValue == QStringLiteral("percentage")) {
                m_promoValue->setMaximum(100.0);
                m_promoValue->setSuffix(tr(" %"));
                if (m_promoValue->value() > 100.0) {
                    m_promoValue->setValue(10.0);
                }
            } else {
                m_promoValue->setMaximum(1000000.0);
                m_promoValue->setSuffix(tr(" 元"));
            }
        };
        adjustValueField();
        connect(m_promoType, &QComboBox::currentIndexChanged, this, [this, adjustValueField](int){
            adjustValueField();
        });
    }

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
    connect(m_couponRefreshBtn, &QPushButton::clicked, this, &AdminTab::refreshAvailableCoupons);
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
    refreshAvailableCoupons();
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
    QString statusText = (row >= 0 && m_usersTable->item(row, 3)) ? m_usersTable->item(row, 3)->text() : QString();
    const QString normalized = statusText.trimmed().toLower();
    static const QStringList disabledKeywords = {
        QStringLiteral("inactive"), QStringLiteral("disabled"), QStringLiteral("disable"),
        QStringLiteral("banned"), QStringLiteral("ban"), QStringLiteral("suspended"),
        QStringLiteral("suspend"), QStringLiteral("停用"), QStringLiteral("禁用"), QStringLiteral("封禁")
    };
    bool currentlyEnabled = true;
    for (const QString &keyword : disabledKeywords) {
        if (normalized == keyword || normalized.contains(keyword)) {
            currentlyEnabled = false;
            break;
        }
    }
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
    if (uid < 0 || !m_couponIssueCombo) return;
    QString code = m_couponIssueCombo->currentText().trimmed();
    if (code.isEmpty() && m_couponIssueCombo->currentIndex() >= 0) {
        code = m_couponIssueCombo->currentData(Qt::UserRole).toString().trimmed();
    }
    if (code.isEmpty()) return;
    QString escaped = code;
    escaped.replace("\\", "\\\\");
    escaped.replace('"', "\\\"");
    QString cmd = QStringLiteral("ASSIGN_COUPON %1 \"%2\"").arg(uid).arg(escaped);
    sendCommand(cmd,
        [this](const QJsonDocument &){
            emit statusMessage(tr("优惠券已发放"), true);
            if (m_couponIssueCombo && m_couponIssueCombo->lineEdit()) {
                m_couponIssueCombo->lineEdit()->clear();
            }
            refreshAvailableCoupons();
            fetchUserCoupons();
        },
        tr("发放优惠券"));
}

void AdminTab::refreshAvailableCoupons()
{
    if (!m_couponIssueCombo) return;
    sendCommand(QStringLiteral("GET_AVAILABLE_COUPONS"),
        [this](const QJsonDocument &doc){
            if (!m_couponIssueCombo) return;
            QSignalBlocker blocker(m_couponIssueCombo);
            const int previousIndex = m_couponIssueCombo->currentIndex();
            m_couponIssueCombo->clear();

            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.coupons");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }

            for (const QJsonValue &val : arr) {
                QJsonObject obj = val.toObject();
                const QString code = obj.value(QStringLiteral("code")).toString();
                const QString name = obj.value(QStringLiteral("name")).toString();
                const QString title = name.isEmpty() ? code : tr("%1 (%2)").arg(name, code);
                m_couponIssueCombo->addItem(title, code);
            }

            if (m_couponIssueCombo->count() == 0) {
                m_couponIssueCombo->addItem(tr("暂无可发放的优惠券"), QString());
            }

            const int restoreIndex = (previousIndex >= 0 && previousIndex < m_couponIssueCombo->count()) ? previousIndex : 0;
            m_couponIssueCombo->setCurrentIndex(restoreIndex);
            if (m_couponIssueCombo->isEditable() && m_couponIssueCombo->lineEdit()) {
                m_couponIssueCombo->lineEdit()->clear();
            }
        },
        tr("获取可发放优惠券"));
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
            QJsonValue v = JsonUtils::extract(doc, "data.user_coupons");
            if (v.isArray()) {
                arr = v.toArray();
            }
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data.coupons");
                if (v.isArray()) arr = v.toArray();
            }
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }
            QStringList lines;
            for (const QJsonValue &val : arr) {
                QJsonObject o = val.toObject();
                const QString code = o.value(QStringLiteral("code")).toString();
                const QString name = o.value(QStringLiteral("name")).toString(code);
                QString status = o.value(QStringLiteral("status")).toString();
                if (status.isEmpty()) {
                    status = o.value(QStringLiteral("user_coupon_status")).toString();
                }
                if (status.isEmpty()) status = tr("未知");

                QString expire = o.value(QStringLiteral("expire_at")).toString();
                if (expire.isEmpty()) expire = o.value(QStringLiteral("end_time")).toString();

                const QString received = o.value(QStringLiteral("received_at")).toString();
                const QString usedTime = o.value(QStringLiteral("used_at")).toString();

                QStringList pieces;
                pieces << tr("状态:%1").arg(status);
                if (!received.isEmpty()) pieces << tr("领取:%1").arg(received);
                if (!usedTime.isEmpty()) pieces << tr("使用:%1").arg(usedTime);
                if (!expire.isEmpty()) pieces << tr("到期:%1").arg(expire);

                lines << tr("%1 (%2) %3").arg(code, name, pieces.join(QLatin1Char(' ')));
            }
            if (m_userCouponsView) {
                m_userCouponsView->setPlainText(lines.isEmpty() ? tr("暂无优惠券") : lines.join('\n'));
            }
        },
        tr("获取用户优惠券"));
}

void AdminTab::refreshLowStock()
{
    const int requestedThreshold = m_lowStockThreshold ? m_lowStockThreshold->value() : 10;
    sendCommand(QStringLiteral("GET_LOW_STOCK_PRODUCTS %1").arg(requestedThreshold),
        [this, requestedThreshold](const QJsonDocument &doc){
            if (!m_lowStockTable) return;
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.low_stock_products.products");
            if (v.isArray()) {
                arr = v.toArray();
            }
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data.products");
                if (v.isArray()) arr = v.toArray();
            }
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }

            int effectiveThreshold = requestedThreshold;
            QJsonValue thresholdVal = JsonUtils::extract(doc, "data.low_stock_products.threshold");
            if (!thresholdVal.isDouble()) {
                thresholdVal = JsonUtils::extract(doc, "data.threshold");
            }
            if (thresholdVal.isDouble()) {
                effectiveThreshold = static_cast<int>(thresholdVal.toDouble());
                if (m_lowStockThreshold) m_lowStockThreshold->setValue(effectiveThreshold);
            }

            m_lowStockTable->setRowCount(arr.size());
            for (int i = 0; i < arr.size(); ++i) {
                QJsonObject o = arr.at(i).toObject();
                const qlonglong productId = JsonUtils::asLongLong(o.value("product_id"), -1);
                const QString name = o.value(QStringLiteral("name")).toString();
                const int stock = static_cast<int>(JsonUtils::asLongLong(o.value("stock"), 0));
                const bool isLow = o.value(QStringLiteral("is_low_stock")).toInt() == 1 ||
                                   o.value(QStringLiteral("is_low_stock")).toBool(false) ||
                                   stock <= effectiveThreshold;

                m_lowStockTable->setItem(i, 0, new QTableWidgetItem(QString::number(productId)));
                m_lowStockTable->setItem(i, 1, new QTableWidgetItem(name));

                auto *stockItem = new QTableWidgetItem(QString::number(stock));
                if (isLow) {
                    stockItem->setForeground(QBrush(QColor(200, 0, 0)));
                    stockItem->setToolTip(tr("库存告急 (≤%1)").arg(effectiveThreshold));
                }
                m_lowStockTable->setItem(i, 2, stockItem);
            }

            if (!arr.isEmpty()) {
                m_lowStockTable->sortItems(2, Qt::AscendingOrder);
            }
        },
        tr("刷新库存"));
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
                // 操作列：根据订单状态显示不同按钮
                QWidget *ops = new QWidget(m_ordersTable);
                auto *opsLayout = new QHBoxLayout(ops); opsLayout->setContentsMargins(0,0,0,0);
                auto *btnStatus = new QPushButton(tr("改状态"), ops);
                auto *btnDetail = new QPushButton(tr("详情"), ops);
                
                // 如果订单状态是refunding,显示审批/拒绝按钮
                QString orderStatus = o.value("status").toString();
                if (orderStatus == "refunding") {
                    auto *btnApprove = new QPushButton(tr("审批退款"), ops);
                    auto *btnReject = new QPushButton(tr("拒绝退款"), ops);
                    opsLayout->addWidget(btnStatus); opsLayout->addWidget(btnDetail);
                    opsLayout->addWidget(btnApprove); opsLayout->addWidget(btnReject);
                    opsLayout->addStretch();
                    connect(btnApprove, &QPushButton::clicked, this, [this, orderId]{ approveRefund(orderId, true); });
                    connect(btnReject, &QPushButton::clicked, this, [this, orderId]{ approveRefund(orderId, false); });
                } else {
                    auto *btnRefund = new QPushButton(tr("退款"), ops);
                    opsLayout->addWidget(btnStatus); opsLayout->addWidget(btnDetail); opsLayout->addWidget(btnRefund);
                    opsLayout->addStretch();
                    connect(btnRefund, &QPushButton::clicked, this, [this, orderId]{ refundOrder(orderId); });
                }
                m_ordersTable->setCellWidget(i, 6, ops);
                connect(btnStatus, &QPushButton::clicked, this, [this, orderId]{ changeOrderStatus(orderId); });
                connect(btnDetail, &QPushButton::clicked, this, [this, orderId]{ viewOrderDetail(orderId); });
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

void AdminTab::approveRefund(qlonglong orderId, bool approve)
{
    // 首先获取该订单的refund_id
    // 传递负数的orderId，让后端按order_id查询
    QString getCmd = QString("GET_USER_REFUND_REQUESTS %1").arg(-orderId);
    sendCommand(getCmd, [this, orderId, approve](const QJsonDocument &doc){
        // 从返回结果中查找该订单的退款申请
        QJsonArray refunds = JsonUtils::extract(doc, "data").toArray();
        
        long refundId = 0;
        for (const QJsonValue &val : refunds) {
            QJsonObject refund = val.toObject();
            if (refund.value("order_id").toVariant().toLongLong() == orderId) {
                refundId = refund.value("refund_id").toVariant().toLongLong();
                break;
            }
        }
        
        if (refundId == 0) {
            emit statusMessage(tr("未找到该订单的退款申请"), false);
            return;
        }
        
        // 调用审批接口
        QString approveStr = approve ? "true" : "false";
        QString adminReply = approve ? "管理员已同意退款" : "管理员已拒绝退款";
        QString cmd = QString("APPROVE_REFUND %1 %2 %3").arg(refundId).arg(approveStr).arg(adminReply);
        sendCommand(cmd, [this, approve](const QJsonDocument&){
            emit statusMessage(approve ? tr("退款已审批通过") : tr("退款已拒绝"), true);
            refreshAllOrders();
        }, approve ? tr("审批退款") : tr("拒绝退款"));
    }, tr("获取退款信息"));
}

void AdminTab::refreshPromotions()
{
    sendCommand("GET_ACTIVE_PROMOTIONS",
        [this](const QJsonDocument &doc){
            QJsonArray arr;
            QJsonValue v = JsonUtils::extract(doc, "data.promotions");
            if (v.isArray()) arr = v.toArray();
            if (arr.isEmpty()) {
                v = JsonUtils::extract(doc, "data");
                if (v.isArray()) arr = v.toArray();
            }
            m_promotionsTable->setRowCount(arr.size());
            for (int i=0;i<arr.size();++i){
                QJsonObject o = arr[i].toObject();
                const QString name = o.value("name").toString();
                QString code = o.value("code").toString();
                if (code.isEmpty()) code = o.value("promotion_code").toString();
                if (code.isEmpty()) code = o.value("coupon_code").toString();
                if (code.isEmpty()) code = o.value("id").toVariant().toString();

                QString type = o.value("discount_type").toString();
                if (type.isEmpty()) type = o.value("type").toString();
                const double value = JsonUtils::asDouble(o.value("discount_value"), JsonUtils::asDouble(o.value("value"), 0.0));
                const double minAmount = JsonUtils::asDouble(o.value("min_amount"), 0.0);
                const double maxDiscount = JsonUtils::asDouble(o.value("max_discount"), -1.0);
                QString start = o.value("start_date").toString();
                if (start.isEmpty()) start = o.value("start_time").toString();
                QString end = o.value("end_date").toString();
                if (end.isEmpty()) end = o.value("end_time").toString();
                QString status = o.value("status").toString();
                if (status.isEmpty()) status = o.value("is_active").toBool(true) ? tr("active") : tr("inactive");
                const QString description = o.value("description").toString();

                QString discountLabel;
                if (type.contains("percent", Qt::CaseInsensitive)) {
                    discountLabel = tr("折扣:%1%").arg(value, 0, 'f', value == static_cast<int>(value) ? 0 : 1);
                } else if (type.contains("free_shipping", Qt::CaseInsensitive)) {
                    discountLabel = tr("免运费");
                } else if (value > 0.0) {
                    discountLabel = tr("立减:%1元").arg(value, 0, 'f', 2);
                }

                QString thresholdLabel;
                if (minAmount > 0.0) {
                    thresholdLabel = tr("门槛:%1元").arg(minAmount, 0, 'f', 2);
                }
                QString maxLabel;
                if (maxDiscount >= 0.0) {
                    maxLabel = tr("封顶:%1元").arg(maxDiscount, 0, 'f', 2);
                }
                QString validity;
                if (!start.isEmpty() || !end.isEmpty()) {
                    validity = tr("有效期:%1~%2").arg(start.isEmpty() ? tr("立即") : start,
                                                   end.isEmpty() ? tr("长期") : end);
                }

                QStringList detailParts;
                if (!discountLabel.isEmpty()) detailParts << discountLabel;
                if (!thresholdLabel.isEmpty()) detailParts << thresholdLabel;
                if (!maxLabel.isEmpty()) detailParts << maxLabel;
                if (!validity.isEmpty()) detailParts << validity;
                if (!status.isEmpty()) detailParts << tr("状态:%1").arg(status);
                if (!description.isEmpty()) detailParts << description;

                m_promotionsTable->setItem(i,0,new QTableWidgetItem(name));
                m_promotionsTable->setItem(i,1,new QTableWidgetItem(code));
                m_promotionsTable->setItem(i,2,new QTableWidgetItem(detailParts.join(QLatin1String(" | "))));
                m_promotionsTable->item(i,2)->setData(Qt::UserRole, QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
            }
        },
        tr("刷新促销"));
}

void AdminTab::createPromotion()
{
    const QString name = m_promoName ? m_promoName->text().trimmed() : QString();
    QString code = m_promoCode ? m_promoCode->text().trimmed() : QString();
    if (code.isEmpty() || name.isEmpty()) {
        emit statusMessage(tr("请填写活动名称和代码"), false);
        return;
    }
    code = code.toUpper();

    const QString type = m_promoType ? m_promoType->currentData().toString() : QStringLiteral("fixed_amount");
    const double value = m_promoValue ? m_promoValue->value() : 0.0;
    if (type == QStringLiteral("percentage") && (value <= 0.0 || value > 100.0)) {
        emit statusMessage(tr("折扣百分比需在0-100之间"), false);
        return;
    }
    if ((type == QStringLiteral("fixed_amount") || type == QStringLiteral("free_shipping")) && value < 0.0) {
        emit statusMessage(tr("折扣金额不能为负"), false);
        return;
    }

    const double minAmount = m_promoMinAmount ? m_promoMinAmount->value() : 0.0;
    const QString description = m_promoDescription ? m_promoDescription->toPlainText().trimmed() : QString();
    const QDateTime startDt = m_promoStart ? m_promoStart->dateTime() : QDateTime();
    const QDateTime endDt = m_promoEnd ? m_promoEnd->dateTime() : QDateTime();
    if (m_promoStart && m_promoEnd && startDt > endDt) {
        emit statusMessage(tr("结束时间需晚于开始时间"), false);
        return;
    }

    QJsonObject obj;
    obj["name"] = name;
    obj["code"] = code;
    obj["discount_type"] = type;
    obj["discount_value"] = value;
    obj["min_amount"] = minAmount;
    obj["status"] = QStringLiteral("active");
    if (!description.isEmpty()) {
        obj["description"] = description;
    }
    if (m_promoStart) {
        obj["start_date"] = startDt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    }
    if (m_promoEnd) {
        obj["end_date"] = endDt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    }

    QString payload = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    const QString cmd = QStringLiteral("CREATE_PROMOTION %1").arg(quoteForCommand(payload));
    sendCommand(cmd,
        [this](const QJsonDocument &doc){
            emit statusMessage(JsonUtils::message(doc).isEmpty() ? tr("促销活动创建成功") : JsonUtils::message(doc), JsonUtils::isSuccess(doc));
            refreshPromotions();
            refreshAvailableCoupons();
            if (JsonUtils::isSuccess(doc)) {
                if (m_promoName) m_promoName->clear();
                if (m_promoCode) m_promoCode->clear();
                if (m_promoDescription) m_promoDescription->clear();
            }
        },
        tr("创建促销"));
}
