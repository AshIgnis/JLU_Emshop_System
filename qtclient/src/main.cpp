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

    ApplicationContext context;

    LoginDialog loginDialog(context);
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow mainWindow(context);
    mainWindow.show();

    return app.exec();
}
