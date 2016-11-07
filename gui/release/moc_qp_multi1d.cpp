/****************************************************************************
** Meta object code from reading C++ file 'qp_multi1d.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../plots/qp_multi1d.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qp_multi1d.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QPlot__Multi1D_t {
    QByteArrayData data[9];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QPlot__Multi1D_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QPlot__Multi1D_t qt_meta_stringdata_QPlot__Multi1D = {
    {
QT_MOC_LITERAL(0, 0, 14), // "QPlot::Multi1D"
QT_MOC_LITERAL(1, 15, 11), // "clickedLeft"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 12), // "clickedRight"
QT_MOC_LITERAL(4, 41, 7), // "zoomOut"
QT_MOC_LITERAL(5, 49, 12), // "mousePressed"
QT_MOC_LITERAL(6, 62, 12), // "QMouseEvent*"
QT_MOC_LITERAL(7, 75, 13), // "mouseReleased"
QT_MOC_LITERAL(8, 89, 7) // "adjustY"

    },
    "QPlot::Multi1D\0clickedLeft\0\0clickedRight\0"
    "zoomOut\0mousePressed\0QMouseEvent*\0"
    "mouseReleased\0adjustY"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QPlot__Multi1D[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       3,    1,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   50,    2, 0x0a /* Public */,
       5,    1,   51,    2, 0x09 /* Protected */,
       7,    1,   54,    2, 0x09 /* Protected */,
       8,    0,   57,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void,

       0        // eod
};

void QPlot::Multi1D::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Multi1D *_t = static_cast<Multi1D *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->clickedLeft((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->clickedRight((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->zoomOut(); break;
        case 3: _t->mousePressed((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 4: _t->mouseReleased((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 5: _t->adjustY(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Multi1D::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Multi1D::clickedLeft)) {
                *result = 0;
            }
        }
        {
            typedef void (Multi1D::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Multi1D::clickedRight)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject QPlot::Multi1D::staticMetaObject = {
    { &GenericPlot::staticMetaObject, qt_meta_stringdata_QPlot__Multi1D.data,
      qt_meta_data_QPlot__Multi1D,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QPlot::Multi1D::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QPlot::Multi1D::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QPlot__Multi1D.stringdata0))
        return static_cast<void*>(const_cast< Multi1D*>(this));
    return GenericPlot::qt_metacast(_clname);
}

int QPlot::Multi1D::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GenericPlot::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void QPlot::Multi1D::clickedLeft(double _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QPlot::Multi1D::clickedRight(double _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
