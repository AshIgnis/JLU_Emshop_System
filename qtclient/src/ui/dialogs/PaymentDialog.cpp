#include "ui/dialogs/PaymentDialog.h"

#include "core/ApplicationContext.h"
#include "network/NetworkClient.h"
#include "utils/JsonUtils.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStringList>

namespace {
QJsonArray extractMethods(const QJsonDocument &doc)
{
    static const QStringList paths = {
        QStringLiteral("data.methods"),
        QStringLiteral("methods"),
        QStringLiteral("data")
    };
    for (const auto &path : paths) {
        QJsonValue value = JsonUtils::extract(doc, path);
        if (value.isArray()) {
            return value.toArray();
        }
    }
    if (doc.isArray()) {
        return doc.array();
    }
    return {};
}
}

PaymentDialog::PaymentDialog(ApplicationContext &context, qlonglong orderId, double suggestedAmount, QWidget *parent)
    : QDialog(parent)
    , m_context(context)
    , m_orderId(orderId)
{
    setWindowTitle(tr("订单 %1 支付").arg(orderId));
    setModal(true);
    setMinimumWidth(400);
    
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
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #ffffff;
            border: 2px solid #dfe6e9;
            border-radius: 8px;
            padding: 8px 12px;
            color: #2c3e50;
            selection-background-color: #3498db;
            font-size: 10pt;
        }
        QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
            border-color: #3498db;
            background-color: #f8f9fa;
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
        
        /* 下拉框箭头 */
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 8px solid #636e72;
            margin-right: 8px;
        }
        QComboBox::down-arrow:hover {
            border-top-color: #2c3e50;
        }
        
        /* 数字输入框按钮 */
        QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
            background-color: #dfe6e9;
            border: none;
            border-radius: 4px;
            width: 20px;
            height: 16px;
        }
        QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #b2bec3;
        }
        QDoubleSpinBox::up-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 6px solid #2c3e50;
        }
        QDoubleSpinBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #2c3e50;
        }
    )");

    m_methodCombo = new QComboBox(this);
    populateDefaultMethods();

    m_amountSpin = new QDoubleSpinBox(this);
    m_amountSpin->setRange(0.01, 1000000.0);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setValue(suggestedAmount > 0 ? suggestedAmount : 0.01);

    m_accountInfoEdit = new QLineEdit(this);
    m_accountInfoEdit->setPlaceholderText(tr("可选：支付账号或交易号"));

    auto *fetchButton = new QPushButton(tr("同步支付方式"), this);
    connect(fetchButton, &QPushButton::clicked, this, &PaymentDialog::fetchMethodsFromServer);

    m_statusLabel = new QLabel(this);
    updateStatus(tr("请选择支付方式"), true);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("支付方式"), m_methodCombo);
    formLayout->addRow(QString(), fetchButton);
    formLayout->addRow(tr("支付金额"), m_amountSpin);
    formLayout->addRow(tr("支付备注"), m_accountInfoEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PaymentDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PaymentDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(m_statusLabel);
    layout->addWidget(buttonBox);
}

void PaymentDialog::populateDefaultMethods()
{
    m_methodCombo->clear();
    struct Method { const char *code; const char *name; } methods[] = {
        {"alipay", QT_TR_NOOP("支付宝")},
        {"wechat", QT_TR_NOOP("微信支付")},
        {"unionpay", QT_TR_NOOP("银联")},
        {"credit_card", QT_TR_NOOP("信用卡")},
        {"debit_card", QT_TR_NOOP("借记卡")}
    };
    for (const auto &method : methods) {
        m_methodCombo->addItem(tr(method.name), QString::fromUtf8(method.code));
    }
}

PaymentRequest PaymentDialog::request() const
{
    PaymentRequest req;
    req.method = m_methodCombo->currentData().toString();
    if (req.method.isEmpty()) {
        req.method = m_methodCombo->currentText();
    }
    req.amount = m_amountSpin->value();
    req.accountInfo = m_accountInfoEdit->text().trimmed();
    return req;
}

void PaymentDialog::fetchMethodsFromServer()
{
    NetworkClient *client = m_context.networkClient();
    if (!client->isConnected()) {
        updateStatus(tr("未连接服务器"), false);
        return;
    }

    updateStatus(tr("正在向服务器请求支付方式..."), true);
    QPointer<PaymentDialog> guard(this);
    client->sendCommand(QStringLiteral("GET_PAYMENT_METHODS"),
                        [this, guard](const QString &response) {
                            if (!guard) {
                                return;
                            }
                            bool ok = false;
                            QString error;
                            QJsonDocument doc = JsonUtils::parse(response, &ok, &error);
                            if (!ok) {
                                updateStatus(error, false);
                                return;
                            }
                            if (!JsonUtils::isSuccess(doc)) {
                                QString msg = JsonUtils::message(doc);
                                if (msg.isEmpty()) msg = response;
                                updateStatus(msg, false);
                                return;
                            }

                            QJsonArray methods = extractMethods(doc);
                            if (methods.isEmpty()) {
                                updateStatus(tr("服务器未返回支付方式"), false);
                                return;
                            }

                            m_methodCombo->clear();
                            for (const QJsonValue &value : methods) {
                                QJsonObject obj = value.toObject();
                                QString code = obj.value(QStringLiteral("code")).toString();
                                QString name = obj.value(QStringLiteral("name")).toString(code);
                                bool enabled = obj.value(QStringLiteral("enabled")).toBool(true);
                                if (enabled) {
                                    m_methodCombo->addItem(name, code);
                                }
                            }

                            updateStatus(tr("已同步支付方式"), true);
                        },
                        [this, guard](const QString &error) {
                            if (!guard) {
                                return;
                            }
                            updateStatus(error, false);
                        });
}

void PaymentDialog::updateStatus(const QString &message, bool ok)
{
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet(ok ? QStringLiteral("color:#188038;")
                                    : QStringLiteral("color:#c5221f;"));
}
