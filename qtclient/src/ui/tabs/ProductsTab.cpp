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
#include <QTimer>

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
    
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true);
    m_categoryCombo->addItem(tr("全部"), QStringLiteral("all"));
    // 使用数据库中的实际分类ID (从emshop_database_init.sql)
    m_categoryCombo->addItem(tr("电子数码"), QStringLiteral("1"));  // category_id=1
    m_categoryCombo->addItem(tr("手机通讯"), QStringLiteral("2"));  // category_id=2
    m_categoryCombo->addItem(tr("电脑办公"), QStringLiteral("3"));  // category_id=3
    m_categoryCombo->addItem(tr("家用电器"), QStringLiteral("4"));  // category_id=4
    m_categoryCombo->addItem(tr("服装鞋帽"), QStringLiteral("5"));  // category_id=5
    m_categoryCombo->addItem(tr("运动户外"), QStringLiteral("8"));  // category_id=8
    m_categoryCombo->addItem(tr("图书音像"), QStringLiteral("9"));  // category_id=9
    m_categoryCombo->addItem(tr("食品生鲜"), QStringLiteral("10")); // category_id=10

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
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({tr("商品ID"), tr("名称"), tr("分类"), tr("价格"), tr("库存")});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(R"(
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

    // 输入框样式 - 确保文字为深色，包括下拉列表
    QString inputStyle = R"(
        QLineEdit, QSpinBox, QComboBox {
            background-color: #ffffff;
            color: #2c3e50;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 8px 12px;
            selection-background-color: #3498db;
            selection-color: #ffffff;
            font-size: 10pt;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #3498db;
            background-color: #f8f9fa;
            color: #2c3e50;
        }
        QLineEdit::placeholder {
            color: #95a5a6;
        }
        QComboBox::drop-down {
            border: none;
            background-color: #f8f9fa;
            border-radius: 4px;
            width: 30px;
            margin-right: 2px;
        }
        QComboBox::drop-down:hover {
            background-color: #e0e6ed;
        }
        QComboBox::down-arrow {
            image: none;
            width: 0;
            height: 0;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 8px solid #2c3e50;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #2c3e50;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 4px;
            selection-background-color: #3498db;
            selection-color: #ffffff;
            outline: none;
        }
        QComboBox QAbstractItemView::item {
            padding: 8px 12px;
            color: #2c3e50;
            min-height: 25px;
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: #3498db;
            color: #ffffff;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #ecf0f1;
            color: #2c3e50;
        }
        QSpinBox::up-button, QSpinBox::down-button {
            background-color: #f8f9fa;
            border: none;
            width: 20px;
        }
        QSpinBox::up-button:hover, QSpinBox::down-button:hover {
            background-color: #e0e6ed;
        }
        QSpinBox::up-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 4px solid #2c3e50;
        }
        QSpinBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 4px solid #2c3e50;
        }
    )";
    
    m_categoryCombo->setStyleSheet(inputStyle);
    m_searchEdit->setStyleSheet(inputStyle);
    m_pageSpin->setStyleSheet(inputStyle);
    m_pageSizeSpin->setStyleSheet(inputStyle);
    m_quantitySpin->setStyleSheet(inputStyle);

    auto *filterLayout = new QFormLayout;
    filterLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    filterLayout->setHorizontalSpacing(12);
    filterLayout->setVerticalSpacing(10);
    
    // 显式创建标签并设置深色文字样式
    QString labelStyle = R"(
        QLabel {
            color: #2c3e50;
            font-weight: 600;
            font-size: 10pt;
        }
    )";
    
    auto *categoryLabel = new QLabel(tr("🏷️ 分类"), this);
    categoryLabel->setStyleSheet(labelStyle);
    filterLayout->addRow(categoryLabel, m_categoryCombo);
    
    auto *searchLabel = new QLabel(tr("🔍 关键字"), this);
    searchLabel->setStyleSheet(labelStyle);
    filterLayout->addRow(searchLabel, m_searchEdit);
    
    auto *pageLabel = new QLabel(tr("📄 页码"), this);
    pageLabel->setStyleSheet(labelStyle);
    filterLayout->addRow(pageLabel, m_pageSpin);
    
    auto *pageSizeLabel = new QLabel(tr("📊 每页数量"), this);
    pageSizeLabel->setStyleSheet(labelStyle);
    filterLayout->addRow(pageSizeLabel, m_pageSizeSpin);
    
    auto *quantityLabel = new QLabel(tr("🛒 购买数量"), this);
    quantityLabel->setStyleSheet(labelStyle);
    filterLayout->addRow(quantityLabel, m_quantitySpin);

    auto *refreshButton = new QPushButton(tr("🔄 刷新"), this);
    auto *searchButton = new QPushButton(tr("🔍 搜索"), this);
    auto *addCartButton = new QPushButton(tr("🛒 加入购物车"), this);
    
    // 按钮样式
    refreshButton->setStyleSheet(R"(
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
    )");
    
    searchButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: 600;
            font-size: 10pt;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5568d3, stop:1 #6a3e91);
        }
    )");
    
    addCartButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #11998e, stop:1 #38ef7d);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: 600;
            font-size: 10pt;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0f8a7e, stop:1 #32d86f);
        }
    )");

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(searchButton);
    buttonLayout->addWidget(addCartButton);
    buttonLayout->addStretch();

    auto *detailLabel = new QLabel(tr("📋 响应详情"), this);
    detailLabel->setStyleSheet(R"(
        QLabel {
            font-weight: 600;
            font-size: 11pt;
            color: #2c3e50;
            padding: 8px 0px;
        }
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);
    layout->addLayout(filterLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_table, 1);
    layout->addWidget(detailLabel);
    layout->addWidget(m_detailView);

    connect(refreshButton, &QPushButton::clicked, this, &ProductsTab::refreshProducts);
    connect(searchButton, &QPushButton::clicked, this, &ProductsTab::executeSearch);
    connect(addCartButton, &QPushButton::clicked, this, &ProductsTab::addSelectedToCart);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &ProductsTab::updateDetailView);

    m_stockRefreshTimer = new QTimer(this);
    m_stockRefreshTimer->setSingleShot(true);
    m_stockRefreshTimer->setInterval(1200);
    connect(m_stockRefreshTimer, &QTimer::timeout, this, [this]() {
        if (!m_missingProductIds.isEmpty()) {
            emit statusMessage(tr("检测到其他商品库存变动，正在刷新列表"), true);
            refreshProducts();
            m_missingProductIds.clear();
        }
    });

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
        m_missingProductIds.remove(productId);

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
    bool needsRefresh = false;
    for (const auto &val : changes) {
        QJsonObject o = val.toObject();
        qlonglong pid = JsonUtils::asLongLong(o.value("product_id"), -1);
        if (pid < 0) {
            continue;
        }
        if (!m_rowIndex.contains(pid)) {
            m_missingProductIds.insert(pid);
            needsRefresh = true;
            continue;
        }
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

    if (needsRefresh && m_stockRefreshTimer && !m_stockRefreshTimer->isActive()) {
        m_stockRefreshTimer->start();
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
