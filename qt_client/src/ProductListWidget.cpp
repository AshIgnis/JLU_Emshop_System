#include "ProductListWidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QTimer>

ProductListWidget::ProductListWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
    , m_selectedProductId(-1)
{
    setupUI();
}

void ProductListWidget::setEmshopClient(EmshopClient *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &EmshopClient::productsReceived,
                this, &ProductListWidget::onProductsReceived);
        connect(m_client, &EmshopClient::searchResultsReceived,
                this, &ProductListWidget::onSearchResultsReceived);
        connect(m_client, &EmshopClient::cartUpdated,
                this, &ProductListWidget::onCartUpdated);
    }
}

void ProductListWidget::refreshProducts()
{
    if (m_client && m_client->isAuthenticated()) {
        QString category = m_categoryCombo->currentData().toString();
        m_client->getProducts(category, 1, 50); // 获取前50个产品
        m_statusLabel->setText("正在加载商品...");
    }
}

void ProductListWidget::onSearchClicked()
{
    if (!m_client || !m_client->isAuthenticated()) {
        return;
    }
    
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        refreshProducts();
        return;
    }
    
    m_client->searchProducts(keyword, 1, 50);
    m_statusLabel->setText("正在搜索...");
}

void ProductListWidget::onProductsReceived(const QJsonObject &data)
{
    if (data.contains("items")) {
        QJsonArray products = data["items"].toArray();
        loadProductsToTree(products);
        
        int total = data["total"].toInt();
        m_statusLabel->setText(QString("共找到 %1 个商品").arg(total));
    }
}

void ProductListWidget::onSearchResultsReceived(const QJsonObject &data)
{
    onProductsReceived(data); // 搜索结果和商品列表格式相同
}

void ProductListWidget::onAddToCartClicked()
{
    QTreeWidgetItem *current = m_productTree->currentItem();
    if (!current) {
        QMessageBox::warning(this, "选择商品", "请先选择一个商品");
        return;
    }
    
    if (!m_client || !m_client->isAuthenticated()) {
        return;
    }
    
    qint64 productId = current->data(0, Qt::UserRole).toLongLong();
    int quantity = m_quantitySpinBox->value();
    
    m_client->addToCart(productId, quantity);
    m_statusLabel->setText("正在添加到购物车...");
}

void ProductListWidget::onCartUpdated(const QJsonObject &data)
{
    Q_UNUSED(data)
    m_statusLabel->setText("商品已添加到购物车");
    
    // 3秒后清除状态信息
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText("准备就绪");
    });
}

void ProductListWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout();
    
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("全部", "all");
    m_categoryCombo->addItem("电子产品", "electronics");
    m_categoryCombo->addItem("服装", "clothing");
    m_categoryCombo->addItem("书籍", "books");
    m_categoryCombo->addItem("家居", "home");
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("搜索商品...");
    
    m_searchButton = new QPushButton("搜索", this);
    m_refreshButton = new QPushButton("刷新", this);
    
    searchLayout->addWidget(new QLabel("分类:", this));
    searchLayout->addWidget(m_categoryCombo);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_refreshButton);
    searchLayout->addStretch();
    
    // 商品列表
    m_productTree = new QTreeWidget(this);
    m_productTree->setHeaderLabels(QStringList() << "ID" << "商品名称" << "价格" << "库存" << "描述");
    m_productTree->header()->setStretchLastSection(true);
    m_productTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_productTree->setAlternatingRowColors(true);
    
    // 操作栏
    QHBoxLayout *actionLayout = new QHBoxLayout();
    
    actionLayout->addWidget(new QLabel("数量:", this));
    m_quantitySpinBox = new QSpinBox(this);
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(99);
    m_quantitySpinBox->setValue(1);
    
    m_addToCartButton = new QPushButton("添加到购物车", this);
    
    actionLayout->addWidget(m_quantitySpinBox);
    actionLayout->addWidget(m_addToCartButton);
    actionLayout->addStretch();
    
    // 状态栏
    m_statusLabel = new QLabel("准备就绪", this);
    
    // 组装布局
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(m_productTree);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_searchButton, &QPushButton::clicked, this, &ProductListWidget::onSearchClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &ProductListWidget::refreshProducts);
    connect(m_addToCartButton, &QPushButton::clicked, this, &ProductListWidget::onAddToCartClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ProductListWidget::onSearchClicked);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProductListWidget::refreshProducts);
}

void ProductListWidget::loadProductsToTree(const QJsonArray &products)
{
    m_productTree->clear();
    
    for (const QJsonValue &value : products) {
        QJsonObject product = value.toObject();
        QTreeWidgetItem *item = createProductItem(product);
        m_productTree->addTopLevelItem(item);
    }
    
    // 调整列宽
    for (int i = 0; i < m_productTree->columnCount(); ++i) {
        m_productTree->resizeColumnToContents(i);
    }
}

QTreeWidgetItem *ProductListWidget::createProductItem(const QJsonObject &product)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    
    qint64 id = product["id"].toVariant().toLongLong();
    QString name = product["name"].toString();
    double price = product["price"].toDouble();
    int stock = product["stock"].toInt();
    QString description = product["description"].toString();
    
    item->setText(0, QString::number(id));
    item->setText(1, name);
    item->setText(2, QString("¥%1").arg(price, 0, 'f', 2));
    item->setText(3, QString::number(stock));
    item->setText(4, description);
    
    // 存储产品ID用于后续操作
    item->setData(0, Qt::UserRole, id);
    
    // 根据库存设置颜色
    if (stock == 0) {
        item->setForeground(0, QBrush(Qt::red));
        item->setForeground(1, QBrush(Qt::red));
    } else if (stock < 10) {
        item->setForeground(0, QBrush(Qt::darkYellow));
        item->setForeground(1, QBrush(Qt::darkYellow));
    }
    
    return item;
}