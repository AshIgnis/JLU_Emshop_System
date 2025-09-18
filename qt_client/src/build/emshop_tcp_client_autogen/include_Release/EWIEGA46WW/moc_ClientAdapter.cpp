/****************************************************************************
** Meta object code from reading C++ file 'ClientAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../ClientAdapter.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClientAdapter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ClientAdapter_t {
    QByteArrayData data[36];
    char stringdata0[471];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ClientAdapter_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ClientAdapter_t qt_meta_stringdata_ClientAdapter = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ClientAdapter"
QT_MOC_LITERAL(1, 14, 22), // "connectionStateChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 30), // "ClientAdapter::ConnectionState"
QT_MOC_LITERAL(4, 69, 5), // "state"
QT_MOC_LITERAL(5, 75, 9), // "connected"
QT_MOC_LITERAL(6, 85, 12), // "disconnected"
QT_MOC_LITERAL(7, 98, 5), // "error"
QT_MOC_LITERAL(8, 104, 13), // "authenticated"
QT_MOC_LITERAL(9, 118, 8), // "userInfo"
QT_MOC_LITERAL(10, 127, 20), // "authenticationFailed"
QT_MOC_LITERAL(11, 148, 16), // "productsReceived"
QT_MOC_LITERAL(12, 165, 4), // "data"
QT_MOC_LITERAL(13, 170, 21), // "searchResultsReceived"
QT_MOC_LITERAL(14, 192, 11), // "cartUpdated"
QT_MOC_LITERAL(15, 204, 12), // "cartReceived"
QT_MOC_LITERAL(16, 217, 14), // "ordersReceived"
QT_MOC_LITERAL(17, 232, 19), // "orderDetailReceived"
QT_MOC_LITERAL(18, 252, 17), // "addressesReceived"
QT_MOC_LITERAL(19, 270, 12), // "addressAdded"
QT_MOC_LITERAL(20, 283, 12), // "orderCreated"
QT_MOC_LITERAL(21, 296, 16), // "paymentProcessed"
QT_MOC_LITERAL(22, 313, 14), // "orderCancelled"
QT_MOC_LITERAL(23, 328, 18), // "orderStatusUpdated"
QT_MOC_LITERAL(24, 347, 7), // "orderId"
QT_MOC_LITERAL(25, 355, 6), // "status"
QT_MOC_LITERAL(26, 362, 7), // "message"
QT_MOC_LITERAL(27, 370, 12), // "stockUpdated"
QT_MOC_LITERAL(28, 383, 9), // "productId"
QT_MOC_LITERAL(29, 393, 8), // "newStock"
QT_MOC_LITERAL(30, 402, 12), // "priceUpdated"
QT_MOC_LITERAL(31, 415, 8), // "newPrice"
QT_MOC_LITERAL(32, 424, 26), // "systemNotificationReceived"
QT_MOC_LITERAL(33, 451, 5), // "title"
QT_MOC_LITERAL(34, 457, 7), // "content"
QT_MOC_LITERAL(35, 465, 5) // "level"

    },
    "ClientAdapter\0connectionStateChanged\0"
    "\0ClientAdapter::ConnectionState\0state\0"
    "connected\0disconnected\0error\0authenticated\0"
    "userInfo\0authenticationFailed\0"
    "productsReceived\0data\0searchResultsReceived\0"
    "cartUpdated\0cartReceived\0ordersReceived\0"
    "orderDetailReceived\0addressesReceived\0"
    "addressAdded\0orderCreated\0paymentProcessed\0"
    "orderCancelled\0orderStatusUpdated\0"
    "orderId\0status\0message\0stockUpdated\0"
    "productId\0newStock\0priceUpdated\0"
    "newPrice\0systemNotificationReceived\0"
    "title\0content\0level"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ClientAdapter[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      21,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  119,    2, 0x06 /* Public */,
       5,    0,  122,    2, 0x06 /* Public */,
       6,    0,  123,    2, 0x06 /* Public */,
       7,    1,  124,    2, 0x06 /* Public */,
       8,    1,  127,    2, 0x06 /* Public */,
      10,    1,  130,    2, 0x06 /* Public */,
      11,    1,  133,    2, 0x06 /* Public */,
      13,    1,  136,    2, 0x06 /* Public */,
      14,    1,  139,    2, 0x06 /* Public */,
      15,    1,  142,    2, 0x06 /* Public */,
      16,    1,  145,    2, 0x06 /* Public */,
      17,    1,  148,    2, 0x06 /* Public */,
      18,    1,  151,    2, 0x06 /* Public */,
      19,    1,  154,    2, 0x06 /* Public */,
      20,    1,  157,    2, 0x06 /* Public */,
      21,    1,  160,    2, 0x06 /* Public */,
      22,    1,  163,    2, 0x06 /* Public */,
      23,    3,  166,    2, 0x06 /* Public */,
      27,    2,  173,    2, 0x06 /* Public */,
      30,    2,  178,    2, 0x06 /* Public */,
      32,    3,  183,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QJsonObject,    9,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::QJsonObject,   12,
    QMetaType::Void, QMetaType::LongLong, QMetaType::QString, QMetaType::QString,   24,   25,   26,
    QMetaType::Void, QMetaType::LongLong, QMetaType::Int,   28,   29,
    QMetaType::Void, QMetaType::LongLong, QMetaType::Double,   28,   31,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::QString,   33,   34,   35,

       0        // eod
};

void ClientAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ClientAdapter *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionStateChanged((*reinterpret_cast< ClientAdapter::ConnectionState(*)>(_a[1]))); break;
        case 1: _t->connected(); break;
        case 2: _t->disconnected(); break;
        case 3: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->authenticated((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 5: _t->authenticationFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->productsReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 7: _t->searchResultsReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 8: _t->cartUpdated((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 9: _t->cartReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 10: _t->ordersReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 11: _t->orderDetailReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 12: _t->addressesReceived((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 13: _t->addressAdded((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 14: _t->orderCreated((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 15: _t->paymentProcessed((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 16: _t->orderCancelled((*reinterpret_cast< const QJsonObject(*)>(_a[1]))); break;
        case 17: _t->orderStatusUpdated((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 18: _t->stockUpdated((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 19: _t->priceUpdated((*reinterpret_cast< qint64(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 20: _t->systemNotificationReceived((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ClientAdapter::*)(ClientAdapter::ConnectionState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::connectionStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::connected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::disconnected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::error)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::authenticated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::authenticationFailed)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::productsReceived)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::searchResultsReceived)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::cartUpdated)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::cartReceived)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::ordersReceived)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::orderDetailReceived)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::addressesReceived)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::addressAdded)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::orderCreated)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::paymentProcessed)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::orderCancelled)) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(qint64 , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::orderStatusUpdated)) {
                *result = 17;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(qint64 , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::stockUpdated)) {
                *result = 18;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(qint64 , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::priceUpdated)) {
                *result = 19;
                return;
            }
        }
        {
            using _t = void (ClientAdapter::*)(const QString & , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ClientAdapter::systemNotificationReceived)) {
                *result = 20;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ClientAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ClientAdapter.data,
    qt_meta_data_ClientAdapter,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ClientAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ClientAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ClientAdapter.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ClientAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 21;
    }
    return _id;
}

// SIGNAL 0
void ClientAdapter::connectionStateChanged(ClientAdapter::ConnectionState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ClientAdapter::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ClientAdapter::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ClientAdapter::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void ClientAdapter::authenticated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void ClientAdapter::authenticationFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void ClientAdapter::productsReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void ClientAdapter::searchResultsReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void ClientAdapter::cartUpdated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void ClientAdapter::cartReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void ClientAdapter::ordersReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void ClientAdapter::orderDetailReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void ClientAdapter::addressesReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void ClientAdapter::addressAdded(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void ClientAdapter::orderCreated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void ClientAdapter::paymentProcessed(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void ClientAdapter::orderCancelled(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void ClientAdapter::orderStatusUpdated(qint64 _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void ClientAdapter::stockUpdated(qint64 _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void ClientAdapter::priceUpdated(qint64 _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void ClientAdapter::systemNotificationReceived(const QString & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
