#include <QApplication>

#include "core/ApplicationContext.h"
#include "ui/LoginDialog.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("EmshopClient"));
    QApplication::setOrganizationName(QStringLiteral("JLU"));
    QApplication::setOrganizationDomain(QStringLiteral("jluEmshop.example"));
    
    // 设置全局应用程序样式,确保所有对话框和消息框都清晰可见
    app.setStyleSheet(R"(
        /* 全局对话框样式 */
        QDialog {
            background-color: #f5f7fa;
        }
        QDialog QLabel {
            color: #2c3e50;
            font-size: 10pt;
        }
        
        /* 消息框样式 */
        QMessageBox {
            background-color: #f5f7fa;
        }
        QMessageBox QLabel {
            color: #2c3e50;
            font-size: 10pt;
            min-width: 300px;
        }
        QMessageBox QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #4facfe, stop:1 #00f2fe);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 10pt;
            font-weight: 500;
            min-width: 70px;
            min-height: 32px;
        }
        QMessageBox QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #3d9bef, stop:1 #00d9e5);
        }
        QMessageBox QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #2d8bdf, stop:1 #00c0d5);
        }
        
        /* 输入对话框样式 */
        QInputDialog {
            background-color: #f5f7fa;
        }
        QInputDialog QLabel {
            color: #2c3e50;
            font-size: 10pt;
        }
        QInputDialog QLineEdit {
            background-color: #ffffff;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 8px 12px;
            color: #2c3e50;
            selection-background-color: #3498db;
            font-size: 10pt;
        }
        QInputDialog QLineEdit:focus {
            border-color: #3498db;
            background-color: #f8f9fa;
        }
        QInputDialog QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #4facfe, stop:1 #00f2fe);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 10pt;
            font-weight: 500;
            min-width: 70px;
            min-height: 32px;
        }
        QInputDialog QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #3d9bef, stop:1 #00d9e5);
        }
        QInputDialog QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #2d8bdf, stop:1 #00c0d5);
        }
    )");

    ApplicationContext context;

    LoginDialog loginDialog(context);
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow mainWindow(context);
    mainWindow.show();

    return app.exec();
}
