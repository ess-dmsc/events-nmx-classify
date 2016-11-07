/****************************************************************************
** Meta object code from reading C++ file 'ViewEvent.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ViewEvent.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ViewEvent.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ViewEvent_t {
    QByteArrayData data[16];
    char stringdata0[368];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ViewEvent_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ViewEvent_t qt_meta_stringdata_ViewEvent = {
    {
QT_MOC_LITERAL(0, 0, 9), // "ViewEvent"
QT_MOC_LITERAL(1, 10, 8), // "enableIO"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 11), // "set_indices"
QT_MOC_LITERAL(4, 32, 16), // "std::set<size_t>"
QT_MOC_LITERAL(5, 49, 7), // "indices"
QT_MOC_LITERAL(6, 57, 28), // "on_spinEventIdx_valueChanged"
QT_MOC_LITERAL(7, 86, 4), // "arg1"
QT_MOC_LITERAL(8, 91, 32), // "on_comboPlot_currentIndexChanged"
QT_MOC_LITERAL(9, 124, 35), // "on_comboOverlay_currentIndexC..."
QT_MOC_LITERAL(10, 160, 28), // "on_comboProjection_activated"
QT_MOC_LITERAL(11, 189, 34), // "on_comboPlanes_currentIndexCh..."
QT_MOC_LITERAL(12, 224, 35), // "on_comboPoint1x_currentIndexC..."
QT_MOC_LITERAL(13, 260, 35), // "on_comboPoint2x_currentIndexC..."
QT_MOC_LITERAL(14, 296, 35), // "on_comboPoint1y_currentIndexC..."
QT_MOC_LITERAL(15, 332, 35) // "on_comboPoint2y_currentIndexC..."

    },
    "ViewEvent\0enableIO\0\0set_indices\0"
    "std::set<size_t>\0indices\0"
    "on_spinEventIdx_valueChanged\0arg1\0"
    "on_comboPlot_currentIndexChanged\0"
    "on_comboOverlay_currentIndexChanged\0"
    "on_comboProjection_activated\0"
    "on_comboPlanes_currentIndexChanged\0"
    "on_comboPoint1x_currentIndexChanged\0"
    "on_comboPoint2x_currentIndexChanged\0"
    "on_comboPoint1y_currentIndexChanged\0"
    "on_comboPoint2y_currentIndexChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ViewEvent[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x0a /* Public */,
       3,    1,   72,    2, 0x0a /* Public */,
       6,    1,   75,    2, 0x08 /* Private */,
       8,    1,   78,    2, 0x08 /* Private */,
       9,    1,   81,    2, 0x08 /* Private */,
      10,    1,   84,    2, 0x08 /* Private */,
      11,    1,   87,    2, 0x08 /* Private */,
      12,    1,   90,    2, 0x08 /* Private */,
      13,    1,   93,    2, 0x08 /* Private */,
      14,    1,   96,    2, 0x08 /* Private */,
      15,    1,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,

       0        // eod
};

void ViewEvent::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ViewEvent *_t = static_cast<ViewEvent *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->enableIO((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->set_indices((*reinterpret_cast< std::set<size_t>(*)>(_a[1]))); break;
        case 2: _t->on_spinEventIdx_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_comboPlot_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->on_comboOverlay_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->on_comboProjection_activated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_comboPlanes_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->on_comboPoint1x_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->on_comboPoint2x_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->on_comboPoint1y_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->on_comboPoint2y_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ViewEvent::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ViewEvent.data,
      qt_meta_data_ViewEvent,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ViewEvent::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ViewEvent::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ViewEvent.stringdata0))
        return static_cast<void*>(const_cast< ViewEvent*>(this));
    return QWidget::qt_metacast(_clname);
}

int ViewEvent::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
