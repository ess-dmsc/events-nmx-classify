/****************************************************************************
** Meta object code from reading C++ file 'SpecialDelegate.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../SpecialDelegate.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SpecialDelegate.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_SpecialDelegate_t {
    QByteArrayData data[8];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SpecialDelegate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SpecialDelegate_t qt_meta_stringdata_SpecialDelegate = {
    {
QT_MOC_LITERAL(0, 0, 15), // "SpecialDelegate"
QT_MOC_LITERAL(1, 16, 13), // "begin_editing"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 12), // "edit_integer"
QT_MOC_LITERAL(4, 44, 5), // "index"
QT_MOC_LITERAL(5, 50, 5), // "value"
QT_MOC_LITERAL(6, 56, 4), // "role"
QT_MOC_LITERAL(7, 61, 15) // "intValueChanged"

    },
    "SpecialDelegate\0begin_editing\0\0"
    "edit_integer\0index\0value\0role\0"
    "intValueChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SpecialDelegate[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,
       3,    3,   30,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   37,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex, QMetaType::QVariant, QMetaType::Int,    4,    5,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void SpecialDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SpecialDelegate *_t = static_cast<SpecialDelegate *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->begin_editing(); break;
        case 1: _t->edit_integer((*reinterpret_cast< QModelIndex(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 2: _t->intValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SpecialDelegate::*_t)() const;
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SpecialDelegate::begin_editing)) {
                *result = 0;
            }
        }
        {
            typedef void (SpecialDelegate::*_t)(QModelIndex , QVariant , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SpecialDelegate::edit_integer)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject SpecialDelegate::staticMetaObject = {
    { &QStyledItemDelegate::staticMetaObject, qt_meta_stringdata_SpecialDelegate.data,
      qt_meta_data_SpecialDelegate,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SpecialDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SpecialDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SpecialDelegate.stringdata0))
        return static_cast<void*>(const_cast< SpecialDelegate*>(this));
    return QStyledItemDelegate::qt_metacast(_clname);
}

int SpecialDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void SpecialDelegate::begin_editing()const
{
    QMetaObject::activate(const_cast< SpecialDelegate *>(this), &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void SpecialDelegate::edit_integer(QModelIndex _t1, QVariant _t2, int _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
