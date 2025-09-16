#include <QApplication>
#include <QMessageBox>
#include "LoginDialog.h"
#include "MainWindow.h"
#include "EmshopClient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Emshop Qt Client");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("JLU");
    
    // 创建客户端
    EmshopClient client;
    
    // 显示登录对话框
    LoginDialog loginDialog;
    
    // 连接登录对话框信号
    QObject::connect(&loginDialog, &LoginDialog::connectRequested, &client, &EmshopClient::connectToServer);
    QObject::connect(&loginDialog, &LoginDialog::loginRequested, &client, &EmshopClient::authenticate);
    
    // 连接客户端状态信号
    QObject::connect(&client, &EmshopClient::connectionStateChanged, [&](EmshopClient::ConnectionState state) {
        if (state == EmshopClient::Connecting) {
            loginDialog.setConnecting(true);
        } else if (state == EmshopClient::Connected) {
            loginDialog.onConnected();
            loginDialog.setConnecting(false);
        } else if (state == EmshopClient::Disconnected) {
            loginDialog.setConnecting(false);
        }
    });
    
    QObject::connect(&client, &EmshopClient::error, &loginDialog, &LoginDialog::showError);
    
    // 主窗口指针
    MainWindow *mainWindow = nullptr;
    
    // 认证成功后显示主窗口
    QObject::connect(&client, &EmshopClient::authenticated, [&](const QJsonObject &userInfo) {
        loginDialog.accept(); // 关闭登录对话框
        
        // 创建主窗口
        mainWindow = new MainWindow();
        mainWindow->setEmshopClient(&client);
        mainWindow->show();
    });
    
    // 认证失败处理
    QObject::connect(&client, &EmshopClient::authenticationFailed, [&](const QString &error) {
        QMessageBox::warning(&loginDialog, "登录失败", error);
    });
    
    // 主窗口关闭后重新显示登录对话框
    if (mainWindow) {
        QObject::connect(mainWindow, &MainWindow::destroyed, [&]() {
            client.disconnectFromServer();
            loginDialog.show();
            mainWindow = nullptr;
        });
    }
    
    // 显示登录对话框
    if (loginDialog.exec() == QDialog::Rejected) {
        return 0; // 用户取消登录
    }
    
    // 如果没有主窗口，创建一个（防止意外情况）
    if (!mainWindow) {
        mainWindow = new MainWindow();
        mainWindow->setEmshopClient(&client);
        mainWindow->show();
    }
    
    // 程序关闭时断开连接
    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        client.disconnectFromServer();
        if (mainWindow) {
            mainWindow->deleteLater();
        }
    });
    
    return app.exec();
}