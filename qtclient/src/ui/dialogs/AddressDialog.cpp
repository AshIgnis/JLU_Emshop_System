#include "ui/dialogs/AddressDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QVBoxLayout>

AddressDialog::AddressDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("新增收货地址"));
    setModal(true);

    m_receiverEdit = new QLineEdit(this);
    m_phoneEdit = new QLineEdit(this);
    m_provinceEdit = new QLineEdit(this);
    m_cityEdit = new QLineEdit(this);
    m_districtEdit = new QLineEdit(this);
    m_detailEdit = new QLineEdit(this);
    m_postalEdit = new QLineEdit(this);
    m_defaultCheck = new QCheckBox(tr("设为默认地址"), this);

    m_phoneEdit->setPlaceholderText(tr("手机或联系电话"));
    m_detailEdit->setPlaceholderText(tr("街道/楼栋/房间号"));
    m_postalEdit->setPlaceholderText(tr("如不确定可留空"));

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("收件人"), m_receiverEdit);
    formLayout->addRow(tr("手机号"), m_phoneEdit);
    formLayout->addRow(tr("省份"), m_provinceEdit);
    formLayout->addRow(tr("城市"), m_cityEdit);
    formLayout->addRow(tr("区县"), m_districtEdit);
    formLayout->addRow(tr("详细地址"), m_detailEdit);
    formLayout->addRow(tr("邮编"), m_postalEdit);
    formLayout->addRow(QString(), m_defaultCheck);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddressDialog::validateAndAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddressDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);
}

AddressFormData AddressDialog::formData() const
{
    AddressFormData data;
    data.receiverName = m_receiverEdit->text().trimmed();
    data.phone = m_phoneEdit->text().trimmed();
    data.province = m_provinceEdit->text().trimmed();
    data.city = m_cityEdit->text().trimmed();
    data.district = m_districtEdit->text().trimmed();
    data.detailAddress = m_detailEdit->text().trimmed();
    data.postalCode = m_postalEdit->text().trimmed();
    data.isDefault = m_defaultCheck->isChecked();
    return data;
}

void AddressDialog::validateAndAccept()
{
    AddressFormData data = formData();
    if (data.receiverName.isEmpty() || data.phone.isEmpty() || data.province.isEmpty() || data.city.isEmpty() || data.detailAddress.isEmpty()) {
        QMessageBox::warning(this, tr("信息不完整"), tr("请至少填写收件人、电话、省份、城市和详细地址"));
        return;
    }
    accept();
}
