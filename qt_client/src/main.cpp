#include <QApplication>
#include <QMessageBox>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "EmshopTcpClient.h"
#include "ClientAdapter.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("JLU Emshop System");
    app.setApplicationVersion("2.0.0");
    app.setOrganizationName("Jilin University");
    app.setApplicationDisplayName("JLU 电商系统");
    
    // 设置全局样式
    app.setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
        }
        QGroupBox {
            background: rgba(255,255,255,0.1);
            border: 2px solid rgba(255,255,255,0.2);
            border-radius: 10px;
            font-weight: bold;
            color: white;
            margin-top: 15px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
            color: white;
            font-size: 16px;
        }
        QLineEdit {
            background: rgba(255,255,255,0.9);
            border: 2px solid transparent;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 14px;
            color: #333;
        }
        QLineEdit:focus {
            border: 2px solid #4CAF50;
            background: white;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4CAF50, stop:1 #45a049);
            border: none;
            border-radius: 6px;
            color: white;
            font-weight: bold;
            padding: 12px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #45a049, stop:1 #3d8b40);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3d8b40, stop:1 #2e7d32);
        }
        QPushButton:disabled {
            background: rgba(255,255,255,0.2);
            color: rgba(255,255,255,0.5);
        }
        QLabel {
            color: white;
        }
    )");
    
    // 主窗口指针，用于管理生命周期
    MainWindow *mainWindow = nullptr;
    EmshopTcpClient *client = nullptr;
    
    // 登录流程函数
    auto showLoginDialog = [&]() -> bool {
        // 创建新的客户端实例
        if (client) {
            client->deleteLater();
        }
        client = new EmshopTcpClient(&app);
        
        // 创建登录对话框
        LoginDialog loginDialog;
        
        // 连接登录对话框信号
        QObject::connect(&loginDialog, QOverload<const QString &, quint16>::of(&LoginDialog::connectRequested), 
                        client, &EmshopTcpClient::connectToServer);
        QObject::connect(&loginDialog, &LoginDialog::loginRequested, 
                        client, &EmshopTcpClient::authenticate);
        
        // 连接客户端状态信号
        QObject::connect(client, &EmshopTcpClient::connectionStateChanged, 
                        [&](EmshopTcpClient::ConnectionState state) {
            if (state == EmshopTcpClient::Connecting) {
                loginDialog.setConnecting(true);
            } else if (state == EmshopTcpClient::Connected) {
                loginDialog.onConnected();
                loginDialog.setConnecting(false);
            } else if (state == EmshopTcpClient::Disconnected) {
                loginDialog.setConnecting(false);
                loginDialog.setStatusMessage("❌ 连接已断开");
            }
        });
        
        QObject::connect(client, &EmshopTcpClient::error, &loginDialog, &LoginDialog::showError);
        
        // 认证成功处理
        bool authSuccess = false;
        QObject::connect(client, &EmshopTcpClient::authenticated, 
                        [&](const QJsonObject &userInfo) {
            authSuccess = true;
            loginDialog.accept();
            
            // 创建或更新主窗口
            if (mainWindow) {
                mainWindow->deleteLater();
            }
            mainWindow = new MainWindow();
            
            // 创建适配器以兼容现有MainWindow接口
            ClientAdapter *adapter = new ClientAdapter(client, mainWindow);
            mainWindow->setEmshopClient(adapter);  // 使用适配器而不是直接使用client
            mainWindow->show();
            
            // 显示欢迎消息
            QString username = userInfo["username"].toString();
            QString role = userInfo["role"].toString();
            QString welcomeMsg = QString("🎉 欢迎 %1 (%2) 使用JLU电商系统！\n\n✅ 已成功连接到你的原有Netty服务器")
                               .arg(username).arg(role);
            QMessageBox::information(mainWindow, "登录成功", welcomeMsg);
        });
        
        // 认证失败处理
        QObject::connect(client, &EmshopTcpClient::authenticationFailed, 
                        [&](const QString &error) {
            QString errorMsg = QString("🚫 登录失败: %1\n\n💡 请检查用户名密码或联系管理员。").arg(error);
            QMessageBox::warning(&loginDialog, "登录失败", errorMsg);
        });
        
        // 显示登录对话框
        int result = loginDialog.exec();
        return (result == QDialog::Accepted && authSuccess);
    };
    
    // 主循环：显示登录对话框
    while (true) {
        if (!showLoginDialog()) {
            // 用户取消登录或认证失败
            break;
        }
        
        // 主窗口关闭处理
        if (mainWindow) {
            // 等待主窗口关闭
            QEventLoop loop;
            QObject::connect(mainWindow, &MainWindow::destroyed, &loop, &QEventLoop::quit);
            
            // 主窗口关闭时的清理工作
            QObject::connect(mainWindow, &MainWindow::destroyed, [&]() {
                if (client) {
                    client->disconnectFromServer();
                }
                
                // 询问是否重新登录
                int ret = QMessageBox::question(nullptr, "重新登录", 
                    "🔄 是否要重新登录到JLU电商系统？",
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes);
                
                if (ret == QMessageBox::No) {
                    QApplication::quit();
                }
            });
            
            // 如果应用程序要退出，也退出循环
            QObject::connect(&app, &QApplication::aboutToQuit, &loop, &QEventLoop::quit);
            
            loop.exec(); // 等待主窗口关闭
            
            // 检查应用程序是否要退出
            if (app.closingDown()) {
                break;
            }
        } else {
            break; // 没有主窗口，异常情况
        }
    }
    
    // 最终清理
    if (client) {
        client->disconnectFromServer();
        client->deleteLater();
    }
    if (mainWindow) {
        mainWindow->deleteLater();
    }
    
    return 0; // 正常退出
}