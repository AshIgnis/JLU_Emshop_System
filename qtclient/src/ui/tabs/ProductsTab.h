#pragma once

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;
class QTableWidget;

class ApplicationContext;
struct UserSession;

class ProductsTab : public QWidget {
    Q_OBJECT
public:
    explicit ProductsTab(ApplicationContext &context, QWidget *parent = nullptr);

signals:
    void cartChanged();
    void statusMessage(const QString &message, bool success);

public slots:
    void refreshProducts();
    void handleSessionChanged(const UserSession &session);

private slots:
    void executeSearch();
    void addSelectedToCart();
    void updateDetailView();

private:
    void requestProducts(const QString &command, const QString &actionLabel);
    void populateTable(const QJsonDocument &doc);
    qlonglong productIdFromRow(int row) const;
    QJsonObject productObjectFromRow(int row) const;
    void showResponseInDetail(const QString &label, const QString &payload);

    ApplicationContext &m_context;
    QComboBox *m_categoryCombo;
    QLineEdit *m_searchEdit;
    QSpinBox *m_pageSpin;
    QSpinBox *m_pageSizeSpin;
    QSpinBox *m_quantitySpin;
    QTableWidget *m_table;
    QPlainTextEdit *m_detailView;
    QString m_lastRawResponse;
    bool m_isLoggedIn = false;
};
