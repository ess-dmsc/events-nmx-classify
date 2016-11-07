/****************************************************************************
** Meta object code from reading C++ file 'tpcc.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../tpcc.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tpcc.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_tpcc_t {
    QByteArrayData data[20];
    char stringdata0[292];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_tpcc_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_tpcc_t qt_meta_stringdata_tpcc = {
    {
QT_MOC_LITERAL(0, 0, 4), // "tpcc"
QT_MOC_LITERAL(1, 5, 8), // "enableIO"
QT_MOC_LITERAL(2, 14, 0), // ""
QT_MOC_LITERAL(3, 15, 12), // "saveSettings"
QT_MOC_LITERAL(4, 28, 12), // "loadSettings"
QT_MOC_LITERAL(5, 41, 8), // "toggleIO"
QT_MOC_LITERAL(6, 50, 12), // "run_complete"
QT_MOC_LITERAL(7, 63, 15), // "update_progress"
QT_MOC_LITERAL(8, 79, 12), // "percent_done"
QT_MOC_LITERAL(9, 92, 13), // "table_changed"
QT_MOC_LITERAL(10, 106, 14), // "collect_params"
QT_MOC_LITERAL(11, 121, 14), // "display_params"
QT_MOC_LITERAL(12, 136, 19), // "on_pushStop_clicked"
QT_MOC_LITERAL(13, 156, 20), // "on_pushStart_clicked"
QT_MOC_LITERAL(14, 177, 23), // "on_comboGroup_activated"
QT_MOC_LITERAL(15, 201, 4), // "arg1"
QT_MOC_LITERAL(16, 206, 23), // "on_pushNewGroup_clicked"
QT_MOC_LITERAL(17, 230, 26), // "on_pushDeleteGroup_clicked"
QT_MOC_LITERAL(18, 257, 14), // "populate_combo"
QT_MOC_LITERAL(19, 272, 19) // "on_pushOpen_clicked"

    },
    "tpcc\0enableIO\0\0saveSettings\0loadSettings\0"
    "toggleIO\0run_complete\0update_progress\0"
    "percent_done\0table_changed\0collect_params\0"
    "display_params\0on_pushStop_clicked\0"
    "on_pushStart_clicked\0on_comboGroup_activated\0"
    "arg1\0on_pushNewGroup_clicked\0"
    "on_pushDeleteGroup_clicked\0populate_combo\0"
    "on_pushOpen_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_tpcc[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   97,    2, 0x08 /* Private */,
       4,    0,   98,    2, 0x08 /* Private */,
       5,    1,   99,    2, 0x08 /* Private */,
       6,    0,  102,    2, 0x08 /* Private */,
       7,    1,  103,    2, 0x08 /* Private */,
       9,    1,  106,    2, 0x08 /* Private */,
      10,    0,  109,    2, 0x08 /* Private */,
      11,    0,  110,    2, 0x08 /* Private */,
      12,    0,  111,    2, 0x08 /* Private */,
      13,    0,  112,    2, 0x08 /* Private */,
      14,    1,  113,    2, 0x08 /* Private */,
      16,    0,  116,    2, 0x08 /* Private */,
      17,    0,  117,    2, 0x08 /* Private */,
      18,    0,  118,    2, 0x08 /* Private */,
      19,    0,  119,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    8,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void tpcc::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        tpcc *_t = static_cast<tpcc *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->enableIO((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->saveSettings(); break;
        case 2: _t->loadSettings(); break;
        case 3: _t->toggleIO((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->run_complete(); break;
        case 5: _t->update_progress((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->table_changed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->collect_params(); break;
        case 8: _t->display_params(); break;
        case 9: _t->on_pushStop_clicked(); break;
        case 10: _t->on_pushStart_clicked(); break;
        case 11: _t->on_comboGroup_activated((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->on_pushNewGroup_clicked(); break;
        case 13: _t->on_pushDeleteGroup_clicked(); break;
        case 14: _t->populate_combo(); break;
        case 15: _t->on_pushOpen_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (tpcc::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&tpcc::enableIO)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject tpcc::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_tpcc.data,
      qt_meta_data_tpcc,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *tpcc::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *tpcc::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_tpcc.stringdata0))
        return static_cast<void*>(const_cast< tpcc*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int tpcc::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void tpcc::enableIO(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
