#pragma once
#include "ui_qt_client.h"
#include <QMainWindow>

class qt_client : public QMainWindow {
    Q_OBJECT
    
public:
    qt_client(QWidget* parent = nullptr);
    ~qt_client();

private:
    Ui_qt_client* ui;
};