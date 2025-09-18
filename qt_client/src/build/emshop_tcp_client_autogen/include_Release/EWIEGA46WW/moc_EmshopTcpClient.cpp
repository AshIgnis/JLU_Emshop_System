/****************************************************************************
** Meta object code from reading C++ file 'EmshopTcpClient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../EmshopTcpClient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EmshopTcpClient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_EmshopTcpClient_t {
    QByteArrayData data[43];
    char stringdata0[577];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EmshopTcpClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EmshopTcpClient_t qt_meta_stringdata_EmshopTcpClient = {
    {
QT_MOC_LITERAL(0, 0, 15), // "EmshopTcpClient"
QT_MOC_LITERAL(1, 16, 22), // "connectionStateChanged"
QT_MOC_LITERAL(2, 39, 0), // ""
QT_MOC_LITERAL(3, 40, 32), // "EmshopTcpClient::ConnectionState"
QT_MOC_LITERAL(4, 73, 5), // "state"
QT_MOC_LITERAL(5, 79, 9), // "connected"
QT_MOC_LITERAL(6, 89, 12), // "disconnected"
QT_MOC_LITERAL(7, 102, 5), // "error"
QT_MOC_LITERAL(8, 108, 13), // "authenticated"
QT_MOC_LITERAL(9, 122, 8), // "userInfo"
QT_MOC_LITERAL(10, 131, 20), // "authenticationFailed"
QT_MOC_LITERAL(11, 152, 16), // "productsReceived"
QT_MOC_LITERAL(12, 169, 4), // "data"
QT_MOC_LITERAL(13, 174, 21), // "searchResultsReceived"
QT_MOC_LITERAL(14, 196, 11), // "cartUpdated"
QT_MOC_LITERAL(15, 208, 12), // "cartReceived"
QT_MOC_LITERAL(16, 221, 14), // "ordersReceived"
QT_MOC_LITERAL(17, 236, 19), // "orderDetailReceived"
QT_MOC_LITERAL(18, 256, 17), // "addressesReceived"
QT_MOC_LITERAL(19, 274, 12), // "addressAdded"
QT_MOC_LITERAL(20, 287, 12), // "orderCreated"
QT_MOC_LITERAL(21, 300, 16), // "paymentProcessed"
QT_MOC_LITERAL(22, 317, 14), // "orderCancelled"
QT_MOC_LITERAL(23, 332, 18), // "orderStatusUpdated"
QT_MOC_LITERAL(24, 351, 7), // "orderId"
QT_MOC_LITERAL(25, 359, 6), // "status"
QT_MOC_LITERAL(26, 366, 7), // "message"
QT_MOC_LITERAL(27, 374, 12), // "stockUpdated"
QT_MOC_LITERAL(28, 387, 9), // "productId"
QT_MOC_LITERAL(29, 397, 8), // "newStock"
QT_MOC_LITERAL(30, 406, 12), // "priceUpdated"
QT_MOC_LITERAL(31, 419, 8), // "newPrice"
QT_MOC_LITERAL(32, 428, 26), // "systemNotificationReceived"
QT_MOC_LITERAL(33, 455, 5), // "title"
QT_MOC_LITERAL(34, 461, 7), // "content"
QT_MOC_LITERAL(35, 469, 5), // "level"
QT_MOC_LITERAL(36, 475, 11), // "onConnected"
QT_MOC_LITERAL(37, 487, 14), // "onDisconnected"
QT_MOC_LITERAL(38, 502, 11), // "onReadyRead"
QT_MOC_LITERAL(39, 514, 7), // "onError"
QT_MOC_LITERAL(40, 522, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(41, 551, 11), // "socketError"
QT_MOC_LITERAL(42, 563, 13) // "sendHeartbeat"

    },
    "EmshopTcpClient\0connectionStateChanged\0"
    "\0EmshopTcpClient::ConnectionState\0"
    "state\0connected\0disconnected\0error\0"
    "authenticated\0userInfo\0authenticationFailed\0"
    "productsReceived\0data\0searchResultsReceived\0"
    "cartUpdated\0cartReceived\0ordersReceived\0"
    "orderDetailReceived\0addressesReceived\0"
    "addressAdded\0orderCreated\0paymentProcessed\0"
    "orderCancelled\0orderStatusUpdated\0"
    "orderId\0status\0message\0stockUpdated\0"
    "productId\0newStock\0priceUpdated\0"
    "newPrice\0systemNotificationReceived\0"
    "title\0content\0level\0onConnected\0"
    "onDisconnected\0onReadyRead\0onError\0"
    "QAbstractSocket::SocketError\0socketError\0"
    "sendHeartbeat"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EmshopTcpClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      21,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  144,    2, 0x06 /* Public */,
       5,    0,  147,    2, 0x06 /* Public */,
       6,    0,  148,    2, 0x06 /* Public */,
       7,    1,  149,    2, 0x06 /* Public */,
       8,    1,  152,    2, 0x06 /* Public */,
      10,    1,  155,    2, 0x06 /* Public */,
      11,    1,  158,    2, 0x06 /* Public */,
      13,    1,  161,    2, 0x06 /* Public */,
      14,    1,  164,    2, 0x06 /* Public */,
      15,    1,  167,    2, 0x06 /* Public */,
      16,    1,  170,    2, 0x06 /* Public */,
      17,    1,  173,    2, 0x06 /* Public */,
      18,    1,  176,    2, 0x06 /* Public */,
      19,    1,  179,    2, 0x06 /* Public */,
      20,    1,  182,    2, 0x06 /* Public */,
      21,    1,  185,    2, 0x06 /* Public */,
      22,    1,  188,    2, 0x06 /* Public */,
      23,    3,  191,    2, 0x06 /* Public */,
      27,    2,  198,    2, 0x06 /* Public */,
      30,    2,  203,    2, 0x06 /* Public */,
      32,    3,  208,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      36,    0,  215,    2, 0x08 /* Private */,
      37,    0,  216,    2, 0x08 /* Private */,
      38,    0,  217,    2, 0x08 /* Private */,
      39,    1,  218,    2, 0x08 /* Private */,
      42,    0,  221,    2, 0x08 /* Private */,

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

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 40,   41,
    QMetaType::Void,

       0        // eod
};

void EmshopTcpClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EmshopTcpClient *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->connectionStateChanged((*reinterpret_cast< EmshopTcpClient::ConnectionState(*)>(_a[1]))); break;
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
        case 21: _t->onConnected(); break;
        case 22: _t->onDisconnected(); break;
        case 23: _t->onReadyRead(); break;
        case 24: _t->onError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 25: _t->sendHeartbeat(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 24:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EmshopTcpClient::*)(EmshopTcpClient::ConnectionState );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::connectionStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::connected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::disconnected)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::error)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::authenticated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::authenticationFailed)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::productsReceived)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::searchResultsReceived)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::cartUpdated)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::cartReceived)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::ordersReceived)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::orderDetailReceived)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::addressesReceived)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::addressAdded)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::orderCreated)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::paymentProcessed)) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QJsonObject & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::orderCancelled)) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(qint64 , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::orderStatusUpdated)) {
                *result = 17;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(qint64 , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::stockUpdated)) {
                *result = 18;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(qint64 , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::priceUpdated)) {
                *result = 19;
                return;
            }
        }
        {
            using _t = void (EmshopTcpClient::*)(const QString & , const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EmshopTcpClient::systemNotificationReceived)) {
                *result = 20;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject EmshopTcpClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_EmshopTcpClient.data,
    qt_meta_data_EmshopTcpClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EmshopTcpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EmshopTcpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EmshopTcpClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int EmshopTcpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void EmshopTcpClient::connectionStateChanged(EmshopTcpClient::ConnectionState _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void EmshopTcpClient::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void EmshopTcpClient::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void EmshopTcpClient::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void EmshopTcpClient::authenticated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void EmshopTcpClient::authenticationFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void EmshopTcpClient::productsReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void EmshopTcpClient::searchResultsReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void EmshopTcpClient::cartUpdated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void EmshopTcpClient::cartReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void EmshopTcpClient::ordersReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void EmshopTcpClient::orderDetailReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void EmshopTcpClient::addressesReceived(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void EmshopTcpClient::addressAdded(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void EmshopTcpClient::orderCreated(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void EmshopTcpClient::paymentProcessed(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void EmshopTcpClient::orderCancelled(const QJsonObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void EmshopTcpClient::orderStatusUpdated(qint64 _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void EmshopTcpClient::stockUpdated(qint64 _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void EmshopTcpClient::priceUpdated(qint64 _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void EmshopTcpClient::systemNotificationReceived(const QString & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 20, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
