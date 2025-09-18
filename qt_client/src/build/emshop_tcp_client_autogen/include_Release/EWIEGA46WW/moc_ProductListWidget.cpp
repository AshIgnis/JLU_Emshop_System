/****************************************************************************
** Meta object code from reading C++ file 'ProductListWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../ProductListWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ProductListWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ProductListWidget_t {
    QByteArrayData data[8];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ProductListWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ProductListWidget_t qt_meta_stringdata_ProductListWidget = {
    {
QT_MOC_LITERAL(0, 0, 17), // "ProductListWidget"
QT_MOC_LITERAL(1, 18, 15), // "onSearchClicked"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 18), // "onProductsReceived"
QT_MOC_LITERAL(4, 54, 4), // "data"
QT_MOC_LITERAL(5, 59, 23), // "onSearchResultsReceived"
QT_MOC_LITERAL(6, 83, 18), // "onAddToCartClicked"
QT_MOC_LITERAL(7, 102, 13) // "onCartUpdated"

    },
    "ProductListWidget\0onSearchClicked\0\0"
    "onProductsReceived\0data\0onSearchResultsReceived\0"
    "onAddToCartClicked\0onCartUpdated"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ProductListWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x08 /* Private */,
       3,    1,   40,    2, 0x08 /* Private */,
       5,    1,   43,    2, 0x08 /* Private */,
       6,    0,   46,    2, 0x08 /* Private */,
       7,    1,   47,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QJsonObject,    4,
    QMetaType::Void, QMetaType::QJsonObject,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QJsonObject,    4,

       0        // eod
};

void ProductListWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ProductListWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onSearchClicked(); break;
        case 1: _t->onProductsReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 2: _t->onSearchResultsReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 3: _t->onAddToCartClicked(); break;
        case 4: _t->onCartUpdated((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ProductListWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ProductListWidget.data,
    qt_meta_data_ProductListWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ProductListWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProductListWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ProductListWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ProductListWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
