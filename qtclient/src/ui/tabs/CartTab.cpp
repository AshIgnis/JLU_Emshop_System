#include "ui/tabs/CartTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "ui/dialogs/AddressDialog.h"
#include "utils/JsonUtils.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
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
    m_cartTable->setColumnCount(5);
    m_cartTable->setHorizontalHeaderLabels({tr("商品ID"), tr("商品"), tr("单价"), tr("数量"), tr("小计")});
    m_cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cartTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(150);

    m_summaryLabel = new QLabel(tr("购物车为空"), this);

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
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(clearButton);
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
    layout->addLayout(addressLayout);
    layout->addLayout(orderLayout);
    layout->addWidget(createOrderButton);

    connect(refreshButton, &QPushButton::clicked, this, &CartTab::refreshCart);
    connect(updateButton, &QPushButton::clicked, this, &CartTab::updateItemQuantity);
    connect(removeButton, &QPushButton::clicked, this, &CartTab::removeSelectedItem);
    connect(clearButton, &QPushButton::clicked, this, &CartTab::clearCart);
    connect(refreshAddrButton, &QPushButton::clicked, this, &CartTab::refreshAddresses);
    connect(addAddressButton, &QPushButton::clicked, this, &CartTab::addNewAddress);
    connect(m_couponCombo, &QComboBox::activated, this, [this](int){ /* no-op placeholder */ });
    connect(createOrderButton, &QPushButton::clicked, this, &CartTab::createOrder);
    connect(m_cartTable, &QTableWidget::itemSelectionChanged, this, &CartTab::updateDetailView);

    setLoggedIn(false);
}

void CartTab::handleSessionChanged(const UserSession &session)
{
    setLoggedIn(session.isValid());
    if (session.isValid()) {
        refreshCart();
        refreshAddresses();
        refreshUserCoupons();
    } else {
        m_cartTable->setRowCount(0);
        m_addresses.clear();
        m_addressCombo->clear();
        m_summaryLabel->setText(tr("购物车为空"));
        m_detailView->clear();
        m_couponCombo->clear();
        m_couponCombo->addItem(tr("不使用优惠券"), QString());
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

    // 如果选中了某个具体的购物车条目，则只对该条目下单；否则对已选中(服务器侧selected=1)的全部条目下单
    qlonglong productId = selectedProductId();
    QString command;
    if (productId >= 0) {
        int quantity = readQuantity(selectedCartItem());
        command = QStringLiteral("CREATE_ORDER_ITEM %1 %2 %3 %4 %5")
                      .arg(productId)
                      .arg(quantity)
                      .arg(address.id)
                      .arg(couponArg)
                      .arg(quoteForCommand(remarkArg));
    } else {
        command = QStringLiteral("CREATE_ORDER %1 %2 %3")
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
                            emit statusMessage(tr("订单创建成功"), true);
                            // 下单成功后刷新购物车（已移除相应条目或清空）
                            refreshCart();
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
                                emit statusMessage(tr("刷新优惠券失败: %1").arg(ok? JsonUtils::message(doc) : error), false);
                                return;
                            }
                            // 期望数据路径 data 或 data.list / data.items
                            QJsonArray arr;
                            QJsonValue v = JsonUtils::extract(doc, QStringLiteral("data.coupons"));
                            if (v.isArray()) arr = v.toArray();
                            if (arr.isEmpty()) {
                                v = JsonUtils::extract(doc, QStringLiteral("data.list"));
                                if (v.isArray()) arr = v.toArray();
                            }
                            if (arr.isEmpty()) {
                                v = JsonUtils::extract(doc, QStringLiteral("data"));
                                if (v.isArray()) arr = v.toArray();
                            }
                            // 重建下拉
                            m_couponCombo->clear();
                            m_couponCombo->addItem(tr("不使用优惠券"), QString());
                            for (const QJsonValue &val : arr) {
                                QJsonObject o = val.toObject();
                                const QString code = o.value(QStringLiteral("code")).toString();
                                const QString name = o.value(QStringLiteral("name")).toString(code);
                                if (!code.isEmpty()) {
                                    m_couponCombo->addItem(QString("%1 (%2)").arg(name, code), code);
                                }
                            }
                            emit statusMessage(tr("已同步优惠券"), true);
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
                                        emit statusMessage(tr("刷新优惠券失败: %1").arg(ok2? JsonUtils::message(doc2) : err2), false);
                                        return;
                                    }
                                    QJsonArray arr; QJsonValue v = JsonUtils::extract(doc2, QStringLiteral("data.coupons"));
                                    if (v.isArray()) arr = v.toArray();
                                    if (arr.isEmpty()) { v = JsonUtils::extract(doc2, QStringLiteral("data")); if (v.isArray()) arr = v.toArray(); }
                                    m_couponCombo->clear(); m_couponCombo->addItem(tr("不使用优惠券"), QString());
                                    for (const QJsonValue &val : arr) {
                                        QJsonObject o = val.toObject();
                                        const QString code = o.value(QStringLiteral("code")).toString();
                                        const QString name = o.value(QStringLiteral("name")).toString(code);
                                        if (!code.isEmpty()) m_couponCombo->addItem(QString("%1 (%2)").arg(name, code), code);
                                    }
                                    emit statusMessage(tr("已同步优惠券"), true);
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
    QJsonArray items = extractCartItems(doc);
    m_cartTable->setRowCount(items.size());

    double total = 0.0;
    double discount = 0.0;
    double payable = 0.0;

    for (int row = 0; row < items.size(); ++row) {
        const QJsonObject obj = items.at(row).toObject();
        const qlonglong productId = readProductId(obj);
        const QString name = readProductName(obj);
        const double price = readPrice(obj);
        const int quantity = readQuantity(obj);
        const double subtotal = readSubtotal(obj);

        auto setItem = [this, row, obj](int column, const QString &text) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(obj).toJson(QJsonDocument::Compact));
            m_cartTable->setItem(row, column, item);
        };

        setItem(0, QString::number(productId));
        setItem(1, name);
        setItem(2, QString::number(price, 'f', 2));
        setItem(3, QString::number(quantity));
        setItem(4, QString::number(subtotal, 'f', 2));

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

    m_summaryLabel->setText(tr("合计: %1 元 | 优惠: %2 元 | 应付: %3 元")
                                .arg(total, 0, 'f', 2)
                                .arg(discount, 0, 'f', 2)
                                .arg(payable, 0, 'f', 2));

    if (!items.isEmpty()) {
        m_cartTable->selectRow(0);
    } else {
        m_detailView->clear();
    }

    m_detailView->setPlainText(JsonUtils::pretty(doc));
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
    QTableWidgetItem *item = m_cartTable->item(row, 0);
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
