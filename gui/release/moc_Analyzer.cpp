/****************************************************************************
** Meta object code from reading C++ file 'Analyzer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../Analyzer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Analyzer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_Analyzer_t {
    QByteArrayData data[27];
    char stringdata0[509];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Analyzer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Analyzer_t qt_meta_stringdata_Analyzer = {
    {
QT_MOC_LITERAL(0, 0, 8), // "Analyzer"
QT_MOC_LITERAL(1, 9, 8), // "toggleIO"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 14), // "select_indices"
QT_MOC_LITERAL(4, 34, 16), // "std::set<size_t>"
QT_MOC_LITERAL(5, 51, 8), // "enableIO"
QT_MOC_LITERAL(6, 60, 10), // "plot_boxes"
QT_MOC_LITERAL(7, 71, 14), // "parameters_set"
QT_MOC_LITERAL(8, 86, 36), // "on_comboWeightsX_currentIndex..."
QT_MOC_LITERAL(9, 123, 4), // "arg1"
QT_MOC_LITERAL(10, 128, 36), // "on_comboWeightsY_currentIndex..."
QT_MOC_LITERAL(11, 165, 36), // "on_comboWeightsZ_currentIndex..."
QT_MOC_LITERAL(12, 202, 27), // "on_spinMinX_editingFinished"
QT_MOC_LITERAL(13, 230, 27), // "on_spinMaxX_editingFinished"
QT_MOC_LITERAL(14, 258, 27), // "on_spinMinY_editingFinished"
QT_MOC_LITERAL(15, 286, 27), // "on_spinMaxY_editingFinished"
QT_MOC_LITERAL(16, 314, 27), // "on_spinMinZ_editingFinished"
QT_MOC_LITERAL(17, 342, 27), // "on_spinMaxZ_editingFinished"
QT_MOC_LITERAL(18, 370, 21), // "on_pushAddBox_clicked"
QT_MOC_LITERAL(19, 392, 10), // "update_box"
QT_MOC_LITERAL(20, 403, 1), // "x"
QT_MOC_LITERAL(21, 405, 1), // "y"
QT_MOC_LITERAL(22, 407, 15), // "Qt::MouseButton"
QT_MOC_LITERAL(23, 423, 10), // "left_mouse"
QT_MOC_LITERAL(24, 434, 24), // "on_spinMinZ_valueChanged"
QT_MOC_LITERAL(25, 459, 24), // "on_spinMaxZ_valueChanged"
QT_MOC_LITERAL(26, 484, 24) // "on_pushRemoveBox_clicked"

    },
    "Analyzer\0toggleIO\0\0select_indices\0"
    "std::set<size_t>\0enableIO\0plot_boxes\0"
    "parameters_set\0on_comboWeightsX_currentIndexChanged\0"
    "arg1\0on_comboWeightsY_currentIndexChanged\0"
    "on_comboWeightsZ_currentIndexChanged\0"
    "on_spinMinX_editingFinished\0"
    "on_spinMaxX_editingFinished\0"
    "on_spinMinY_editingFinished\0"
    "on_spinMaxY_editingFinished\0"
    "on_spinMinZ_editingFinished\0"
    "on_spinMaxZ_editingFinished\0"
    "on_pushAddBox_clicked\0update_box\0x\0y\0"
    "Qt::MouseButton\0left_mouse\0"
    "on_spinMinZ_valueChanged\0"
    "on_spinMaxZ_valueChanged\0"
    "on_pushRemoveBox_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Analyzer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  109,    2, 0x06 /* Public */,
       3,    1,  112,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,  115,    2, 0x0a /* Public */,
       6,    0,  118,    2, 0x08 /* Private */,
       7,    0,  119,    2, 0x08 /* Private */,
       8,    1,  120,    2, 0x08 /* Private */,
      10,    1,  123,    2, 0x08 /* Private */,
      11,    1,  126,    2, 0x08 /* Private */,
      12,    0,  129,    2, 0x08 /* Private */,
      13,    0,  130,    2, 0x08 /* Private */,
      14,    0,  131,    2, 0x08 /* Private */,
      15,    0,  132,    2, 0x08 /* Private */,
      16,    0,  133,    2, 0x08 /* Private */,
      17,    0,  134,    2, 0x08 /* Private */,
      18,    0,  135,    2, 0x08 /* Private */,
      19,    3,  136,    2, 0x08 /* Private */,
      24,    1,  143,    2, 0x08 /* Private */,
      25,    1,  146,    2, 0x08 /* Private */,
      26,    0,  149,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, 0x80000000 | 4,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double, 0x80000000 | 22,   20,   21,   23,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,

       0        // eod
};

void Analyzer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Analyzer *_t = static_cast<Analyzer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->toggleIO((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->select_indices((*reinterpret_cast< std::set<size_t>(*)>(_a[1]))); break;
        case 2: _t->enableIO((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->plot_boxes(); break;
        case 4: _t->parameters_set(); break;
        case 5: _t->on_comboWeightsX_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->on_comboWeightsY_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->on_comboWeightsZ_currentIndexChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->on_spinMinX_editingFinished(); break;
        case 9: _t->on_spinMaxX_editingFinished(); break;
        case 10: _t->on_spinMinY_editingFinished(); break;
        case 11: _t->on_spinMaxY_editingFinished(); break;
        case 12: _t->on_spinMinZ_editingFinished(); break;
        case 13: _t->on_spinMaxZ_editingFinished(); break;
        case 14: _t->on_pushAddBox_clicked(); break;
        case 15: _t->update_box((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3]))); break;
        case 16: _t->on_spinMinZ_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 17: _t->on_spinMaxZ_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->on_pushRemoveBox_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (Analyzer::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Analyzer::toggleIO)) {
                *result = 0;
            }
        }
        {
            typedef void (Analyzer::*_t)(std::set<size_t> );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&Analyzer::select_indices)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject Analyzer::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Analyzer.data,
      qt_meta_data_Analyzer,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Analyzer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Analyzer::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Analyzer.stringdata0))
        return static_cast<void*>(const_cast< Analyzer*>(this));
    return QWidget::qt_metacast(_clname);
}

int Analyzer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void Analyzer::toggleIO(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Analyzer::select_indices(std::set<size_t> _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
