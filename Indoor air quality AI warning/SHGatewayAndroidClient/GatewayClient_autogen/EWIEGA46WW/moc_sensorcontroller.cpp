/****************************************************************************
** Meta object code from reading C++ file 'sensorcontroller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../sensorcontroller.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sensorcontroller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
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
struct qt_meta_tag_ZN16SensorControllerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16SensorControllerE = QtMocHelpers::stringData(
    "SensorController",
    "connectedChanged",
    "",
    "serverIpChanged",
    "serverPortChanged",
    "dataChanged",
    "thresholdChanged",
    "alertReceived",
    "action",
    "value",
    "timestamp",
    "historyReceived",
    "sensorType",
    "range",
    "dataJson",
    "historyQueryError",
    "msg",
    "connectionError",
    "errorMsg",
    "thresholdApplied",
    "success",
    "connectToServer",
    "ip",
    "port",
    "disconnectFromServer",
    "requestSensor",
    "sendControl",
    "controlType",
    "QVariantMap",
    "params",
    "applyThreshold",
    "co2Max",
    "pm25Max",
    "tempMax",
    "tempMin",
    "methaneEnable",
    "cooldownSeconds",
    "fanCooldownSeconds",
    "queryHistory",
    "onDataReceived",
    "data",
    "onServerCommand",
    "cmd",
    "onConnected",
    "onDisconnected",
    "onError",
    "error",
    "connected",
    "serverIp",
    "serverPort",
    "temperature",
    "humidity",
    "co2",
    "pm25",
    "methane",
    "light",
    "smoke",
    "fire"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16SensorControllerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      21,   14, // methods
      18,  213, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      10,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  140,    2, 0x06,   19 /* Public */,
       3,    0,  141,    2, 0x06,   20 /* Public */,
       4,    0,  142,    2, 0x06,   21 /* Public */,
       5,    0,  143,    2, 0x06,   22 /* Public */,
       6,    0,  144,    2, 0x06,   23 /* Public */,
       7,    3,  145,    2, 0x06,   24 /* Public */,
      11,    3,  152,    2, 0x06,   28 /* Public */,
      15,    1,  159,    2, 0x06,   32 /* Public */,
      17,    1,  162,    2, 0x06,   34 /* Public */,
      19,    1,  165,    2, 0x06,   36 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      21,    2,  168,    2, 0x0a,   38 /* Public */,
      24,    0,  173,    2, 0x0a,   41 /* Public */,
      25,    1,  174,    2, 0x0a,   42 /* Public */,
      26,    2,  177,    2, 0x0a,   44 /* Public */,
      30,    7,  182,    2, 0x0a,   47 /* Public */,
      38,    2,  197,    2, 0x0a,   55 /* Public */,
      39,    1,  202,    2, 0x08,   58 /* Private */,
      41,    1,  205,    2, 0x08,   60 /* Private */,
      43,    0,  208,    2, 0x08,   62 /* Private */,
      44,    0,  209,    2, 0x08,   63 /* Private */,
      45,    1,  210,    2, 0x08,   64 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Double, QMetaType::QString,    8,    9,   10,
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::QString,   12,   13,   14,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::Bool,   20,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Int,   22,   23,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 28,   27,   29,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Double, QMetaType::Double, QMetaType::Bool, QMetaType::Int, QMetaType::Int,   31,   32,   33,   34,   35,   36,   37,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,   12,   13,
    QMetaType::Void, QMetaType::QJsonObject,   40,
    QMetaType::Void, QMetaType::QJsonObject,   42,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   46,

 // properties: name, type, flags, notifyId, revision
      47, QMetaType::Bool, 0x00015001, uint(0), 0,
      48, QMetaType::QString, 0x00015001, uint(1), 0,
      49, QMetaType::Int, 0x00015001, uint(2), 0,
      50, QMetaType::Double, 0x00015001, uint(3), 0,
      51, QMetaType::Double, 0x00015001, uint(3), 0,
      52, QMetaType::Double, 0x00015001, uint(3), 0,
      53, QMetaType::Double, 0x00015001, uint(3), 0,
      54, QMetaType::Bool, 0x00015001, uint(3), 0,
      55, QMetaType::Double, 0x00015001, uint(3), 0,
      56, QMetaType::Bool, 0x00015001, uint(3), 0,
      57, QMetaType::Bool, 0x00015001, uint(3), 0,
      31, QMetaType::Int, 0x00015001, uint(4), 0,
      32, QMetaType::Int, 0x00015001, uint(4), 0,
      33, QMetaType::Double, 0x00015001, uint(4), 0,
      34, QMetaType::Double, 0x00015001, uint(4), 0,
      35, QMetaType::Bool, 0x00015001, uint(4), 0,
      36, QMetaType::Int, 0x00015001, uint(4), 0,
      37, QMetaType::Int, 0x00015001, uint(4), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SensorController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN16SensorControllerE.offsetsAndSizes,
    qt_meta_data_ZN16SensorControllerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16SensorControllerE_t,
        // property 'connected'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'serverIp'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'serverPort'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'temperature'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'humidity'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'co2'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'pm25'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'methane'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'light'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'smoke'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'fire'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'co2Max'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'pm25Max'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'tempMax'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'tempMin'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'methaneEnable'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'cooldownSeconds'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'fanCooldownSeconds'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SensorController, std::true_type>,
        // method 'connectedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'serverIpChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'serverPortChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'thresholdChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'alertReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'historyReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'historyQueryError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'connectionError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'thresholdApplied'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'connectToServer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'disconnectFromServer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'requestSensor'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'sendControl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVariantMap &, std::false_type>,
        // method 'applyThreshold'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'queryHistory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onDataReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'onServerCommand'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'onConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void SensorController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SensorController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->connectedChanged(); break;
        case 1: _t->serverIpChanged(); break;
        case 2: _t->serverPortChanged(); break;
        case 3: _t->dataChanged(); break;
        case 4: _t->thresholdChanged(); break;
        case 5: _t->alertReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 6: _t->historyReceived((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 7: _t->historyQueryError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->connectionError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->thresholdApplied((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->connectToServer((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->disconnectFromServer(); break;
        case 12: _t->requestSensor((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->sendControl((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[2]))); break;
        case 14: _t->applyThreshold((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[7]))); break;
        case 15: _t->queryHistory((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 16: _t->onDataReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 17: _t->onServerCommand((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 18: _t->onConnected(); break;
        case 19: _t->onDisconnected(); break;
        case 20: _t->onError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (SensorController::*)();
            if (_q_method_type _q_method = &SensorController::connectedChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)();
            if (_q_method_type _q_method = &SensorController::serverIpChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)();
            if (_q_method_type _q_method = &SensorController::serverPortChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)();
            if (_q_method_type _q_method = &SensorController::dataChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)();
            if (_q_method_type _q_method = &SensorController::thresholdChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)(const QString & , double , const QString & );
            if (_q_method_type _q_method = &SensorController::alertReceived; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)(int , const QString & , const QString & );
            if (_q_method_type _q_method = &SensorController::historyReceived; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)(const QString & );
            if (_q_method_type _q_method = &SensorController::historyQueryError; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)(const QString & );
            if (_q_method_type _q_method = &SensorController::connectionError; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _q_method_type = void (SensorController::*)(bool );
            if (_q_method_type _q_method = &SensorController::thresholdApplied; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->isConnected(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->serverIp(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->serverPort(); break;
        case 3: *reinterpret_cast< double*>(_v) = _t->temperature(); break;
        case 4: *reinterpret_cast< double*>(_v) = _t->humidity(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->co2(); break;
        case 6: *reinterpret_cast< double*>(_v) = _t->pm25(); break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->methane(); break;
        case 8: *reinterpret_cast< double*>(_v) = _t->light(); break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->smoke(); break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->fire(); break;
        case 11: *reinterpret_cast< int*>(_v) = _t->co2Max(); break;
        case 12: *reinterpret_cast< int*>(_v) = _t->pm25Max(); break;
        case 13: *reinterpret_cast< double*>(_v) = _t->tempMax(); break;
        case 14: *reinterpret_cast< double*>(_v) = _t->tempMin(); break;
        case 15: *reinterpret_cast< bool*>(_v) = _t->methaneEnable(); break;
        case 16: *reinterpret_cast< int*>(_v) = _t->cooldownSeconds(); break;
        case 17: *reinterpret_cast< int*>(_v) = _t->fanCooldownSeconds(); break;
        default: break;
        }
    }
}

const QMetaObject *SensorController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SensorController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16SensorControllerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SensorController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 21)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 21;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 21)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 21;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void SensorController::connectedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SensorController::serverIpChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SensorController::serverPortChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SensorController::dataChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SensorController::thresholdChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SensorController::alertReceived(const QString & _t1, double _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void SensorController::historyReceived(int _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void SensorController::historyQueryError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void SensorController::connectionError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void SensorController::thresholdApplied(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}
QT_WARNING_POP
