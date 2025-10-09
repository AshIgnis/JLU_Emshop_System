#include "ui/MainWindow.h"

#include "core/ApplicationContext.h"
#include "core/UserSession.h"
#include "network/NetworkClient.h"
#include "ui/tabs/CartTab.h"
#include "ui/tabs/DashboardTab.h"
#include "ui/tabs/OrdersTab.h"
#include "ui/tabs/ProductsTab.h"
#include "ui/tabs/AdminTab.h"

#include <QAction>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>

MainWindow::MainWindow(ApplicationContext &context, QWidget *parent)
    : QMainWindow(parent)
    , m_context(context)
    , m_tabWidget(new QTabWidget(this))
    , m_dashboardTab(new DashboardTab(context, this))
    , m_productsTab(new ProductsTab(context, this))
    , m_cartTab(new CartTab(context, this))
    , m_ordersTab(new OrdersTab(context, this))
    , m_statusConnectionLabel(new QLabel(this))
    , m_statusUserLabel(new QLabel(this))
{
    setupUi();
    setupMenus();

    setCentralWidget(m_tabWidget);
    m_tabWidget->addTab(m_dashboardTab, tr("概览"));
    m_tabWidget->addTab(m_productsTab, tr("商品"));
    m_tabWidget->addTab(m_cartTab, tr("购物车"));
    m_tabWidget->addTab(m_ordersTab, tr("订单"));
    // 管理员标签延后在handleSessionChanged中按需添加

    auto *status = statusBar();
    status->addPermanentWidget(m_statusConnectionLabel);
    status->addPermanentWidget(m_statusUserLabel);

    NetworkClient *client = m_context.networkClient();
    connect(client, &NetworkClient::connectionStateChanged, this, &MainWindow::updateConnectionStatus);
    connect(client, &NetworkClient::serverMessageReceived, this, [this](const QString &message) {
        statusBar()->showMessage(tr("服务器: %1").arg(message), 5000);
    });

    connect(&m_context, &ApplicationContext::sessionChanged, this, &MainWindow::handleSessionChanged);

    connect(m_productsTab, &ProductsTab::cartChanged, this, &MainWindow::handleCartChanged);
    connect(m_productsTab, &ProductsTab::statusMessage, this, &MainWindow::handleStatusMessage);
    connect(m_cartTab, &CartTab::statusMessage, this, &MainWindow::handleStatusMessage);
    connect(m_cartTab, &CartTab::orderCreated, m_ordersTab, &OrdersTab::refreshOrders);
    // 全量刷新与局部刷新并存：orderCreatedWithStock 先局部更新，普通信号仍可触发全量（保留冗余）
    connect(m_cartTab, &CartTab::orderCreatedWithStock, m_productsTab, &ProductsTab::applyStockChanges);
    connect(m_cartTab, &CartTab::orderCreated, m_productsTab, &ProductsTab::refreshProducts);
    connect(m_ordersTab, &OrdersTab::statusMessage, this, &MainWindow::handleStatusMessage);
    // AdminTab 的状态提示统一到状态栏
    if (m_adminTab) {
        connect(m_adminTab, &AdminTab::statusMessage, this, &MainWindow::handleStatusMessage);
    }

    handleSessionChanged(m_context.session());
    updateConnectionStatus(client->isConnected());
}

void MainWindow::setupUi()
{
    resize(1100, 720);
    setWindowTitle(tr("Emshop 桌面客户端"));
}

void MainWindow::setupMenus()
{
    auto *fileMenu = menuBar()->addMenu(tr("文件"));
    QAction *logoutAction = fileMenu->addAction(tr("注销"));
    QAction *exitAction = fileMenu->addAction(tr("退出"));

    connect(logoutAction, &QAction::triggered, this, &MainWindow::logout);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto *helpMenu = menuBar()->addMenu(tr("帮助"));
    QAction *aboutAction = helpMenu->addAction(tr("关于"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::updateConnectionStatus(bool connected)
{
    NetworkClient *client = m_context.networkClient();
    if (connected) {
        m_statusConnectionLabel->setText(tr("已连接 %1:%2")
                                             .arg(client->host())
                                             .arg(client->port()));
    } else {
        m_statusConnectionLabel->setText(tr("离线"));
    }
}

void MainWindow::handleSessionChanged(const UserSession &session)
{
    if (session.isValid()) {
        m_statusUserLabel->setText(tr("用户: %1 (ID:%2, 角色:%3)")
                                       .arg(session.username)
                                       .arg(session.userId)
                                       .arg(session.role));
    } else {
        m_statusUserLabel->setText(tr("未登录"));
    }
    m_dashboardTab->handleSessionChanged(session);
    m_productsTab->handleSessionChanged(session);
    m_cartTab->handleSessionChanged(session);
    m_ordersTab->handleSessionChanged(session);

    // 动态管理 AdminTab
    int adminIndex = -1;
    for (int i=0;i<m_tabWidget->count();++i) {
        if (m_tabWidget->tabText(i) == tr("管理员")) { adminIndex = i; break; }
    }
    const bool isAdmin = session.isValid() && session.isAdmin();
    if (isAdmin) {
        if (!m_adminTab) m_adminTab = new AdminTab(m_context, this);
        if (adminIndex < 0) {
            m_tabWidget->addTab(m_adminTab, tr("管理员"));
            statusBar()->showMessage(tr("管理员功能已启用"), 4000);
        }
        // 确保信号连接（重复连接不会出错，如果担心可先断开再连）
        connect(m_adminTab, &AdminTab::statusMessage, this, &MainWindow::handleStatusMessage);
        m_adminTab->handleSessionChanged(session);
    } else {
        if (adminIndex >= 0) {
            QWidget *w = m_tabWidget->widget(adminIndex);
            m_tabWidget->removeTab(adminIndex);
            // 保留对象以便后续复用，若需释放可在此 delete w;
            if (w) {
                disconnect(w, nullptr, this, nullptr);
            }
        }
    }
    updateWindowTitle();
}

void MainWindow::handleCartChanged()
{
    m_cartTab->refreshCart();
}

void MainWindow::handleStatusMessage(const QString &message, bool success)
{
    statusBar()->showMessage(message, success ? 4000 : 7000);
}

void MainWindow::logout()
{
    m_context.clearSession();
    m_context.networkClient()->disconnectFromServer();
    close();
}

void MainWindow::showAbout()
{
    QMessageBox::information(this,
                             tr("关于 Emshop"),
                             tr("Emshop 桌面客户端\n\n基于 Qt 构建，支持与 Netty 服务器进行 TCP 通信，提供商品浏览、购物车、订单管理等功能。"));
}

void MainWindow::updateWindowTitle()
{
    const UserSession &session = m_context.session();
    if (session.isValid()) {
        setWindowTitle(tr("Emshop - %1 (%2)").arg(session.username, session.role));
    } else {
        setWindowTitle(tr("Emshop 桌面客户端"));
    }
}
