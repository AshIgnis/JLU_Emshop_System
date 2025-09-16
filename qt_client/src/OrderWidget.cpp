#include "OrderWidget.h"
#include <QHeaderView>
#include <QJsonArray>

OrderWidget::OrderWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
{
    setupUI();
}

void OrderWidget::setEmshopClient(EmshopClient *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &EmshopClient::ordersReceived,
                this, &OrderWidget::onOrdersReceived);
        connect(m_client, &EmshopClient::orderStatusUpdated,
                this, &OrderWidget::onOrderStatusUpdated);
    }
}

void OrderWidget::refreshOrders()
{
    if (m_client && m_client->isAuthenticated()) {
        m_client->getUserOrders();
        m_statusLabel->setText("正在加载订单...");
    }
}

void OrderWidget::onOrdersReceived(const QJsonObject &data)
{
    loadOrdersToTree(data);
    m_statusLabel->setText("订单列表已更新");
}

void OrderWidget::onOrderStatusUpdated(qint64 orderId, const QString &status, const QString &message)
{
    // 查找并更新对应的订单项
    for (int i = 0; i < m_orderTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = m_orderTree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toLongLong() == orderId) {
            item->setText(2, status);
            m_statusLabel->setText(QString("订单 %1 状态更新: %2").arg(orderId).arg(message));
            break;
        }
    }
}

void OrderWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 工具栏
    QHBoxLayout *toolLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("刷新", this);
    
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addStretch();
    
    // 订单列表
    m_orderTree = new QTreeWidget(this);
    m_orderTree->setHeaderLabels(QStringList() << "订单号" << "创建时间" << "状态" << "总金额");
    m_orderTree->header()->setStretchLastSection(true);
    m_orderTree->setAlternatingRowColors(true);
    
    // 状态
    m_statusLabel = new QLabel("准备就绪", this);
    
    // 组装布局
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(m_orderTree);
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &OrderWidget::refreshOrders);
}

void OrderWidget::loadOrdersToTree(const QJsonObject &ordersData)
{
    m_orderTree->clear();
    
    if (!ordersData.contains("orders")) {
        return;
    }
    
    QJsonArray orders = ordersData["orders"].toArray();
    
    for (const QJsonValue &value : orders) {
        QJsonObject order = value.toObject();
        
        QTreeWidgetItem *item = new QTreeWidgetItem();
        
        qint64 orderId = order["order_id"].toVariant().toLongLong();
        QString createTime = order["created_at"].toString();
        QString status = order["status"].toString();
        double totalAmount = order["total_amount"].toDouble();
        
        item->setText(0, QString::number(orderId));
        item->setText(1, createTime);
        item->setText(2, status);
        item->setText(3, QString("¥%1").arg(totalAmount, 0, 'f', 2));
        
        item->setData(0, Qt::UserRole, orderId);
        
        // 根据状态设置颜色
        if (status == "已取消" || status == "canceled") {
            item->setForeground(0, QBrush(Qt::red));
        } else if (status == "已完成" || status == "completed") {
            item->setForeground(0, QBrush(Qt::darkGreen));
        } else if (status == "待支付" || status == "pending_payment") {
            item->setForeground(0, QBrush(Qt::darkYellow));
        }
        
        m_orderTree->addTopLevelItem(item);
    }
    
    // 调整列宽
    for (int i = 0; i < m_orderTree->columnCount(); ++i) {
        m_orderTree->resizeColumnToContents(i);
    }
}