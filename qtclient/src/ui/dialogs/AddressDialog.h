#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QCheckBox;

struct AddressFormData {
    QString receiverName;
    QString phone;
    QString province;
    QString city;
    QString district;
    QString detailAddress;
    QString postalCode;
    bool isDefault = false;
};

class AddressDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddressDialog(QWidget *parent = nullptr);

    AddressFormData formData() const;

private slots:
    void validateAndAccept();

private:
    QLineEdit *m_receiverEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_provinceEdit;
    QLineEdit *m_cityEdit;
    QLineEdit *m_districtEdit;
    QLineEdit *m_detailEdit;
    QLineEdit *m_postalEdit;
    QCheckBox *m_defaultCheck;
};
