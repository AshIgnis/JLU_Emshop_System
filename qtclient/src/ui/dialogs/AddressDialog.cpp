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
    
    // 设置完整的对话框样式,确保所有元素清晰可见
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f7fa;
        }
        QLabel {
            color: #2c3e50;
            font-weight: 500;
            font-size: 10pt;
        }
        QLineEdit {
            background-color: #ffffff;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 8px 12px;
            color: #2c3e50;
            selection-background-color: #3498db;
            font-size: 10pt;
        }
        QLineEdit:focus {
            border-color: #3498db;
            background-color: #f8f9fa;
        }
        
        /* 复选框样式 */
        QCheckBox {
            color: #2c3e50;
            font-size: 10pt;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 20px;
            height: 20px;
            border: 2px solid #dfe6e9;
            border-radius: 4px;
            background-color: #ffffff;
        }
        QCheckBox::indicator:hover {
            border-color: #3498db;
        }
        QCheckBox::indicator:checked {
            background-color: #3498db;
            border-color: #3498db;
        }
        
        /* 按钮样式 - 蓝色渐变 */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #4facfe, stop:1 #00f2fe);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 10pt;
            font-weight: 500;
            min-height: 36px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #3d9bef, stop:1 #00d9e5);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                      stop:0 #2d8bdf, stop:1 #00c0d5);
        }
        QPushButton:disabled {
            background: #b2bec3;
            color: #636e72;
        }
        
        /* 对话框按钮框中的按钮 */
        QDialogButtonBox QPushButton {
            min-width: 80px;
        }
    )");

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
