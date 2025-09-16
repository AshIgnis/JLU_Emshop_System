#include "CartWidget.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QJsonArray>

CartWidget::CartWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
{
    setupUI();
}

void CartWidget::setEmshopClient(EmshopClient *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &EmshopClient::cartReceived,
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
    
    // 工具栏
    QHBoxLayout *toolLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("刷新", this);
    m_removeButton = new QPushButton("移除选中", this);
    m_checkoutButton = new QPushButton("去结算", this);
    
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addWidget(m_removeButton);
    toolLayout->addStretch();
    toolLayout->addWidget(m_checkoutButton);
    
    // 购物车列表
    m_cartTree = new QTreeWidget(this);
    m_cartTree->setHeaderLabels(QStringList() << "商品" << "单价" << "数量" << "小计");
    m_cartTree->header()->setStretchLastSection(true);
    m_cartTree->setAlternatingRowColors(true);
    
    // 总计和状态
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    m_totalLabel = new QLabel("总计: ¥0.00", this);
    m_totalLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_statusLabel = new QLabel("准备就绪", this);
    
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_totalLabel);
    
    // 组装布局
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(m_cartTree);
    mainLayout->addLayout(bottomLayout);
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &CartWidget::refreshCart);
    connect(m_removeButton, &QPushButton::clicked, this, &CartWidget::onRemoveFromCartClicked);
    connect(m_checkoutButton, &QPushButton::clicked, this, &CartWidget::onCheckoutClicked);
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