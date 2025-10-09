/****************************************************************************
** Meta object code from reading C++ file 'AdminTab.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/ui/tabs/AdminTab.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AdminTab.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN8AdminTabE_t {};
} // unnamed namespace

template <> constexpr inline auto AdminTab::qt_create_metaobjectdata<qt_meta_tag_ZN8AdminTabE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AdminTab",
        "statusMessage",
        "",
        "message",
        "success",
        "refreshLowStock",
        "applyStockChange",
        "refreshAllOrders",
        "prevPage",
        "nextPage",
        "changeOrderStatus",
        "orderId",
        "viewOrderDetail",
        "refundOrder",
        "refreshPromotions",
        "createPromotion",
        "refreshUsers",
        "showSelectedUserDetail",
        "applyUserRole",
        "toggleUserStatus",
        "issueCouponToUser",
        "fetchUserOrders",
        "fetchUserCoupons"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'statusMessage'
        QtMocHelpers::SignalData<void(const QString &, bool)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 4 },
        }}),
        // Slot 'refreshLowStock'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'applyStockChange'
        QtMocHelpers::SlotData<void()>(6, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'refreshAllOrders'
        QtMocHelpers::SlotData<void()>(7, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'prevPage'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'nextPage'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'changeOrderStatus'
        QtMocHelpers::SlotData<void(qlonglong)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 11 },
        }}),
        // Slot 'viewOrderDetail'
        QtMocHelpers::SlotData<void(qlonglong)>(12, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 11 },
        }}),
        // Slot 'refundOrder'
        QtMocHelpers::SlotData<void(qlonglong)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::LongLong, 11 },
        }}),
        // Slot 'refreshPromotions'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'createPromotion'
        QtMocHelpers::SlotData<void()>(15, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'refreshUsers'
        QtMocHelpers::SlotData<void()>(16, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showSelectedUserDetail'
        QtMocHelpers::SlotData<void()>(17, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'applyUserRole'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'toggleUserStatus'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'issueCouponToUser'
        QtMocHelpers::SlotData<void()>(20, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'fetchUserOrders'
        QtMocHelpers::SlotData<void()>(21, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'fetchUserCoupons'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<AdminTab, qt_meta_tag_ZN8AdminTabE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AdminTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8AdminTabE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8AdminTabE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN8AdminTabE_t>.metaTypes,
    nullptr
} };

void AdminTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AdminTab *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->statusMessage((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 1: _t->refreshLowStock(); break;
        case 2: _t->applyStockChange(); break;
        case 3: _t->refreshAllOrders(); break;
        case 4: _t->prevPage(); break;
        case 5: _t->nextPage(); break;
        case 6: _t->changeOrderStatus((*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 7: _t->viewOrderDetail((*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 8: _t->refundOrder((*reinterpret_cast< std::add_pointer_t<qlonglong>>(_a[1]))); break;
        case 9: _t->refreshPromotions(); break;
        case 10: _t->createPromotion(); break;
        case 11: _t->refreshUsers(); break;
        case 12: _t->showSelectedUserDetail(); break;
        case 13: _t->applyUserRole(); break;
        case 14: _t->toggleUserStatus(); break;
        case 15: _t->issueCouponToUser(); break;
        case 16: _t->fetchUserOrders(); break;
        case 17: _t->fetchUserCoupons(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AdminTab::*)(const QString & , bool )>(_a, &AdminTab::statusMessage, 0))
            return;
    }
}

const QMetaObject *AdminTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AdminTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN8AdminTabE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AdminTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void AdminTab::statusMessage(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}
QT_WARNING_POP
