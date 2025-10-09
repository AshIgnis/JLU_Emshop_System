#include "ui/tabs/ProductsTab.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "utils/JsonUtils.h"

#include <QBrush>
#include <QColor>
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
QJsonArray extractProductArray(const QJsonDocument &doc)
{
    static const QStringList candidatePaths = {
        QStringLiteral("data.products"),
        QStringLiteral("data.items"),
        QStringLiteral("products"),
        QStringLiteral("items"),
        QStringLiteral("data.list")
    };

    for (const auto &path : candidatePaths) {
        QJsonValue val = JsonUtils::extract(doc, path);
        if (val.isArray()) {
            return val.toArray();
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
        QStringLiteral("id"),
        QStringLiteral("productId")
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
    return QObject::tr("未知商品");
}

QString readCategory(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("category"),
        QStringLiteral("category_name")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toString();
        }
    }
    return QObject::tr("默认");
}

int readStock(const QJsonObject &obj)
{
    static const QStringList keys = {
        QStringLiteral("stock"),
        QStringLiteral("inventory"),
        QStringLiteral("quantity")
    };
    for (const auto &key : keys) {
        if (obj.contains(key)) {
            return obj.value(key).toInt();
        }
    }
    return 0;
}

} // namespace

ProductsTab::ProductsTab(ApplicationContext &context, QWidget *parent)
    : QWidget(parent)
    , m_context(context)
{
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true);
    m_categoryCombo->addItem(tr("全部"), QStringLiteral("all"));
    m_categoryCombo->addItem(tr("手机/数码"), QStringLiteral("electronics"));
    m_categoryCombo->addItem(tr("家居家电"), QStringLiteral("home"));
    m_categoryCombo->addItem(tr("服饰箱包"), QStringLiteral("fashion"));

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("关键字搜索"));

    m_pageSpin = new QSpinBox(this);
    m_pageSpin->setRange(1, 999);
    m_pageSpin->setValue(1);

    m_pageSizeSpin = new QSpinBox(this);
    m_pageSizeSpin->setRange(1, 200);
    m_pageSizeSpin->setValue(20);

    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(1, 99);
    m_quantitySpin->setValue(1);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({tr("商品ID"), tr("名称"), tr("分类"), tr("价格"), tr("库存"), tr("描述")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    m_detailView = new QPlainTextEdit(this);
    m_detailView->setReadOnly(true);
    m_detailView->setMinimumHeight(180);

    auto *filterLayout = new QFormLayout;
    filterLayout->addRow(tr("分类"), m_categoryCombo);
    filterLayout->addRow(tr("关键字"), m_searchEdit);
    filterLayout->addRow(tr("页码"), m_pageSpin);
    filterLayout->addRow(tr("每页数量"), m_pageSizeSpin);
    filterLayout->addRow(tr("购买数量"), m_quantitySpin);

    auto *refreshButton = new QPushButton(tr("刷新"), this);
    auto *searchButton = new QPushButton(tr("搜索"), this);
    auto *addCartButton = new QPushButton(tr("加入购物车"), this);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(addCartButton);
    buttonLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(filterLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_table, 1);
    layout->addWidget(new QLabel(tr("响应详情"), this));
    layout->addWidget(m_detailView);

    connect(refreshButton, &QPushButton::clicked, this, &ProductsTab::refreshProducts);
    connect(searchButton, &QPushButton::clicked, this, &ProductsTab::executeSearch);
    connect(addCartButton, &QPushButton::clicked, this, &ProductsTab::addSelectedToCart);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &ProductsTab::updateDetailView);

    refreshProducts();
}

void ProductsTab::handleSessionChanged(const UserSession &session)
{
    m_isLoggedIn = session.isValid();
}

void ProductsTab::refreshProducts()
{
    const QString category = m_categoryCombo->currentData().toString().isEmpty()
                                 ? m_categoryCombo->currentText().trimmed()
                                 : m_categoryCombo->currentData().toString();
    const int page = m_pageSpin->value();
    const int pageSize = m_pageSizeSpin->value();

    QString normalizedCategory = category.isEmpty() ? QStringLiteral("all") : category;
    QString command = QStringLiteral("GET_PRODUCTS %1 %2 %3")
                          .arg(normalizedCategory)
                          .arg(page)
                          .arg(pageSize);
    requestProducts(command, tr("获取商品列表"));
}

void ProductsTab::executeSearch()
{
    const QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        refreshProducts();
        return;
    }

    const int page = m_pageSpin->value();
    const int pageSize = m_pageSizeSpin->value();
    QString sortBy = QStringLiteral("id");
    double minPrice = 0.0;
    double maxPrice = 99999.0;

    QString command = QStringLiteral("SEARCH_PRODUCTS %1 %2 %3 %4 %5 %6")
                          .arg(keyword)
                          .arg(page)
                          .arg(pageSize)
                          .arg(sortBy)
                          .arg(minPrice, 0, 'f', 2)
                          .arg(maxPrice, 0, 'f', 2);

    requestProducts(command, tr("搜索商品"));
}

void ProductsTab::requestProducts(const QString &command, const QString &actionLabel)
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        emit statusMessage(tr("尚未连接服务器，无法%1").arg(actionLabel), false);
        return;
    }

    QPointer<ProductsTab> guard(this);
    client->sendCommand(command,
                        [this, guard, actionLabel](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                emit statusMessage(tr("%1失败: %2").arg(actionLabel, error), false);
                                showResponseInDetail(actionLabel, response);
                                return;
                            }

                            if (!JsonUtils::isSuccess(doc)) {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) {
                                    msg = response;
                                }
                                emit statusMessage(tr("%1失败: %2").arg(actionLabel, msg), false);
                                showResponseInDetail(actionLabel, JsonUtils::pretty(doc));
                                return;
                            }

                            populateTable(doc);
                            emit statusMessage(tr("%1成功，加载 %2 个商品")
                                                   .arg(actionLabel)
                                                   .arg(m_table->rowCount()),
                                               true);
                            showResponseInDetail(actionLabel, JsonUtils::pretty(doc));
                        },
                        [this, guard, actionLabel](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("%1失败: %2").arg(actionLabel, error), false);
                            showResponseInDetail(actionLabel, error);
                        });
}

void ProductsTab::populateTable(const QJsonDocument &doc)
{
    QJsonArray products = extractProductArray(doc);
    m_table->setRowCount(products.size());
    m_rowIndex.clear();

    for (int row = 0; row < products.size(); ++row) {
        const QJsonValue value = products.at(row);
        const QJsonObject obj = value.toObject();

        qlonglong productId = readProductId(obj);
        QString name = readProductName(obj);
        QString category = readCategory(obj);
        double price = JsonUtils::asDouble(obj.value(QStringLiteral("price")), 0.0);
        int stock = readStock(obj);
        QString description = obj.value(QStringLiteral("description")).toString();

        auto setItem = [this, row](int column, const QString &text, const QJsonObject &payload) {
            auto *item = new QTableWidgetItem(text);
            item->setData(Qt::UserRole, QJsonDocument(payload).toJson(QJsonDocument::Compact));
            m_table->setItem(row, column, item);
        };

        setItem(0, QString::number(productId), obj);
        setItem(1, name, obj);
        setItem(2, category, obj);
        setItem(3, QString::number(price, 'f', 2), obj);
        setItem(4, QString::number(stock), obj);
        m_rowIndex[productId] = row;
        setItem(5, description, obj);

        if (stock <= 0) {
            for (int c=0;c<m_table->columnCount();++c) {
                QTableWidgetItem *it = m_table->item(row,c);
                if (it) {
                    it->setForeground(QBrush(QColor(160,160,160)));
                    it->setToolTip(tr("库存不足，无法加入购物车"));
                }
            }
        }
    }

    if (!products.isEmpty()) {
        m_table->selectRow(0);
    }
}

void ProductsTab::applyStockChanges(const QJsonArray &changes)
{
    if (changes.isEmpty()) return;
    for (const auto &val : changes) {
        QJsonObject o = val.toObject();
        qlonglong pid = JsonUtils::asLongLong(o.value("product_id"), -1);
        if (pid < 0 || !m_rowIndex.contains(pid)) continue;
        int row = m_rowIndex.value(pid);
        int remaining = o.value("remaining").toInt(-1);
        int deducted = o.value("deducted").toInt(0);
        if (row >=0 && row < m_table->rowCount() && remaining >= 0) {
            QTableWidgetItem *stockItem = m_table->item(row,4);
            if (stockItem) {
                stockItem->setText(QString::number(remaining));
                // 根据剩余库存调整样式
                if (remaining <= 0) {
                    for (int c=0;c<m_table->columnCount();++c) {
                        QTableWidgetItem *it = m_table->item(row,c);
                        if (it) {
                            it->setForeground(QBrush(QColor(160,160,160)));
                            it->setToolTip(tr("库存不足，无法加入购物车"));
                        }
                    }
                } else if (remaining < 5) {
                    stockItem->setForeground(QBrush(QColor(200,0,0)));
                    stockItem->setToolTip(tr("库存紧张 (剩 %1) - 刚扣减 %2").arg(remaining).arg(deducted));
                } else {
                    stockItem->setForeground(QBrush());
                    stockItem->setToolTip(tr("剩余 %1 (刚扣减 %2)").arg(remaining).arg(deducted));
                }
            }
        }
    }
}

void ProductsTab::addSelectedToCart()
{
    if (!m_isLoggedIn) {
        emit statusMessage(tr("请先登录后再加入购物车"), false);
        return;
    }

    const int row = m_table->currentRow();
    if (row < 0) {
        emit statusMessage(tr("请先选择商品"), false);
        return;
    }

    qlonglong productId = productIdFromRow(row);
    if (productId < 0) {
        emit statusMessage(tr("无法获取所选商品ID"), false);
        return;
    }

    const int quantity = m_quantitySpin->value();
    // 若库存为0禁止加入
    QTableWidgetItem *stockItem = m_table->item(row, 4);
    int currentStock = stockItem ? stockItem->text().toInt() : 0;
    if (currentStock <= 0) {
        emit statusMessage(tr("该商品库存不足，无法加入购物车"), false);
        return;
    }
    QString command = QStringLiteral("ADD_TO_CART %1 %2").arg(productId).arg(quantity);

    NetworkClient *client = m_context.networkClient();
    QPointer<ProductsTab> guard(this);
    client->sendCommand(command,
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (ok && JsonUtils::isSuccess(doc)) {
                                emit statusMessage(JsonUtils::message(doc).isEmpty() ? tr("加入购物车成功")
                                                                                       : JsonUtils::message(doc),
                                                   true);
                                emit cartChanged();
                            } else {
                                QString msg = ok ? JsonUtils::message(doc) : error;
                                if (msg.isEmpty()) {
                                    msg = response;
                                }
                                emit statusMessage(tr("加入购物车失败: %1").arg(msg), false);
                            }
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            emit statusMessage(tr("加入购物车失败: %1").arg(error), false);
                        });
}

void ProductsTab::updateDetailView()
{
    const int row = m_table->currentRow();
    if (row < 0) {
        m_detailView->clear();
        return;
    }

    QJsonObject obj = productObjectFromRow(row);
    if (obj.isEmpty()) {
        m_detailView->clear();
        return;
    }
    m_detailView->setPlainText(JsonUtils::pretty(QJsonDocument(obj)));
}

qlonglong ProductsTab::productIdFromRow(int row) const
{
    QJsonObject obj = productObjectFromRow(row);
    return readProductId(obj);
}

QJsonObject ProductsTab::productObjectFromRow(int row) const
{
    if (row < 0) {
        return {};
    }
    QTableWidgetItem *item = m_table->item(row, 0);
    if (!item) {
        return {};
    }
    const QString payload = item->data(Qt::UserRole).toString();
    QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    return doc.object();
}

void ProductsTab::showResponseInDetail(const QString &label, const QString &payload)
{
    m_lastRawResponse = payload;
    m_detailView->setPlainText(QStringLiteral("[%1]\n%2").arg(label, payload));
}
