#include "qtclient.h"

qtclient::qtclient(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_qtclient)
{
    ui->setupUi(this);
}

qtclient::~qtclient()
{
    delete ui; 
}