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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 设置整体样式
    setStyleSheet(R"(
        QComboBox, QLineEdit {
            padding: 8px;
            border: 2px solid #ddd;
            border-radius: 6px;
            font-size: 14px;
        }
        QComboBox:focus, QLineEdit:focus {
            border-color: #4CAF50;
        }
        QTreeWidget {
            border: 1px solid #ddd;
            border-radius: 8px;
            selection-background-color: #e3f2fd;
            font-size: 13px;
        }
        QTreeWidget::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }
        QTreeWidget::item:selected {
            background-color: #e3f2fd;
            color: #1976d2;
        }
        QSpinBox {
            padding: 6px;
            border: 2px solid #ddd;
            border-radius: 6px;
            font-size: 14px;
        }
    )");
    
    // 标题栏
    QLabel *titleLabel = new QLabel("🛍️ 商品浏览", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 搜索和筛选栏
    QWidget *filterWidget = new QWidget(this);
    filterWidget->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border-radius: 10px;
            padding: 15px;
        }
    )");
    QHBoxLayout *searchLayout = new QHBoxLayout(filterWidget);
    
    QLabel *categoryLabel = new QLabel("📂 分类:", this);
    categoryLabel->setStyleSheet("font-weight: bold; color: #555;");
    
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setMinimumWidth(120);
    m_categoryCombo->addItem("🔍 全部分类", "all");
    m_categoryCombo->addItem("📱 电子产品", "electronics");
    m_categoryCombo->addItem("👕 服装服饰", "clothing");  
    m_categoryCombo->addItem("📚 图书文具", "books");
    m_categoryCombo->addItem("🏠 家居用品", "home");
    m_categoryCombo->addItem("🎮 娱乐休闲", "entertainment");
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("🔍 搜索商品名称、描述...");
    m_searchEdit->setMinimumWidth(300);
    
    m_searchButton = new QPushButton("🔍 搜索", this);
    m_searchButton->setStyleSheet("background-color: #2196F3;");
    
    m_refreshButton = new QPushButton("🔄 刷新", this);
    m_refreshButton->setStyleSheet("background-color: #4CAF50;");
    
    searchLayout->addWidget(categoryLabel);
    searchLayout->addWidget(m_categoryCombo);
    searchLayout->addSpacing(20);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_refreshButton);
    searchLayout->addStretch();
    
    mainLayout->addWidget(filterWidget);
    
    // 商品展示区域
    m_productTree = new QTreeWidget(this);
    m_productTree->setHeaderLabels(QStringList() << "商品ID" << "商品名称" << "价格(¥)" << "库存" << "分类" << "描述");
    
    // 设置列宽
    m_productTree->setColumnWidth(0, 80);   // ID
    m_productTree->setColumnWidth(1, 200);  // 名称
    m_productTree->setColumnWidth(2, 100);  // 价格
    m_productTree->setColumnWidth(3, 80);   // 库存
    m_productTree->setColumnWidth(4, 120);  // 分类
    
    m_productTree->header()->setStretchLastSection(true);
    m_productTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_productTree->setAlternatingRowColors(true);
    m_productTree->setRootIsDecorated(false);
    m_productTree->setSortingEnabled(true);
    
    mainLayout->addWidget(m_productTree);
    
    // 购物车操作区域
    QWidget *cartWidget = new QWidget(this);
    cartWidget->setStyleSheet(R"(
        QWidget {
            background-color: #fff3e0;
            border: 2px solid #ff9800;
            border-radius: 10px;
            padding: 15px;
        }
    )");
    
    QLabel *cartLabel = new QLabel("🛒 加入购物车:", this);
    cartLabel->setStyleSheet("font-weight: bold; color: #ff6f00; font-size: 14px;");
    
    QLabel *quantityLabel = new QLabel("数量:", this);
    quantityLabel->setStyleSheet("color: #555; font-weight: bold;");
    
    m_quantitySpinBox = new QSpinBox(this);
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(99);
    m_quantitySpinBox->setValue(1);
    m_quantitySpinBox->setMinimumWidth(80);
    
    m_addToCartButton = new QPushButton("🛒 加入购物车", this);
    m_addToCartButton->setStyleSheet("background-color: #ff9800; font-weight: bold; min-width: 120px;");
    
    QPushButton *viewDetailsButton = new QPushButton("👁️ 查看详情", this);
    viewDetailsButton->setStyleSheet("background-color: #9c27b0; font-weight: bold; min-width: 120px;");
    
    actionLayout->addWidget(cartLabel);
    actionLayout->addSpacing(15);
    actionLayout->addWidget(quantityLabel);
    actionLayout->addWidget(m_quantitySpinBox);
    actionLayout->addSpacing(10);
    actionLayout->addWidget(m_addToCartButton);
    actionLayout->addWidget(viewDetailsButton);
    actionLayout->addStretch();
    
    mainLayout->addWidget(cartWidget);
    
    // 状态栏
    m_statusLabel = new QLabel("📋 准备就绪", this);
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            background-color: #e8f5e8;
            border: 1px solid #4caf50;
            border-radius: 6px;
            padding: 8px 15px;
            color: #2e7d32;
            font-weight: bold;
        }
    )");
    
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_searchButton, &QPushButton::clicked, this, &ProductListWidget::onSearchClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &ProductListWidget::refreshProducts);
    connect(m_addToCartButton, &QPushButton::clicked, this, &ProductListWidget::onAddToCartClicked);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &ProductListWidget::onSearchClicked);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ProductListWidget::refreshProducts);
            
    // 查看详情功能
    connect(viewDetailsButton, &QPushButton::clicked, [this]() {
        QTreeWidgetItem *current = m_productTree->currentItem();
        if (!current) {
            QMessageBox::information(this, "提示", "请先选择一个商品");
            return;
        }
        
        QString productName = current->text(1);
        QString price = current->text(2);
        QString stock = current->text(3);
        QString category = current->text(4);
        QString description = current->text(5);
        
        QString details = QString(
            "📦 商品详情\n\n"
            "🏷️ 商品名称：%1\n"
            "💰 价格：%2 元\n"
            "📦 库存：%3 件\n"
            "📂 分类：%4\n"
            "📝 描述：%5"
        ).arg(productName, price, stock, category, description);
        
        QMessageBox::information(this, "商品详情", details);
    });
    
    // 双击商品查看详情
    connect(m_productTree, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item) {
        if (item) {
            QString productName = item->text(1);
            QString price = item->text(2);
            QString stock = item->text(3);
            QString category = item->text(4);
            QString description = item->text(5);
            
            QString details = QString(
                "📦 商品详情\n\n"
                "🏷️ 商品名称：%1\n"
                "💰 价格：%2 元\n"
                "📦 库存：%3 件\n" 
                "📂 分类：%4\n"
                "📝 描述：%5\n\n"
                "💡 提示：您可以设置数量后点击"加入购物车"按钮"
            ).arg(productName, price, stock, category, description);
            
            QMessageBox::information(this, "商品详情", details);
        }
    });
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