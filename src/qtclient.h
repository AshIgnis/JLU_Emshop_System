#pragma once
#include "ui_qtclient.h"
#include <QMainWindow>

class qtclient : public QMainWindow {
    Q_OBJECT
    
public:
    qtclient(QWidget* parent = nullptr);
    ~qtclient();

private:
    Ui_qtclient* ui;
};