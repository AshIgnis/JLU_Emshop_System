/****************************************************************************
** Meta object code from reading C++ file 'ClientAdapter.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../qt_client/src/ClientAdapter.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClientAdapter.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN13ClientAdapterE_t {};
} // unnamed namespace

template <> constexpr inline auto ClientAdapter::qt_create_metaobjectdata<qt_meta_tag_ZN13ClientAdapterE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ClientAdapter",
        "connectionStateChanged",
        "",
        "ClientAdapter::ConnectionState",
        "state",
        "connected",
        "disconnected",
        "error",
        "authenticated",
        "userInfo",
        "authenticationFailed",
        "productsReceived",
        "data",
        "searchResultsReceived",
        "cartUpdated",
        "cartReceived",
        "ordersReceived",
        "orderDetailReceived",
        "addressesReceived",
        "addressAdded",
        "orderCreated",
        "paymentProcessed",
        "orderCancelled",
        "orderStatusUpdated",
        "orderId",
        "status",
        "message",
        "stockUpdated",
        "productId",
        "newStock",
        "priceUpdated",
        "newPrice",
        "systemNotificationReceived",
        "title",
        "content",
        "level",
        "refundProcessed",
        "orderTrackingReceived",
        "availableCouponsReceived",
        "userCouponsReceived",
        "couponClaimed",
        "cartDiscountCalculated",
        "paymentMethodsReceived",
        "paymentValidationResult"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'connectionStateChanged'
        QtMocHelpers::SignalData<void(ClientAdapter::ConnectionState)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 },
        }}),
        // Signal 'connected'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'disconnected'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'error'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'authenticated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 9 },
        }}),
        // Signal 'authenticationFailed'
        QtMocHelpers::SignalData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'productsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(11, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'searchResultsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'cartUpdated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'cartReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'ordersReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(16, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'orderDetailReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'addressesReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'addressAdded'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'orderCreated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'paymentProcessed'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(21, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'orderCancelled'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'orderStatusUpdated'
        QtMocHelpers::SignalData<void(qint64, const QString &, const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 24 }, { QMetaType::QString, 25 }, { QMetaType::QString, 26 },
        }}),
        // Signal 'stockUpdated'
        QtMocHelpers::SignalData<void(qint64, int)>(27, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 28 }, { QMetaType::Int, 29 },
        }}),
        // Signal 'priceUpdated'
        QtMocHelpers::SignalData<void(qint64, double)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::LongLong, 28 }, { QMetaType::Double, 31 },
        }}),
        // Signal 'systemNotificationReceived'
        QtMocHelpers::SignalData<void(const QString &, const QString &, const QString &)>(32, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 33 }, { QMetaType::QString, 34 }, { QMetaType::QString, 35 },
        }}),
        // Signal 'refundProcessed'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(36, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'orderTrackingReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(37, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'availableCouponsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'userCouponsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(39, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'couponClaimed'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(40, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'cartDiscountCalculated'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(41, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'paymentMethodsReceived'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(42, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
        // Signal 'paymentValidationResult'
        QtMocHelpers::SignalData<void(const QJsonObject &)>(43, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QJsonObject, 12 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ClientAdapter, qt_meta_tag_ZN13ClientAdapterE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ClientAdapter::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientAdapterE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientAdapterE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13ClientAdapterE_t>.metaTypes,
    nullptr
} };

void ClientAdapter::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ClientAdapter *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectionStateChanged((*reinterpret_cast< std::add_pointer_t<ClientAdapter::ConnectionState>>(_a[1]))); break;
        case 1: _t->connected(); break;
        case 2: _t->disconnected(); break;
        case 3: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->authenticated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 5: _t->authenticationFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->productsReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 7: _t->searchResultsReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 8: _t->cartUpdated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 9: _t->cartReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 10: _t->ordersReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 11: _t->orderDetailReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 12: _t->addressesReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 13: _t->addressAdded((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 14: _t->orderCreated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 15: _t->paymentProcessed((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 16: _t->orderCancelled((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 17: _t->orderStatusUpdated((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 18: _t->stockUpdated((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 19: _t->priceUpdated((*reinterpret_cast< std::add_pointer_t<qint64>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 20: _t->systemNotificationReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 21: _t->refundProcessed((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 22: _t->orderTrackingReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 23: _t->availableCouponsReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 24: _t->userCouponsReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 25: _t->couponClaimed((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 26: _t->cartDiscountCalculated((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 27: _t->paymentMethodsReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 28: _t->paymentValidationResult((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(ClientAdapter::ConnectionState )>(_a, &ClientAdapter::connectionStateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)()>(_a, &ClientAdapter::connected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)()>(_a, &ClientAdapter::disconnected, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QString & )>(_a, &ClientAdapter::error, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::authenticated, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QString & )>(_a, &ClientAdapter::authenticationFailed, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::productsReceived, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::searchResultsReceived, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::cartUpdated, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::cartReceived, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::ordersReceived, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::orderDetailReceived, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::addressesReceived, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::addressAdded, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::orderCreated, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::paymentProcessed, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::orderCancelled, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(qint64 , const QString & , const QString & )>(_a, &ClientAdapter::orderStatusUpdated, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(qint64 , int )>(_a, &ClientAdapter::stockUpdated, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(qint64 , double )>(_a, &ClientAdapter::priceUpdated, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QString & , const QString & , const QString & )>(_a, &ClientAdapter::systemNotificationReceived, 20))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::refundProcessed, 21))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::orderTrackingReceived, 22))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::availableCouponsReceived, 23))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::userCouponsReceived, 24))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::couponClaimed, 25))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::cartDiscountCalculated, 26))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::paymentMethodsReceived, 27))
            return;
        if (QtMocHelpers::indexOfMethod<void (ClientAdapter::*)(const QJsonObject & )>(_a, &ClientAdapter::paymentValidationResult, 28))
            return;
    }
}

const QMetaObject *ClientAdapter::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ClientAdapter::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13ClientAdapterE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ClientAdapter::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 29)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 29;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 29)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 29;
    }
    return _id;
}

// SIGNAL 0
void ClientAdapter::connectionStateChanged(ClientAdapter::ConnectionState _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
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
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ClientAdapter::authenticated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void ClientAdapter::authenticationFailed(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ClientAdapter::productsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void ClientAdapter::searchResultsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}

// SIGNAL 8
void ClientAdapter::cartUpdated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1);
}

// SIGNAL 9
void ClientAdapter::cartReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1);
}

// SIGNAL 10
void ClientAdapter::ordersReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void ClientAdapter::orderDetailReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1);
}

// SIGNAL 12
void ClientAdapter::addressesReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1);
}

// SIGNAL 13
void ClientAdapter::addressAdded(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1);
}

// SIGNAL 14
void ClientAdapter::orderCreated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void ClientAdapter::paymentProcessed(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void ClientAdapter::orderCancelled(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void ClientAdapter::orderStatusUpdated(qint64 _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1, _t2, _t3);
}

// SIGNAL 18
void ClientAdapter::stockUpdated(qint64 _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 18, nullptr, _t1, _t2);
}

// SIGNAL 19
void ClientAdapter::priceUpdated(qint64 _t1, double _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 19, nullptr, _t1, _t2);
}

// SIGNAL 20
void ClientAdapter::systemNotificationReceived(const QString & _t1, const QString & _t2, const QString & _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1, _t2, _t3);
}

// SIGNAL 21
void ClientAdapter::refundProcessed(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 21, nullptr, _t1);
}

// SIGNAL 22
void ClientAdapter::orderTrackingReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 22, nullptr, _t1);
}

// SIGNAL 23
void ClientAdapter::availableCouponsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 23, nullptr, _t1);
}

// SIGNAL 24
void ClientAdapter::userCouponsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 24, nullptr, _t1);
}

// SIGNAL 25
void ClientAdapter::couponClaimed(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 25, nullptr, _t1);
}

// SIGNAL 26
void ClientAdapter::cartDiscountCalculated(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 26, nullptr, _t1);
}

// SIGNAL 27
void ClientAdapter::paymentMethodsReceived(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 27, nullptr, _t1);
}

// SIGNAL 28
void ClientAdapter::paymentValidationResult(const QJsonObject & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 28, nullptr, _t1);
}
QT_WARNING_POP
