#ifndef COUPONSTAB_H
#define COUPONSTAB_H

#include <QWidget>
#include <QJsonDocument>
#include <functional>

class ApplicationContext;
class QTableWidget;
class QPlainTextEdit;
class QLabel;

class CouponsTab : public QWidget
{
    Q_OBJECT

public:
    explicit CouponsTab(ApplicationContext &context, QWidget *parent = nullptr);

signals:
    void statusMessage(const QString &message, bool isSuccess);

public slots:
    void handleSessionChanged(const class UserSession &session);
    void refreshCoupons();
    void viewCouponDetail();
    void viewCouponTemplates();

private:
    void populateCoupons(const QJsonDocument &doc);
    void populateCouponTemplates(const QJsonDocument &doc);
    QJsonObject selectedCoupon() const;
    qlonglong selectedCouponId() const;
    void updateDetailView();
    void sendCouponCommand(const QString &command,
                          const QString &actionName,
                          std::function<void(const QJsonDocument &)> onSuccess = nullptr);

    ApplicationContext &m_context;
    QTableWidget *m_couponTable = nullptr;
    QPlainTextEdit *m_detailView = nullptr;
    QLabel *m_summaryLabel = nullptr;
    bool m_loggedIn = false;
    bool m_showingTemplates = false;
};

#endif // COUPONSTAB_H
