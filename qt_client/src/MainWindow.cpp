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
    setWindowTitle("JLU Emshop 电商客户端");
    setMinimumSize(1200, 800);
    
    // 设置应用程序图标和样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QTabWidget::pane {
            border: 1px solid #c0c0c0;
            background-color: white;
        }
        QTabWidget::tab-bar {
            alignment: left;
        }
        QTabBar::tab {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #f0f0f0, stop: 1 #e5e5e5);
            border: 1px solid #c0c0c0;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                                      stop: 0 #fafafa, stop: 1 #ffffff);
            border-bottom-color: #ffffff;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed {
            background-color: #3d8b40;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        QStatusBar {
            background-color: #e0e0e0;
            border-top: 1px solid #c0c0c0;
        }
    )");
    
    setupUI();
    setupMenuBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setEmshopClient(ClientAdapter *client)
{
    m_client = client;
    
    if (m_client) {
    connect(m_client, &ClientAdapter::connectionStateChanged,
        this, &MainWindow::onConnectionStateChanged);
    connect(m_client, &ClientAdapter::authenticated,
        this, &MainWindow::onAuthenticated);
    connect(m_client, &ClientAdapter::systemNotificationReceived,
        this, &MainWindow::onSystemNotificationReceived);
        
    // 传递客户端给子组件
    m_productListWidget->setEmshopClient(m_client);
    m_cartWidget->setEmshopClient(m_client);
    m_orderWidget->setEmshopClient(m_client);
        
    updateStatusBar();
    }
}

void MainWindow::onConnectionStateChanged(ClientAdapter::ConnectionState state)
{
    updateStatusBar();
    
    bool connected = (state == ClientAdapter::Connected || state == ClientAdapter::Authenticated);
    bool authenticated = (state == ClientAdapter::Authenticated);
    
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
    
    // 创建顶部欢迎栏
    QWidget *welcomeBar = new QWidget(this);
    welcomeBar->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,
                                      stop: 0 #667eea, stop: 1 #764ba2);
            color: white;
            padding: 10px;
        }
    )");
    QHBoxLayout *welcomeLayout = new QHBoxLayout(welcomeBar);
    
    QLabel *logoLabel = new QLabel("🛒", this);
    logoLabel->setStyleSheet("font-size: 32px; color: white; font-weight: bold;");
    
    QLabel *titleLabel = new QLabel("JLU 电商系统", this);
    titleLabel->setStyleSheet("font-size: 24px; color: white; font-weight: bold; margin-left: 10px;");
    
    welcomeLayout->addWidget(logoLabel);
    welcomeLayout->addWidget(titleLabel);
    welcomeLayout->addStretch();
    
    // 创建工具栏
    QWidget *toolBar = new QWidget(this);
    toolBar->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-bottom: 2px solid #e0e0e0;
            padding: 10px;
        }
    )");
    QHBoxLayout *toolLayout = new QHBoxLayout(toolBar);
    
    m_refreshButton = new QPushButton("🔄 刷新数据", this);
    m_refreshButton->setStyleSheet("background-color: #2196F3;");
    
    QPushButton *helpButton = new QPushButton("❓ 帮助", this);
    helpButton->setStyleSheet("background-color: #FF9800;");
    
    QPushButton *aboutButton = new QPushButton("ℹ️ 关于", this);
    aboutButton->setStyleSheet("background-color: #9C27B0;");
    
    m_disconnectButton = new QPushButton("🚪 断开连接", this);
    m_disconnectButton->setStyleSheet("background-color: #f44336;");
    
    toolLayout->addWidget(m_refreshButton);
    toolLayout->addWidget(helpButton);
    toolLayout->addWidget(aboutButton);
    toolLayout->addStretch();
    toolLayout->addWidget(m_disconnectButton);
    
    // 创建标签页控件
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::North);
    
    // 创建各个页面
    m_productListWidget = new ProductListWidget(this);
    m_cartWidget = new CartWidget(this);
    m_orderWidget = new OrderWidget(this);
    
    // 添加标签页，使用图标
    m_tabWidget->addTab(m_productListWidget, "🛍️ 商品浏览");
    m_tabWidget->addTab(m_cartWidget, "🛒 购物车");
    m_tabWidget->addTab(m_orderWidget, "📋 我的订单");
    
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(welcomeBar);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(m_tabWidget);
    
    // 初始状态
    m_tabWidget->setEnabled(false);
    m_refreshButton->setEnabled(false);
    
    // 连接信号
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onRefreshData);
    connect(m_disconnectButton, &QPushButton::clicked, this, &MainWindow::onDisconnect);
    
    // 帮助和关于按钮
    connect(helpButton, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "帮助", 
            "JLU 电商系统使用帮助：\n\n"
            "1. 商品浏览：浏览和搜索商品，添加到购物车\n"
            "2. 购物车：管理购物车中的商品\n"
            "3. 我的订单：查看订单历史和状态\n\n"
            "如有问题，请联系系统管理员。");
    });
    
    connect(aboutButton, &QPushButton::clicked, [this]() {
        QMessageBox::about(this, "关于",
            "JLU 电商系统 v1.0\n\n"
            "技术栈：\n"
            "• 前端：Qt 6.x + C++\n"
            "• 后端：Java 21 + Netty\n"
            "• 数据库：MySQL 8.0\n"
            "• 通信：WebSocket\n\n"
            "作者：吉林大学计算机学院\n"
            "© 2025 JLU. All rights reserved.");
    });
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
    
    ClientAdapter::ConnectionState state = m_client->connectionState();
    
    switch (state) {
    case ClientAdapter::Disconnected:
            m_statusLabel->setText("未连接");
            break;
    case ClientAdapter::Connecting:
            m_statusLabel->setText("连接中...");
            break;
    case ClientAdapter::Connected:
            m_statusLabel->setText("已连接");
            break;
    case ClientAdapter::Authenticated:
            m_statusLabel->setText("已认证");
            break;
    }
}