#include "OrderWidget.h"
#include <QHeaderView>
#include <QJsonArray>

OrderWidget::OrderWidget(QWidget *parent)
    : QWidget(parent)
    , m_client(nullptr)
{
    setupUI();
}

void OrderWidget::setEmshopClient(ClientAdapter *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &ClientAdapter::ordersReceived,
                this, &OrderWidget::onOrdersReceived);
        connect(m_client, &ClientAdapter::orderStatusUpdated,
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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 设置整体样式
    setStyleSheet(R"(
        QTreeWidget {
            border: 1px solid #ddd;
            border-radius: 8px;
            selection-background-color: #fff3e0;
            font-size: 13px;
        }
        QTreeWidget::item {
            padding: 10px;
            border-bottom: 1px solid #f0f0f0;
        }
        QTreeWidget::item:selected {
            background-color: #fff3e0;
            color: #e65100;
        }
        QPushButton {
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 14px;
        }
        QComboBox {
            padding: 8px;
            border: 2px solid #ddd;
            border-radius: 6px;
            font-size: 14px;
            min-width: 120px;
        }
    )");
    
    // 标题栏
    QLabel *titleLabel = new QLabel("📋 我的订单", this);
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
    
    QLabel *filterLabel = new QLabel("📊 订单状态筛选:", this);
    filterLabel->setStyleSheet("font-weight: bold; color: #555;");
    
    QComboBox *statusFilter = new QComboBox(this);
    statusFilter->addItem("🔍 全部订单", "all");
    statusFilter->addItem("⏳ 待付款", "pending");
    statusFilter->addItem("📦 待发货", "paid");
    statusFilter->addItem("🚚 已发货", "shipped");
    statusFilter->addItem("✅ 已完成", "completed");
    statusFilter->addItem("❌ 已取消", "cancelled");
    
    m_refreshButton = new QPushButton("🔄 刷新订单", this);
    m_refreshButton->setStyleSheet("background-color: #4CAF50;");
    
    QPushButton *viewDetailsButton = new QPushButton("👁️ 查看详情", this);
    viewDetailsButton->setStyleSheet("background-color: #2196f3;");
    
    QPushButton *cancelOrderButton = new QPushButton("❌ 取消订单", this);
    cancelOrderButton->setStyleSheet("background-color: #f44336;");
    
    QPushButton *payOrderButton = new QPushButton("💳 立即支付", this);
    payOrderButton->setStyleSheet("background-color: #ff9800;");
    
    toolLayout->addWidget(filterLabel);
    toolLayout->addWidget(statusFilter);
    toolLayout->addSpacing(20);
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addWidget(viewDetailsButton);
    toolLayout->addWidget(payOrderButton);
    toolLayout->addWidget(cancelOrderButton);
    toolLayout->addStretch();
    
    mainLayout->addWidget(toolWidget);
    
    // 订单列表
    m_orderTree = new QTreeWidget(this);
    m_orderTree->setHeaderLabels(QStringList() << "订单号" << "下单时间" << "订单状态" << "总金额(¥)" << "商品数量" << "备注");
    
    // 设置列宽
    m_orderTree->setColumnWidth(0, 180);  // 订单号
    m_orderTree->setColumnWidth(1, 150);  // 下单时间
    m_orderTree->setColumnWidth(2, 100);  // 状态
    m_orderTree->setColumnWidth(3, 120);  // 金额
    m_orderTree->setColumnWidth(4, 80);   // 数量
    
    m_orderTree->header()->setStretchLastSection(true);
    m_orderTree->setAlternatingRowColors(true);
    m_orderTree->setRootIsDecorated(false);
    m_orderTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_orderTree->setSortingEnabled(true);
    
    mainLayout->addWidget(m_orderTree);
    
    // 统计信息
    QWidget *summaryWidget = new QWidget(this);
    summaryWidget->setStyleSheet(R"(
        QWidget {
            background-color: #fce4ec;
            border: 2px solid #e91e63;
            border-radius: 10px;
            padding: 15px;
        }
    )");
    QHBoxLayout *summaryLayout = new QHBoxLayout(summaryWidget);
    
    QLabel *orderStatsLabel = new QLabel("📈 订单统计", this);
    orderStatsLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #ad1457;");
    
    QLabel *totalOrdersLabel = new QLabel("📋 总订单数: 0", this);
    totalOrdersLabel->setObjectName("totalOrdersLabel");
    totalOrdersLabel->setStyleSheet("font-weight: bold; color: #c2185b;");
    
    QLabel *totalAmountLabel = new QLabel("💰 累计消费: ¥0.00", this);
    totalAmountLabel->setObjectName("totalAmountLabel");
    totalAmountLabel->setStyleSheet("font-weight: bold; color: #c2185b;");
    
    summaryLayout->addWidget(orderStatsLabel);
    summaryLayout->addStretch();
    summaryLayout->addWidget(totalOrdersLabel);
    summaryLayout->addSpacing(20);
    summaryLayout->addWidget(totalAmountLabel);
    
    mainLayout->addWidget(summaryWidget);
    
    // 状态栏
    m_statusLabel = new QLabel("📋 订单列表已就绪", this);
    m_statusLabel->setStyleSheet(R"(
        QLabel {
            background-color: #fff3e0;
            border: 1px solid #ff9800;
            border-radius: 6px;
            padding: 8px 15px;
            color: #e65100;
            font-weight: bold;
        }
    )");
    
    mainLayout->addWidget(m_statusLabel);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &OrderWidget::refreshOrders);
    
    // 状态筛选
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, statusFilter]() {
        QString selectedStatus = statusFilter->currentData().toString();
        filterOrdersByStatus(selectedStatus);
    });
    
    // 查看订单详情
    connect(viewDetailsButton, &QPushButton::clicked, [this]() {
        QTreeWidgetItem *current = m_orderTree->currentItem();
        if (!current) {
            QMessageBox::information(this, "提示", "请先选择一个订单");
            return;
        }
        
        QString orderNo = current->text(0);
        QString orderTime = current->text(1);
        QString status = current->text(2);
        QString amount = current->text(3);
        QString itemCount = current->text(4);
        QString remark = current->text(5);
        
        QString details = QString(
            "📋 订单详情\n\n"
            "🔖 订单号：%1\n"
            "⏰ 下单时间：%2\n"
            "📊 订单状态：%3\n"
            "💰 订单金额：%4 元\n"
            "📦 商品数量：%5 件\n"
            "📝 订单备注：%6"
        ).arg(orderNo, orderTime, status, amount, itemCount, 
              remark.isEmpty() ? "无" : remark);
        
        QMessageBox::information(this, "订单详情", details);
    });
    
    // 支付订单
    connect(payOrderButton, &QPushButton::clicked, [this]() {
        QTreeWidgetItem *current = m_orderTree->currentItem();
        if (!current) {
            QMessageBox::information(this, "提示", "请先选择要支付的订单");
            return;
        }
        
        QString status = current->text(2);
        if (status != "pending" && status != "待付款") {
            QMessageBox::information(this, "提示", "该订单当前状态不支持支付操作");
            return;
        }
        
        QString orderNo = current->text(0);
        QString amount = current->text(3);
        
        int result = QMessageBox::question(this, "确认支付",
            QString("确认支付订单？\n\n订单号：%1\n支付金额：%2 元").arg(orderNo, amount),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
            
        if (result == QMessageBox::Yes) {
            QMessageBox::information(this, "提示", "支付功能待实现");
        }
    });
    
    // 取消订单
    connect(cancelOrderButton, &QPushButton::clicked, [this]() {
        QTreeWidgetItem *current = m_orderTree->currentItem();
        if (!current) {
            QMessageBox::information(this, "提示", "请先选择要取消的订单");
            return;
        }
        
        QString status = current->text(2);
        if (status == "completed" || status == "已完成" || 
            status == "cancelled" || status == "已取消") {
            QMessageBox::information(this, "提示", "该订单当前状态不允许取消");
            return;
        }
        
        QString orderNo = current->text(0);
        
        int result = QMessageBox::question(this, "确认取消",
            QString("确定要取消这个订单吗？\n\n订单号：%1\n\n取消后无法恢复！").arg(orderNo),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
            
        if (result == QMessageBox::Yes) {
            QMessageBox::information(this, "提示", "取消订单功能待实现");
        }
    });
    
    // 双击查看订单详情
    connect(m_orderTree, &QTreeWidget::itemDoubleClicked, viewDetailsButton, &QPushButton::clicked);
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

void OrderWidget::filterOrdersByStatus(const QString &status)
{
    if (status == "all") {
        // 显示所有订单
        for (int i = 0; i < m_orderTree->topLevelItemCount(); ++i) {
            m_orderTree->topLevelItem(i)->setHidden(false);
        }
        m_statusLabel->setText("📋 显示所有订单");
    } else {
        // 按状态筛选
        int visibleCount = 0;
        for (int i = 0; i < m_orderTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = m_orderTree->topLevelItem(i);
            QString itemStatus = item->text(2).toLower();
            
            bool shouldShow = false;
            if (status == "pending" && (itemStatus.contains("pending") || itemStatus.contains("待付款"))) {
                shouldShow = true;
            } else if (status == "paid" && (itemStatus.contains("paid") || itemStatus.contains("待发货"))) {
                shouldShow = true;
            } else if (status == "shipped" && (itemStatus.contains("shipped") || itemStatus.contains("已发货"))) {
                shouldShow = true;
            } else if (status == "completed" && (itemStatus.contains("completed") || itemStatus.contains("已完成"))) {
                shouldShow = true;
            } else if (status == "cancelled" && (itemStatus.contains("cancelled") || itemStatus.contains("已取消"))) {
                shouldShow = true;
            }
            
            item->setHidden(!shouldShow);
            if (shouldShow) visibleCount++;
        }
        
        m_statusLabel->setText(QString("📋 筛选显示 %1 个订单").arg(visibleCount));
    }
}