#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>

/**
 * @brief 样式辅助类 - 提供现代化UI样式
 */
class StyleHelper
{
public:
    /**
     * @brief 获取通用表格样式
     */
    static QString tableStyle()
    {
        return R"(
            QTableWidget, QTableView {
                background-color: white;
                alternate-background-color: #f8f9fa;
                gridline-color: #ecf0f1;
                border: 2px solid #dfe6e9;
                border-radius: 10px;
                selection-background-color: #3498db;
                selection-color: white;
            }
            QTableWidget::item, QTableView::item {
                padding: 10px;
                border: none;
            }
            QTableWidget::item:selected {
                background-color: #3498db;
                color: white;
            }
            QTableWidget::item:hover:!selected {
                background-color: #ecf0f1;
            }
            QHeaderView::section {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #34495e, stop:1 #2c3e50);
                color: white;
                padding: 12px;
                border: none;
                font-weight: 600;
                font-size: 10pt;
            }
            QHeaderView::section:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #3d566e, stop:1 #2c3e50);
            }
        )";
    }

    /**
     * @brief 获取主要操作按钮样式（蓝色渐变）
     */
    static QString primaryButtonStyle()
    {
        return R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #667eea, stop:1 #764ba2);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 10px 20px;
                font-weight: 600;
                font-size: 10pt;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #5568d3, stop:1 #6a3e91);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #4d5dbd, stop:1 #5f3681);
            }
            QPushButton:disabled {
                background: #cccccc;
                color: #999999;
            }
        )";
    }

    /**
     * @brief 获取成功按钮样式（绿色渐变）
     */
    static QString successButtonStyle()
    {
        return R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #11998e, stop:1 #38ef7d);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 10px 20px;
                font-weight: 600;
                font-size: 10pt;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #0f8a7e, stop:1 #32d86f);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #0d7a6e, stop:1 #2dc261);
            }
        )";
    }

    /**
     * @brief 获取危险操作按钮样式（红色渐变）
     */
    static QString dangerButtonStyle()
    {
        return R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #eb3349, stop:1 #f45c43);
                color: white;
                border: none;
                border-radius: 8px;
                padding: 10px 20px;
                font-weight: 600;
                font-size: 10pt;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #d42e42, stop:1 #dc523c);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #bd293b, stop:1 #c44835);
            }
        )";
    }

    /**
     * @brief 获取次要按钮样式（透明边框）
     */
    static QString secondaryButtonStyle()
    {
        return R"(
            QPushButton {
                background-color: transparent;
                color: #667eea;
                border: 2px solid #667eea;
                border-radius: 8px;
                padding: 10px 20px;
                font-weight: 500;
                font-size: 10pt;
            }
            QPushButton:hover {
                background-color: rgba(102, 126, 234, 0.1);
            }
            QPushButton:pressed {
                background-color: rgba(102, 126, 234, 0.2);
            }
        )";
    }

    /**
     * @brief 获取输入框样式
     */
    static QString inputStyle()
    {
        return R"(
            QLineEdit, QSpinBox, QComboBox {
                background-color: #ffffff;
                border: 2px solid #dfe6e9;
                border-radius: 8px;
                padding: 10px 14px;
                selection-background-color: #3498db;
                font-size: 10pt;
            }
            QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
                border-color: #3498db;
                background-color: #f8f9fa;
            }
            QLineEdit:hover, QSpinBox:hover, QComboBox:hover {
                border-color: #b2bec3;
            }
        )";
    }

    /**
     * @brief 获取卡片容器样式
     */
    static QString cardStyle()
    {
        return R"(
            QWidget {
                background-color: white;
                border: 2px solid #ecf0f1;
                border-radius: 12px;
                padding: 16px;
            }
        )";
    }

    /**
     * @brief 获取成功消息样式
     */
    static QString successMessageStyle()
    {
        return R"(
            QLabel {
                background-color: #d4edda;
                color: #155724;
                border: 1px solid #c3e6cb;
                border-radius: 6px;
                padding: 12px 16px;
                font-weight: 500;
            }
        )";
    }

    /**
     * @brief 获取错误消息样式
     */
    static QString errorMessageStyle()
    {
        return R"(
            QLabel {
                background-color: #f8d7da;
                color: #721c24;
                border: 1px solid #f5c6cb;
                border-radius: 6px;
                padding: 12px 16px;
                font-weight: 500;
            }
        )";
    }

    /**
     * @brief 获取警告消息样式
     */
    static QString warningMessageStyle()
    {
        return R"(
            QLabel {
                background-color: #fff3cd;
                color: #856404;
                border: 1px solid #ffeeba;
                border-radius: 6px;
                padding: 12px 16px;
                font-weight: 500;
            }
        )";
    }

    /**
     * @brief 获取信息消息样式
     */
    static QString infoMessageStyle()
    {
        return R"(
            QLabel {
                background-color: #d1ecf1;
                color: #0c5460;
                border: 1px solid #bee5eb;
                border-radius: 6px;
                padding: 12px 16px;
                font-weight: 500;
            }
        )";
    }
};

#endif // STYLEHELPER_H
