#include "ui/tabs/CartTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "ui/dialogs/AddressDialog.h"
#include "utils/JsonUtils.h"

#include <QComboBox>
#include <QColor>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>
#include <QTimer>
#include <initializer_list>

namespace {
QString quoteForCommand(const QString &value)
{
    QString escaped = value;
    escaped.replace("\\", "\\\\");
    escaped.replace('"', "\\\"");
    return QStringLiteral("\"%1\"").arg(escaped);
}

QJsonArray extractCartItems(const QJsonDocument &doc)
{
    static const QStringList paths = {
        QStringLiteral("data.items"),
        QStringLiteral("data.cart_items"),
        QStringLiteral("data.cart.items"),
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

QJsonArray extractAddresses(const QJsonDocument &doc)
{
    static const QStringList paths = {
        QStringLiteral("data.addresses"),
        QStringLiteral("data.list"),
        QStringLiteral("addresses"),
        QStringLiteral("data")
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

qlonglong readProductId(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("product_id"),
        QStringLiteral("productId"),
        QStringLiteral("id")
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

QString readProductName(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("name"),
        QStringLiteral("product_name"),
        QStringLiteral("title")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toString();
        }
    }
    return QObject::tr("商品");
}

int readQuantity(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("quantity"),
        QStringLiteral("count"),
        QStringLiteral("qty")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toInt(1);
        }
    }
    return 1;
}

double readPrice(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("price"),
        QStringLiteral("unit_price"),
        QStringLiteral("sale_price")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return JsonUtils::asDouble(obj.value(key), 0.0);
        }
    }
    return 0.0;
}

double readSubtotal(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("subtotal"),
        QStringLiteral("total_price"),
        QStringLiteral("line_total"),
        QStringLiteral("amount")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return JsonUtils::asDouble(obj.value(key), 0.0);
        }
    }
    double price = readPrice(obj);
    return price * readQuantity(obj);
}

qlonglong readAddressId(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("address_id"),
        QStringLiteral("id"),
        QStringLiteral("addressId")
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

int readAvailableStock(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("available_stock"),
        QStringLiteral("stock"),
        QStringLiteral("remain"),
        QStringLiteral("available"),
        QStringLiteral("availableQuantity"),
        QStringLiteral("inventory")
    };
    for (const auto &key : keys) {
        if (!obj.contains(key)) {
            continue;
        }
        const QJsonValue val = obj.value(key);
        if (val.isDouble()) {
            return val.toInt();
        }
        if (val.isString()) {
            bool ok = false;
            const int parsed = val.toString().toInt(&ok);
            if (ok) {
                return parsed;
            }
        }
        if (val.isObject()) {
            const int nested = readAvailableStock(val.toObject());
            if (nested >= 0) {
                return nested;
            }
        }
    }
    return -1;
}

QString formatAddressDisplay(const QJsonObject &obj)
{
    const QString receiver = obj.value(QStringLiteral("receiver_name")).toString(obj.value(QStringLiteral("recipient")).toString(QObject::tr("收件人")));
    const QString phone = obj.value(QStringLiteral("phone")).toString(obj.value(QStringLiteral("mobile")).toString());
    const QString province = obj.value(QStringLiteral("province")).toString();
    const QString city = obj.value(QStringLiteral("city")).toString();
    const QString district = obj.value(QStringLiteral("district")).toString();
    const QString detail = obj.value(QStringLiteral("detail_address")).toString(obj.value(QStringLiteral("address")).toString());
    const bool isDefault = obj.value(QStringLiteral("is_default")).toBool(false);

    QStringList parts;
    parts << receiver;
    if (!phone.isEmpty()) {
        parts << phone;
    }
    QStringList region;
    if (!province.isEmpty()) region << province;
    if (!city.isEmpty()) region << city;
    if (!district.isEmpty()) region << district;
    if (!detail.isEmpty()) region << detail;
    if (!region.isEmpty()) {
        parts << region.join(' ');
    }
    QString display = parts.join(" | ");
    if (isDefault) {
        display.prepend(QObject::tr("[默认] "));
    }
    return display;
}

} // namespace

CartTab::CartTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    m_cartTable = new QTableWidget(this);
    m_cartTable->setColumnCount(6);
    m_cartTable->setHorizontalHeaderLabels({tr("选择"), tr("商品ID"), tr("商品"), tr("单价"), tr("数量"), tr("小计")});
    m_cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cartTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_cartTable, &QTableWidget::itemChanged, this, &CartTab::handleItemChanged);

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(150);

    m_summaryLabel = new QLabel(tr("购物车为空"), this);
    m_couponInfoLabel = new QLabel(tr("优惠券：暂无可用"), this);
    m_couponInfoLabel->setWordWrap(true);
    m_warningLabel = new QLabel(this);
    m_warningLabel->setWordWrap(true);
    m_warningLabel->setStyleSheet(QStringLiteral("color:#d9534f;"));
    m_warningLabel->setVisible(false);

    m_addressCombo = new QComboBox(this);
    // 优惠券：下拉选择 + 手动输入备选
    m_couponCombo = new QComboBox(this);
    m_couponCombo->setEditable(false);
    m_couponCombo->addItem(tr("不使用优惠券"), QString());
    m_couponEdit = new QLineEdit(this);
    m_couponEdit->setPlaceholderText(tr("或手动输入优惠码"));
    m_remarkEdit = new QLineEdit(this);
    m_remarkEdit->setPlaceholderText(tr("可选：订单备注"));
    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 999);
    m_quantitySpin->setValue(1);

    auto *buttonLayout = new QHBoxLayout;
    auto *refreshButton = new QPushButton(tr("刷新购物车"), this);
    auto *updateButton = new QPushButton(tr("更新数量"), this);
    auto *removeButton = new QPushButton(tr("移除"), this);
    auto *clearButton = new QPushButton(tr("清空"), this);
    auto *selectAllButton = new QPushButton(tr("全选"), this);
    auto *selectNoneButton = new QPushButton(tr("全不选"), this);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(selectNoneButton);
    buttonLayout->addStretch();

    auto *addressLayout = new QHBoxLayout;
    auto *refreshAddrButton = new QPushButton(tr("刷新地址"), this);
    auto *addAddressButton = new QPushButton(tr("新建地址"), this);
    addressLayout->addWidget(new QLabel(tr("收货地址"), this));
    addressLayout->addWidget(m_addressCombo);
    addressLayout->addWidget(refreshAddrButton);
    addressLayout->addWidget(addAddressButton);

    auto *orderLayout = new QFormLayout;
    orderLayout->addRow(tr("优惠券"), m_couponCombo);
    orderLayout->addRow(tr("促销码"), m_couponEdit);
    orderLayout->addRow(tr("备注"), m_remarkEdit);
    orderLayout->addRow(tr("数量调整"), m_quantitySpin);

    auto *createOrderButton = new QPushButton(tr("提交订单"), this);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_cartTable, 1);
    layout->addWidget(new QLabel(tr("明细"), this));
    layout->addWidget(m_detailView);
    layout->addWidget(m_summaryLabel);
    layout->addWidget(m_couponInfoLabel);
    layout->addWidget(m_warningLabel);
    layout->addLayout(addressLayout);
    layout->addLayout(orderLayout);
    layout->addWidget(createOrderButton);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(8000);
    connect(m_refreshTimer, &QTimer::timeout, this, [this]{
        if (!m_loggedIn) {
            return;
        }
        refreshCart();
        refreshUserCoupons();
    });

    connect(refreshButton, &QPushButton::clicked, this, &CartTab::refreshCart);
    connect(updateButton, &QPushButton::clicked, this, &CartTab::updateItemQuantity);
    connect(removeButton, &QPushButton::clicked, this, &CartTab::removeSelectedItem);
    connect(clearButton, &QPushButton::clicked, this, &CartTab::clearCart);
    connect(refreshAddrButton, &QPushButton::clicked, this, &CartTab::refreshAddresses);
    connect(addAddressButton, &QPushButton::clicked, this, &CartTab::addNewAddress);
    connect(m_couponCombo, &QComboBox::currentIndexChanged, this, &CartTab::handleCouponSelectionChanged);
    connect(createOrderButton, &QPushButton::clicked, this, &CartTab::createOrder);
    connect(m_cartTable, &QTableWidget::itemSelectionChanged, this, &CartTab::updateDetailView);
    connect(selectAllButton, &QPushButton::clicked, this, [this]{ handleToggleAllSelect(true); });
    connect(selectNoneButton, &QPushButton::clicked, this, [this]{ handleToggleAllSelect(false); });

    setLoggedIn(false);
}

void CartTab::handleSessionChanged(const UserSession &session)
{
    setLoggedIn(session.isValid());
    if (session.isValid()) {
        refreshCart();
        refreshAddresses();
        m_couponCount = 0;
        refreshUserCoupons();
        if (m_refreshTimer && !m_refreshTimer->isActive()) {
            m_refreshTimer->start();
        }
        if (m_couponInfoLabel) {
            m_couponInfoLabel->setText(tr("优惠券：正在同步…"));
        }
    } else {
        if (m_refreshTimer) {
            m_refreshTimer->stop();
        }
        m_cartTable->setRowCount(0);
        m_addresses.clear();
        m_addressCombo->clear();
        m_summaryLabel->setText(tr("购物车为空"));
        m_detailView->clear();
        m_couponCombo->clear();
        m_couponCombo->addItem(tr("不使用优惠券"), QString());
        m_lastCartTotal = 0.0;
        m_couponCount = 0;
        if (m_couponInfoLabel) {
            m_couponInfoLabel->setText(tr("优惠券：请登录后查看"));
        }
        if (m_warningLabel) {
            m_warningLabel->clear();
            m_warningLabel->setVisible(false);
        }
    }
}

void CartTab::refreshCart()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    sendCartCommand(QStringLiteral("GET_CART"), tr("获取购物车"), [this](const QJsonDocument &doc) {
        populateCart(doc);
    });
}

void CartTab::refreshAddresses()
{
    if (!m_loggedIn) {
        return;
    }

    NetworkClient *client = m_context.networkClient();
    QPointer<CartTab> guard(this);
    client->sendCommand(QStringLiteral("VIEW_ADDRESSES"),
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                emit statusMessage(tr("刷新地址失败: %1").arg(error), false);
                                return;
                            }
                            if (!JsonUtils::isSuccess(doc)) {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) msg = response;
                                emit statusMessage(tr("刷新地址失败: %1").arg(msg), false);
                                return;
                            }
                            populateAddresses(doc);
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("刷新地址失败: %1").arg(error), false);
                        });
}

void CartTab::updateItemQuantity()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    qlonglong productId = selectedProductId();
    if (productId < 0) {
        emit statusMessage(tr("请选择要更新的商品"), false);
        return;
    }

    int quantity = m_quantitySpin->value();
    const UserSession &session = m_context.session();
    QString command = QStringLiteral("UPDATE_CART %1 %2 %3")
                          .arg(session.userId)
                          .arg(productId)
                          .arg(quantity);

    sendCartCommand(command, tr("更新购物车"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshCart();
    });
}

void CartTab::removeSelectedItem()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    qlonglong productId = selectedProductId();
    if (productId < 0) {
        emit statusMessage(tr("请选择要移除的商品"), false);
        return;
    }

    // 会话式：仅传 productId；服务端会从 session 取 userId
    QString command = QStringLiteral("REMOVE_FROM_CART %1").arg(productId);

    sendCartCommand(command, tr("移除商品"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshCart();
    });
}

void CartTab::clearCart()
{
    if (!m_loggedIn) {
        return;
    }
    const UserSession &session = m_context.session();
    QString command = QStringLiteral("CLEAR_CART %1").arg(session.userId);
    sendCartCommand(command, tr("清空购物车"), [this](const QJsonDocument &doc) {
        Q_UNUSED(doc);
        refreshCart();
    });
}

void CartTab::createOrder()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    AddressRecord address = currentAddress();
    if (address.id < 0) {
        emit statusMessage(tr("请先选择收货地址"), false);
        return;
    }

    // 优先使用下拉选择的优惠券，否则使用手动输入
    QString coupon = m_couponCombo->currentData().toString();
    if (coupon.isEmpty()) {
        coupon = m_couponEdit->text().trimmed();
    }
    QString couponArg = coupon.isEmpty() ? QStringLiteral("0") : coupon;
    QString remark = m_remarkEdit->text().trimmed();
    QString remarkArg = remark.isEmpty() ? QStringLiteral("无备注") : remark;

    // 决策：若表格中存在任意复选勾选(服务端selected=1)，则批量下单；否则按当前行进行单件下单
    int checkedCount = 0;
    for (int r = 0; r < m_cartTable->rowCount(); ++r) {
        QTableWidgetItem *chk = m_cartTable->item(r, 0);
        if (chk && chk->checkState() == Qt::Checked) {
            ++checkedCount;
        }
    }

    QString command;
    if (checkedCount > 0) {
        // 使用服务端 selected=1 的条目生成订单
        command = QStringLiteral("CREATE_ORDER %1 %2 %3")
                      .arg(address.id)
                      .arg(couponArg)
                      .arg(quoteForCommand(remarkArg));
    } else {
        // 无复选勾选时，提供快速单件下单
        qlonglong productId = selectedProductId();
        if (productId < 0) {
            emit statusMessage(tr("请选择要下单的商品，或勾选需要结算的条目"), false);
            return;
        }
        int quantity = readQuantity(selectedCartItem());
        command = QStringLiteral("CREATE_ORDER_ITEM %1 %2 %3 %4 %5")
                      .arg(productId)
                      .arg(quantity)
                      .arg(address.id)
                      .arg(couponArg)
                      .arg(quoteForCommand(remarkArg));
    }

    NetworkClient *client = m_context.networkClient();
    QPointer<CartTab> guard(this);
    client->sendCommand(command,
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                emit statusMessage(tr("创建订单失败: %1").arg(error), false);
                                return;
                            }
                            if (!JsonUtils::isSuccess(doc)) {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) msg = response;
                                emit statusMessage(tr("创建订单失败: %1").arg(msg), false);
                                return;
                            }
                            // 构造包含库存变动摘要的状态消息
                            QString statusMsg = tr("订单创建成功");
                            QJsonValue stockVal = JsonUtils::extract(doc, QStringLiteral("data.stock_changes"));
                            if (stockVal.isArray()) {
                                QJsonArray arr = stockVal.toArray();
                                if (!arr.isEmpty()) {
                                    QStringList parts;
                                    for (const auto &v : arr) {
                                        QJsonObject o = v.toObject();
                                        qlonglong pid = JsonUtils::asLongLong(o.value("product_id"), -1);
                                        int deducted = o.value("deducted").toInt();
                                        int remain = o.value("remaining").toInt(-1);
                                        parts << tr("#%1 -%2 剩%3").arg(pid).arg(deducted).arg(remain >= 0 ? QString::number(remain) : QStringLiteral("?"));
                                    }
                                    statusMsg += tr(" | 库存: %1").arg(parts.join(", "));
                                    // 发射带库存变化的信号
                                    emit orderCreatedWithStock(arr);
                                }
                            }
                            emit statusMessage(statusMsg, true);
                            // 下单成功后刷新购物车（已移除相应条目或清空）
                            refreshCart();
                            // 触发外部刷新（例如ProductsTab可连接此信号刷新库存展示）
                            emit orderCreated();
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("创建订单失败: %1").arg(error), false);
                        });
}

void CartTab::refreshUserCoupons()
{
    if (!m_loggedIn) {
        return;
    }
    NetworkClient *client = m_context.networkClient();
    QPointer<CartTab> guard(this);
    // 优先按会话获取用户优惠券；失败则回退到全局可用优惠券
    client->sendCommand(QStringLiteral("GET_USER_COUPONS"),
                        [this, guard](const QString &response) {
                            if (!guard) return;
                            bool ok = false; QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok || !JsonUtils::isSuccess(doc)) {
                                const QString msg = ok ? JsonUtils::message(doc) : error;
                                emit statusMessage(tr("刷新优惠券失败: %1").arg(msg.isEmpty() ? response : msg), false);
                                return;
                            }
                            // 期望数据路径 data.user_coupons / data.list / data.items
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

                            QJsonArray arr;
                            for (const QString &path : couponPaths) {
                                QJsonValue v = JsonUtils::extract(doc, path);
                                if (v.isArray()) {
                                    arr = v.toArray();
                                    break;
                                }
                            }

                            auto pickString = [](const QJsonObject &obj, std::initializer_list<QString> keys) -> QString {
                                for (const QString &key : keys) {
                                    const QJsonValue val = obj.value(key);
                                    if (val.isString()) {
                                        const QString text = val.toString().trimmed();
                                        if (!text.isEmpty()) {
                                            return text;
                                        }
                                    }
                                }
                                return {};
                            };

                            // 重建下拉
                            m_couponCombo->clear();
                            m_couponCombo->addItem(tr("不使用优惠券"), QString());
                            m_couponCount = arr.size();
                            for (const QJsonValue &val : arr) {
                                const QJsonObject o = val.toObject();
                                const QString code = pickString(o, {QStringLiteral("code"), QStringLiteral("coupon_code"), QStringLiteral("couponCode"), QStringLiteral("coupon_id_str")});
                                const QString name = pickString(o, {QStringLiteral("name"), QStringLiteral("title"), QStringLiteral("description")});
                                if (!code.isEmpty()) {
                                    const int idx = m_couponCombo->count();
                                    const QString display = name.isEmpty() ? code : QStringLiteral("%1 (%2)").arg(name, code);
                                    m_couponCombo->addItem(display, code);
                                    m_couponCombo->setItemData(idx, QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)), Qt::UserRole + 1);
                                }
                            }

                            if (m_couponCount > 0) {
                                emit statusMessage(tr("已同步优惠券 (%1 张)").arg(m_couponCount), true);
                            } else {
                                emit statusMessage(tr("未找到可用优惠券"), true);
                            }
                            updateCouponInfoHint();
                        },
                        [this, guard](const QString &error) {
                            if (!guard) return;
                            // 回退：尝试获取全局可用优惠券
                            NetworkClient *client2 = m_context.networkClient();
                            client2->sendCommand(QStringLiteral("GET_AVAILABLE_COUPONS"),
                                [this, guard](const QString &resp2) {
                                    if (!guard) return;
                                    bool ok2 = false; QString err2;
                                    QJsonDocument doc2 = JsonUtils::parse(resp2, &ok2, &err2);
                                    if (!ok2 || !JsonUtils::isSuccess(doc2)) {
                                        const QString msg = ok2 ? JsonUtils::message(doc2) : err2;
                                        emit statusMessage(tr("刷新优惠券失败: %1").arg(msg.isEmpty() ? resp2 : msg), false);
                                        return;
                                    }
                                    QJsonArray arr;
                                    QJsonValue v = JsonUtils::extract(doc2, QStringLiteral("data.coupons"));
                                    if (v.isArray()) arr = v.toArray();
                                    if (arr.isEmpty()) {
                                        v = JsonUtils::extract(doc2, QStringLiteral("data"));
                                        if (v.isArray()) arr = v.toArray();
                                    }
                                    auto pickString = [](const QJsonObject &obj, std::initializer_list<QString> keys) -> QString {
                                        for (const QString &key : keys) {
                                            const QJsonValue val = obj.value(key);
                                            if (val.isString()) {
                                                const QString text = val.toString().trimmed();
                                                if (!text.isEmpty()) {
                                                    return text;
                                                }
                                            }
                                        }
                                        return {};
                                    };
                                    m_couponCombo->clear(); m_couponCombo->addItem(tr("不使用优惠券"), QString());
                                    m_couponCount = arr.size();
                                    for (const QJsonValue &val : arr) {
                                        const QJsonObject o = val.toObject();
                                        const QString code = pickString(o, {QStringLiteral("code"), QStringLiteral("coupon_code"), QStringLiteral("couponCode"), QStringLiteral("coupon_id_str")});
                                        const QString name = pickString(o, {QStringLiteral("name"), QStringLiteral("title"), QStringLiteral("description")});
                                        if (!code.isEmpty()) {
                                            const int idx = m_couponCombo->count();
                                            const QString display = name.isEmpty() ? code : QStringLiteral("%1 (%2)").arg(name, code);
                                            m_couponCombo->addItem(display, code);
                                            m_couponCombo->setItemData(idx, QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)), Qt::UserRole + 1);
                                        }
                                    }
                                    if (m_couponCount > 0) {
                                        emit statusMessage(tr("已同步优惠券 (%1 张)").arg(m_couponCount), true);
                                    } else {
                                        emit statusMessage(tr("未找到可用优惠券"), true);
                                    }
                                    updateCouponInfoHint();
                                },
                                [this, guard](const QString &err2) {
                                    if (!guard) return;
                                    emit statusMessage(tr("刷新优惠券失败: %1").arg(err2), false);
                                });
                        });
}

void CartTab::addNewAddress()
{
    if (!m_loggedIn) {
        emit statusMessage(tr("请先登录"), false);
        return;
    }

    AddressDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    AddressFormData data = dialog.formData();
    QString command = QStringLiteral("ADD_ADDRESS %1 %2 %3 %4 %5 %6 %7 %8")
                          .arg(quoteForCommand(data.receiverName))
                          .arg(data.phone)
                          .arg(quoteForCommand(data.province))
                          .arg(quoteForCommand(data.city))
                          .arg(quoteForCommand(data.district))
                          .arg(quoteForCommand(data.detailAddress))
                          .arg(data.postalCode.isEmpty() ? QStringLiteral("000000") : data.postalCode)
                          .arg(data.isDefault ? QStringLiteral("true") : QStringLiteral("false"));

    NetworkClient *client = m_context.networkClient();
    QPointer<CartTab> guard(this);
    client->sendCommand(command,
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (ok && JsonUtils::isSuccess(doc)) {
                                emit statusMessage(tr("新增地址成功"), true);
                                refreshAddresses();
                            } else {
                                QString msg = ok ? JsonUtils::message(doc) : error;
                                if (msg.isEmpty()) msg = response;
                                emit statusMessage(tr("新增地址失败: %1").arg(msg), false);
                            }
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("新增地址失败: %1").arg(error), false);
                        });
}

void CartTab::updateDetailView()
{
    QJsonObject obj = selectedCartItem();
    if (obj.isEmpty()) {
        m_detailView->clear();
        return;
    }
    m_detailView->setPlainText(JsonUtils::pretty(QJsonDocument(obj)));
}

void CartTab::sendCartCommand(const QString &command, const QString &successAction,
                              std::function<void(const QJsonDocument &)> onSuccess)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        emit statusMessage(tr("未连接到服务器"), false);
        return;
    }

    QPointer<CartTab> guard(this);
    client->sendCommand(command,
                        [this, guard, successAction, onSuccess](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                emit statusMessage(tr("%1失败: %2").arg(successAction, error), false);
                                return;
                            }
                            if (!JsonUtils::isSuccess(doc)) {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) msg = response;
                                emit statusMessage(tr("%1失败: %2").arg(successAction, msg), false);
                                return;
                            }
                            emit statusMessage(tr("%1成功").arg(successAction), true);
                            if (onSuccess) {
                                onSuccess(doc);
                            }
                        },
                        [this, guard, successAction](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("%1失败: %2").arg(successAction, error), false);
                        });
}

void CartTab::populateCart(const QJsonDocument &doc)
{
    m_updatingTable = true;
    QJsonArray items = extractCartItems(doc);
    m_cartTable->setRowCount(items.size());

    double total = 0.0;
    double discount = 0.0;
    double payable = 0.0;
    QStringList conflictHints;

    for (int row = 0; row < items.size(); ++row) {
        const QJsonObject obj = items.at(row).toObject();
        const qlonglong productId = readProductId(obj);
        const QString name = readProductName(obj);
        const double price = readPrice(obj);
        const int quantity = readQuantity(obj);
        const double subtotal = readSubtotal(obj);
        const int available = readAvailableStock(obj);
        bool selected = false;
        {
            QJsonValue selVal = obj.value(QStringLiteral("selected"));
            if (selVal.isBool()) selected = selVal.toBool();
            else if (selVal.isDouble()) selected = (selVal.toInt() != 0);
        }

        auto *checkItem = new QTableWidgetItem();
        checkItem->setFlags(checkItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        checkItem->setCheckState(selected ? Qt::Checked : Qt::Unchecked);
        m_cartTable->setItem(row, 0, checkItem);

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            m_cartTable->setItem(row, column, item);
        };

        setItem(1, QString::number(productId));
        setItem(2, name);
        setItem(3, QString::number(price, 'f', 2));
        setItem(4, QString::number(quantity));
        setItem(5, QString::number(subtotal, 'f', 2));

        if (available >= 0 && quantity > available) {
            conflictHints << tr("商品 %1 当前仅剩 %2 件，购物车数量为 %3")
                                 .arg(name)
                                 .arg(available)
                                 .arg(quantity);
            const QString tooltip = tr("库存不足，建议调整数量");
            checkItem->setToolTip(tooltip);
            for (int col = 0; col < m_cartTable->columnCount(); ++col) {
                if (QTableWidgetItem *cell = m_cartTable->item(row, col)) {
                    cell->setBackground(QColor(255, 235, 230));
                    cell->setToolTip(tooltip);
                }
            }
        }

        total += subtotal;
    }

    QJsonValue discountVal = JsonUtils::extract(doc, QStringLiteral("data.discount"));
    if (discountVal.isDouble()) {
        discount = discountVal.toDouble();
    }
    QJsonValue payableVal = JsonUtils::extract(doc, QStringLiteral("data.final_total"));
    if (payableVal.isDouble()) {
        payable = payableVal.toDouble();
    } else {
        QJsonValue totals = JsonUtils::extract(doc, QStringLiteral("data.total"));
        if (totals.isObject()) {
            const QJsonObject totalObj = totals.toObject();
            total = JsonUtils::asDouble(totalObj.value(QStringLiteral("original_total")), total);
            discount = JsonUtils::asDouble(totalObj.value(QStringLiteral("discount")), discount);
            payable = JsonUtils::asDouble(totalObj.value(QStringLiteral("payable")), total - discount);
        } else if (payable == 0.0) {
            payable = total - discount;
        }
    }

    m_lastCartTotal = total;
    m_summaryLabel->setText(tr("合计: %1 元 | 优惠: %2 元 | 应付: %3 元")
                                .arg(total, 0, 'f', 2)
                                .arg(discount, 0, 'f', 2)
                                .arg(payable, 0, 'f', 2));
    updateSummaryPreview();

    if (m_warningLabel) {
        if (!conflictHints.isEmpty()) {
            m_warningLabel->setText(conflictHints.join('\n'));
            m_warningLabel->setVisible(true);
        } else {
            m_warningLabel->clear();
            m_warningLabel->setVisible(false);
        }
    }

    if (!items.isEmpty()) {
        m_cartTable->selectRow(0);
    } else {
        m_detailView->clear();
        if (m_warningLabel) {
            m_warningLabel->clear();
            m_warningLabel->setVisible(false);
        }
        m_lastCartTotal = 0.0;
    }

    m_detailView->setPlainText(JsonUtils::pretty(doc));
    m_updatingTable = false;
}

void CartTab::populateAddresses(const QJsonDocument &doc)
{
    QJsonArray addresses = extractAddresses(doc);
    m_addresses.clear();
    m_addressCombo->clear();

    for (const QJsonValue &value : addresses) {
        QJsonObject obj = value.toObject();
        AddressRecord record;
        record.id = readAddressId(obj);
        record.display = formatAddressDisplay(obj);
        record.raw = obj;
        if (record.id >= 0) {
            m_addresses.push_back(record);
            m_addressCombo->addItem(record.display, record.id);
        }
    }

    if (m_addresses.isEmpty()) {
        m_addressCombo->addItem(tr("尚无地址，请创建"), -1);
    }
}

qlonglong CartTab::selectedProductId() const
{
    QJsonObject obj = selectedCartItem();
    return readProductId(obj);
}

QJsonObject CartTab::selectedCartItem() const
{
    const int row = m_cartTable->currentRow();
    if (row < 0) {
        return {};
    }
    QTableWidgetItem *item = m_cartTable->item(row, 1);
    if (!item) {
        return {};
    }
    const QString payload = item->data(Qt::UserRole).toString();
    return QJsonDocument::fromJson(payload.toUtf8()).object();
}

AddressRecord CartTab::currentAddress() const
{
    const int index = m_addressCombo->currentIndex();
    if (index < 0 || index >= m_addresses.size()) {
        return {};
    }
    return m_addresses.at(index);
}

void CartTab::setLoggedIn(bool loggedIn)
{
    m_loggedIn = loggedIn;
    m_cartTable->setEnabled(loggedIn);
    m_addressCombo->setEnabled(loggedIn);
    m_couponEdit->setEnabled(loggedIn);
    m_remarkEdit->setEnabled(loggedIn);
    m_quantitySpin->setEnabled(loggedIn);
}

void CartTab::handleItemChanged(QTableWidgetItem *item)
{
    if (!item || m_updatingTable) return;
    if (item->column() != 0) return; // 仅处理复选列
    const int row = item->row();
    QTableWidgetItem *idItem = m_cartTable->item(row, 1);
    if (!idItem) return;
    const QString payload = idItem->data(Qt::UserRole).toString();
    const QJsonObject obj = QJsonDocument::fromJson(payload.toUtf8()).object();
    const qlonglong productId = readProductId(obj);
    if (productId < 0) return;

    const bool checked = (item->checkState() == Qt::Checked);
    const QString command = QStringLiteral("SELECT_CART_ITEM %1 %2").arg(productId).arg(checked ? QStringLiteral("true") : QStringLiteral("false"));
    sendCartCommand(command, checked ? tr("选择商品") : tr("取消选择"), [this](const QJsonDocument &doc){ Q_UNUSED(doc); refreshCart(); });
}

void CartTab::handleToggleAllSelect(bool select)
{
    if (!m_loggedIn) { emit statusMessage(tr("请先登录"), false); return; }
    const QString command = QStringLiteral("SELECT_CART_ALL %1").arg(select ? QStringLiteral("true") : QStringLiteral("false"));
    sendCartCommand(command, select ? tr("全选") : tr("全不选"), [this](const QJsonDocument &doc){ Q_UNUSED(doc); refreshCart(); });
}

void CartTab::handleCouponSelectionChanged(int /*index*/)
{
    updateSummaryPreview();
    updateCouponInfoHint();
}

void CartTab::updateSummaryPreview()
{
    // 本地仅用于展示预估优惠，不改变服务端最终计算；优惠券类型简单识别：
    // CODE 形如 P10 / P15 -> 百分比；FIX10 -> 固定金额；MIN50_5 -> 满50减5
    if (m_lastCartTotal <= 0.0) {
        return;
    }
    const QString selCode = m_couponCombo->currentData().toString();
    if (selCode.isEmpty()) {
        return; // 不使用
    }

    const double base = m_lastCartTotal;
    double previewDiscount = 0.0;
    const QString normalized = selCode.trimmed().toUpper();

    if (normalized.startsWith(QStringLiteral("P")) && normalized.length() <= 4) {
        bool ok = false;
        const int pct = normalized.mid(1).toInt(&ok);
        if (ok && pct > 0 && pct <= 90) {
            previewDiscount = base * pct / 100.0;
        }
    } else if (normalized.startsWith(QStringLiteral("FIX"))) {
        bool ok = false;
        const double value = normalized.mid(3).toDouble(&ok);
        if (ok) {
            previewDiscount = value;
        }
    } else if (normalized.startsWith(QStringLiteral("MIN"))) {
        const QString rest = normalized.mid(3); // 形如 100_20
        const QStringList parts = rest.split('_');
        if (parts.size() == 2) {
            bool okNeed = false;
            bool okOff = false;
            const double need = parts.at(0).toDouble(&okNeed);
            const double off = parts.at(1).toDouble(&okOff);
            if (okNeed && okOff && base >= need) {
                previewDiscount = off;
            }
        }
    }

    if (previewDiscount < 0.0) {
        previewDiscount = 0.0;
    }
    if (previewDiscount > base) {
        previewDiscount = base;
    }

    const double payable = base - previewDiscount;

    // 附加 (预估优惠...) 文本提示，不覆盖服务端真实折扣
    QString current = m_summaryLabel->text();
    const int idx = current.indexOf(QStringLiteral("预估优惠"));
    if (idx >= 0) {
        current = current.left(idx).trimmed();
    }
    current += tr("  (预估优惠:%1 应付:%2)")
                   .arg(previewDiscount, 0, 'f', 2)
                   .arg(payable, 0, 'f', 2);
    m_summaryLabel->setText(current);
}

void CartTab::updateCouponInfoHint()
{
    if (!m_couponInfoLabel) {
        return;
    }
    if (!m_loggedIn) {
        m_couponInfoLabel->setText(tr("优惠券：请登录后查看"));
        return;
    }
    if (!m_couponCombo) {
        m_couponInfoLabel->setText(tr("优惠券：暂无可用"));
        return;
    }

    const int index = m_couponCombo->currentIndex();
    if (index <= 0) {
        QString text = tr("优惠券：未使用");
        if (m_couponCount > 0) {
            text += tr("（可用 %1 张）").arg(m_couponCount);
        }
        m_couponInfoLabel->setText(text);
        return;
    }

    const QString rawMeta = m_couponCombo->itemData(index, Qt::UserRole + 1).toString();
    const QString code = m_couponCombo->itemData(index).toString();
    QStringList lines;
    QJsonObject meta;
    if (!rawMeta.isEmpty()) {
        QJsonParseError parseError {};
        const QJsonDocument doc = QJsonDocument::fromJson(rawMeta.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            meta = doc.object();
        }
    }

    auto stringOr = [](const QJsonObject &obj, const QStringList &keys) -> QString {
        for (const QString &key : keys) {
            const QJsonValue val = obj.value(key);
            if (val.isString()) {
                const QString text = val.toString();
                if (!text.isEmpty()) {
                    return text;
                }
            }
        }
        return {};
    };

    auto readDouble = [](const QJsonObject &obj, const QStringList &keys, double &out) -> bool {
        for (const QString &key : keys) {
            const QJsonValue val = obj.value(key);
            if (val.isDouble()) {
                out = val.toDouble();
                return true;
            }
            if (val.isString()) {
                bool ok = false;
                const double parsed = val.toString().toDouble(&ok);
                if (ok) {
                    out = parsed;
                    return true;
                }
            }
        }
        return false;
    };

    QString title = stringOr(meta, {QStringLiteral("name"), QStringLiteral("title"), QStringLiteral("description")});
    if (title.isEmpty()) {
        title = m_couponCombo->currentText();
    }
    QString header;
    if (!title.isEmpty() && !code.isEmpty()) {
        header = tr("优惠券：%1 (%2)").arg(title, code);
    } else if (!title.isEmpty()) {
        header = tr("优惠券：%1").arg(title);
    } else if (!code.isEmpty()) {
        header = tr("优惠券：%1").arg(code);
    } else {
        header = tr("优惠券：详情");
    }
    lines << header;

    double fixedOff = 0.0;
    if (readDouble(meta, {QStringLiteral("discount_amount"), QStringLiteral("amount"), QStringLiteral("value"), QStringLiteral("off"), QStringLiteral("reduction")}, fixedOff) && fixedOff > 0.0) {
        lines << tr("立减 %1 元").arg(fixedOff, 0, 'f', 2);
    }

    double percentage = 0.0;
    if (readDouble(meta, {QStringLiteral("percentage"), QStringLiteral("percent"), QStringLiteral("percent_off"), QStringLiteral("discount_percent")}, percentage) && percentage > 0.0) {
        if (percentage <= 1.0) {
            percentage *= 100.0;
        }
        lines << tr("折扣 %1%%").arg(percentage, 0, 'f', 0);
    }

    double threshold = 0.0;
    if (readDouble(meta, {QStringLiteral("min_spend"), QStringLiteral("minimum_spend"), QStringLiteral("minimum_amount"), QStringLiteral("min_order"), QStringLiteral("threshold"), QStringLiteral("min_amount")}, threshold) && threshold > 0.0) {
        lines << tr("满 %1 元可用").arg(threshold, 0, 'f', 2);
    }

    const QString expire = stringOr(meta, {QStringLiteral("expires_at"), QStringLiteral("expire_at"), QStringLiteral("expiry"), QStringLiteral("expiry_date"), QStringLiteral("valid_until"), QStringLiteral("end_time"), QStringLiteral("end_at")});
    if (!expire.isEmpty()) {
        lines << tr("有效期至 %1").arg(expire);
    }

    const QString status = stringOr(meta, {QStringLiteral("status"), QStringLiteral("state"), QStringLiteral("usage_status")});
    if (!status.isEmpty()) {
        lines << tr("状态：%1").arg(status);
    }

    if (lines.size() == 1) {
        // 只有头部行，补充原始下拉文本
        lines << tr("说明：%1").arg(m_couponCombo->currentText());
    }

    if (m_couponCount > 0) {
        lines << tr("共可用优惠券：%1 张").arg(m_couponCount);
    }

    m_couponInfoLabel->setText(lines.join('\n'));
}
