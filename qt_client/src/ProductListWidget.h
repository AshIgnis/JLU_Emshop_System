#ifndef PRODUCTLISTWIDGET_H
#define PRODUCTLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QJsonObject>
#include <QJsonArray>
#include "EmshopClient.h"

class ProductListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProductListWidget(QWidget *parent = nullptr);
    
    void setEmshopClient(EmshopClient *client);
    void refreshProducts();

private slots:
    void onSearchClicked();
    void onProductsReceived(const QJsonObject &data);
    void onSearchResultsReceived(const QJsonObject &data);
    void onAddToCartClicked();
    void onCartUpdated(const QJsonObject &data);

private:
    void setupUI();
    void loadProductsToTree(const QJsonArray &products);
    QTreeWidgetItem *createProductItem(const QJsonObject &product);
    
    EmshopClient *m_client;
    
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QPushButton *m_refreshButton;
    QComboBox *m_categoryCombo;
    
    QTreeWidget *m_productTree;
    QPushButton *m_addToCartButton;
    QSpinBox *m_quantitySpinBox;
    QLabel *m_statusLabel;
    
    qint64 m_selectedProductId;
};

#endif // PRODUCTLISTWIDGET_H