#include "qt_client.h"

qt_client::qt_client(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_qt_client)
{
    ui->setupUi(this);
}

qt_client::~qt_client()
{
    delete ui; 
}