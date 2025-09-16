#include "MainWindow.h"
#include "ProductListWidget.h"
#include "CartWidget.h"
#include "OrderWidget.h"
#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_client(nullptr)
{
    setWindowTitle("Emshop 客户端");
    setMinimumSize(1000, 700);
    setupUI();
    setupMenuBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setEmshopClient(EmshopClient *client)
{
    m_client = client;
    
    if (m_client) {
        connect(m_client, &EmshopClient::connectionStateChanged,
                this, &MainWindow::onConnectionStateChanged);
        connect(m_client, &EmshopClient::authenticated,
                this, &MainWindow::onAuthenticated);
        connect(m_client, &EmshopClient::systemNotificationReceived,
                this, &MainWindow::onSystemNotificationReceived);
        
        // 传递客户端给子组件
        m_productListWidget->setEmshopClient(m_client);
        m_cartWidget->setEmshopClient(m_client);
        m_orderWidget->setEmshopClient(m_client);
        
        updateStatusBar();
    }
}

void MainWindow::onConnectionStateChanged(EmshopClient::ConnectionState state)
{
    updateStatusBar();
    
    bool connected = (state == EmshopClient::Connected || state == EmshopClient::Authenticated);
    bool authenticated = (state == EmshopClient::Authenticated);
    
    // 启用/禁用功能
    m_tabWidget->setEnabled(authenticated);
    m_refreshButton->setEnabled(authenticated);
    
    if (!connected) {
        m_userLabel->setText("未登录");
    }
}

void MainWindow::onAuthenticated(const QJsonObject &userInfo)
{
    m_userInfo = userInfo;
    QString username = userInfo["username"].toString();
    QString role = userInfo["role"].toString();
    
    m_userLabel->setText(QString("用户: %1 (%2)").arg(username, role));
    
    // 自动刷新数据
    onRefreshData();
}

void MainWindow::onSystemNotificationReceived(const QString &title, const QString &content, const QString &level)
{
    QMessageBox::Icon icon = QMessageBox::Information;
    if (level == "warning") {
        icon = QMessageBox::Warning;
    } else if (level == "error") {
        icon = QMessageBox::Critical;
    }
    
    QMessageBox msgBox(icon, title, content, QMessageBox::Ok, this);
    msgBox.exec();
}

void MainWindow::onRefreshData()
{
    if (m_client && m_client->isAuthenticated()) {
        // 刷新所有数据
        m_productListWidget->refreshProducts();
        m_cartWidget->refreshCart();
        m_orderWidget->refreshOrders();
    }
}

void MainWindow::onDisconnect()
{
    if (m_client) {
        m_client->disconnectFromServer();
    }
    close(); // 关闭主窗口，回到登录界面
}

void MainWindow::setupUI()
{
    // 创建中央组件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    
    // 创建各个页面
    m_productListWidget = new ProductListWidget(this);
    m_cartWidget = new CartWidget(this);
    m_orderWidget = new OrderWidget(this);
    
    // 添加标签页
    m_tabWidget->addTab(m_productListWidget, "商品");
    m_tabWidget->addTab(m_cartWidget, "购物车");
    m_tabWidget->addTab(m_orderWidget, "订单");
    
    // 创建顶部工具栏
    QWidget *toolBar = new QWidget(this);
    QHBoxLayout *toolLayout = new QHBoxLayout(toolBar);
    
    m_refreshButton = new QPushButton("刷新", this);
    m_disconnectButton = new QPushButton("断开连接", this);
    
    toolLayout->addStretch();
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addWidget(m_disconnectButton);
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(m_tabWidget);
    
    // 初始状态
    m_tabWidget->setEnabled(false);
    m_refreshButton->setEnabled(false);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshData);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnect);
}

void MainWindow::setupMenuBar()
{
    // 创建状态栏
    m_statusLabel = new QLabel("未连接", this);
    m_userLabel = new QLabel("未登录", this);
    
    statusBar()->addWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_userLabel);
}

void MainWindow::updateStatusBar()
{
    if (!m_client) {
        m_statusLabel->setText("未连接");
        return;
    }
    
    EmshopClient::ConnectionState state = m_client->connectionState();
    
    switch (state) {
        case EmshopClient::Disconnected:
            m_statusLabel->setText("未连接");
            break;
        case EmshopClient::Connecting:
            m_statusLabel->setText("连接中...");
            break;
        case EmshopClient::Connected:
            m_statusLabel->setText("已连接");
            break;
        case EmshopClient::Authenticated:
            m_statusLabel->setText("已认证");
            break;
    }
}