#include "CartWidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QJsonArray>
#include <QInputDialog>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
{
    setupUI();
}

void CartWidget::setEmshopClient(ClientAdapter *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &ClientAdapter::cartReceived,
                this, &CartWidget::onCartReceived);
    }
}

void CartWidget::refreshCart()
{
    if (m_client && m_client->isAuthenticated()) {
        m_client->getCart();
        m_statusLabel->setText("正在加载购物车...");
    }
}

void CartWidget::onCartReceived(const QJsonObject &data)
{
    loadCartItems(data);
    m_statusLabel->setText("购物车已更新");
}

void CartWidget::onRemoveFromCartClicked()
{
    QTreeWidgetItem *current = m_cartTree->currentItem();
    if (!current) {
        QMessageBox::warning(this, "选择商品", "请先选择要移除的商品");
        return;
    }
    
    if (!m_client || !m_client->isAuthenticated()) {
        return;
    }
    
    qint64 productId = current->data(0, Qt::UserRole).toLongLong();
    m_client->removeFromCart(productId);
    m_statusLabel->setText("正在移除商品...");
}

void CartWidget::onCheckoutClicked()
{
    QMessageBox::information(this, "结算", "结算功能尚未实现");
}

void CartWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 设置整体样式
    setStyleSheet(R"(
        QTreeWidget {
            border: 1px solid #ddd;
            border-radius: 8px;
            selection-background-color: #f3e5f5;
            font-size: 13px;
        }
        QTreeWidget::item {
            padding: 10px;
            border-bottom: 1px solid #f0f0f0;
        }
        QTreeWidget::item:selected {
            background-color: #f3e5f5;
            color: #7b1fa2;
        }
        QPushButton {
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 14px;
        }
    )");
    
    // 标题栏
    QLabel *titleLabel = new QLabel("🛒 我的购物车", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel);
    
    // 工具栏
    QWidget *toolWidget = new QWidget(this);
    toolWidget->setStyleSheet(R"(
        QWidget {
            background-color: #f8f9fa;
            border-radius: 10px;
            padding: 15px;
        }
    )");
    QHBoxLayout *toolLayout = new QHBoxLayout(toolWidget);
    
    m_refreshButton = new QPushButton("🔄 刷新购物车", this);
    m_refreshButton->setStyleSheet("background-color: #4CAF50;");
    
    QPushButton *clearCartButton = new QPushButton("🗑️ 清空购物车", this);
    clearCartButton->setStyleSheet("background-color: #f44336;");
    
    m_removeButton = new QPushButton("❌ 移除选中", this);
    m_removeButton->setStyleSheet("background-color: #ff9800;");
    
    QPushButton *updateQuantityButton = new QPushButton("📝 修改数量", this);
    updateQuantityButton->setStyleSheet("background-color: #2196f3;");
    
    m_checkoutButton = new QPushButton("💳 立即结算", this);
    m_checkoutButton->setStyleSheet("background-color: #9c27b0; font-size: 16px; min-width: 150px;");
    
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addWidget(m_removeButton);
    toolLayout->addWidget(updateQuantityButton);
    toolLayout->addWidget(clearCartButton);
    toolLayout->addStretch();
    toolLayout->addWidget(m_checkoutButton);
    
    mainLayout->addWidget(toolWidget);
    
    // 购物车列表
    m_cartTree = new QTreeWidget(this);
    m_cartTree->setHeaderLabels(QStringList() << "商品名称" << "单价(¥)" << "数量" << "小计(¥)" << "操作");
    
    // 设置列宽
    m_cartTree->setColumnWidth(0, 200);  // 商品名称
    m_cartTree->setColumnWidth(1, 100);  // 单价
    m_cartTree->setColumnWidth(2, 80);   // 数量
    m_cartTree->setColumnWidth(3, 100);  // 小计
    
    m_cartTree->header()->setStretchLastSection(true);
    m_cartTree->setAlternatingRowColors(true);
    m_cartTree->setRootIsDecorated(false);
    m_cartTree->setSelectionMode(QAbstractItemView::SingleSelection);
    
    mainLayout->addWidget(m_cartTree);
    
    // 总计和状态
    QWidget *summaryWidget = new QWidget(this);
    summaryWidget->setStyleSheet(R"(
        QWidget {
            background-color: #e8f5e8;
            border: 2px solid #4caf50;
            border-radius: 10px;
            padding: 15px;
        }
    )");
    QHBoxLayout *bottomLayout = new QHBoxLayout(summaryWidget);
    
    QLabel *cartInfoLabel = new QLabel("🛒 购物车统计", this);
    cartInfoLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #2e7d32;");
    
    m_totalLabel = new QLabel("💰 总计: ¥0.00", this);
    m_totalLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #1b5e20;");
    
    QLabel *itemCountLabel = new QLabel("📦 共 0 件商品", this);
    itemCountLabel->setObjectName("itemCountLabel");
    itemCountLabel->setStyleSheet("font-weight: bold; color: #388e3c;");
    
    bottomLayout->addWidget(cartInfoLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(itemCountLabel);
    bottomLayout->addSpacing(20);
    bottomLayout->addWidget(m_totalLabel);
    
    mainLayout->addWidget(summaryWidget);
    
    // 状态栏
    m_statusLabel = new QLabel("📋 购物车已就绪", this);
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            background-color: #e3f2fd;
            border: 1px solid #2196f3;
            border-radius: 6px;
            padding: 8px 15px;
            color: #1565c0;
            font-weight: bold;
        }
    )");
    
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &CartWidget::refreshCart);
    connect(m_removeButton, &QPushButton::clicked, this, &CartWidget::onRemoveFromCartClicked);
    connect(m_checkoutButton, &QPushButton::clicked, this, &CartWidget::onCheckoutClicked);
    
    // 清空购物车功能
    connect(clearCartButton, &QPushButton::clicked, [this]() {
        int result = QMessageBox::question(this, "确认清空", 
            "确定要清空购物车中的所有商品吗？\n此操作不可撤销！",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
            
        if (result == QMessageBox::Yes && m_client && m_client->isAuthenticated()) {
            // 这里需要调用清空购物车的API
            QMessageBox::information(this, "提示", "清空购物车功能待实现");
        }
    });
    
    // 修改数量功能
    connect(updateQuantityButton, &QPushButton::clicked, [this]() {
        QTreeWidgetItem *current = m_cartTree->currentItem();
        if (!current) {
            QMessageBox::warning(this, "选择商品", "请先选择要修改数量的商品");
            return;
        }
        
        bool ok;
        int currentQty = current->text(2).toInt();
        int newQty = QInputDialog::getInt(this, "修改数量", 
            QString("请输入新的数量\n当前数量: %1").arg(currentQty),
            currentQty, 1, 99, 1, &ok);
            
        if (ok && newQty != currentQty) {
            // 这里需要调用修改数量的API
            QMessageBox::information(this, "提示", 
                QString("将数量从 %1 修改为 %2\n（此功能待实现）").arg(currentQty).arg(newQty));
        }
    });
}

void CartWidget::loadCartItems(const QJsonObject &cartData)
{
    m_cartTree->clear();
    
    if (!cartData.contains("items")) {
        updateTotal(0.0);
        return;
    }
    
    QJsonArray items = cartData["items"].toArray();
    double totalAmount = 0.0;
    
    for (const QJsonValue &value : items) {
        QJsonObject item = value.toObject();
        
        QTreeWidgetItem *treeItem = new QTreeWidgetItem();
        
        QString productName = item["product_name"].toString();
        double price = item["price"].toDouble();
        int quantity = item["quantity"].toInt();
        double subtotal = price * quantity;
        qint64 productId = item["product_id"].toVariant().toLongLong();
        
        treeItem->setText(0, productName);
        treeItem->setText(1, QString("¥%1").arg(price, 0, 'f', 2));
        treeItem->setText(2, QString::number(quantity));
        treeItem->setText(3, QString("¥%1").arg(subtotal, 0, 'f', 2));
        
        treeItem->setData(0, Qt::UserRole, productId);
        
        m_cartTree->addTopLevelItem(treeItem);
        totalAmount += subtotal;
    }
    
    updateTotal(totalAmount);
    
    // 调整列宽
    for (int i = 0; i < m_cartTree->columnCount(); ++i) {
        m_cartTree->resizeColumnToContents(i);
    }
}

void CartWidget::updateTotal(double total)
{
    m_totalLabel->setText(QString("总计: ¥%1").arg(total, 0, 'f', 2));
}