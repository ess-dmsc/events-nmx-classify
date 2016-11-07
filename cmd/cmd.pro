CONFIG += debug_and_release

!include( $$PWD/../common.pri ) {
  error( "Couldn't find the common.pri file!" )
}

TARGET   = $$PWD/../tpcc
TEMPLATE = app

CONFIG(debug, debug|release) {
  TARGET = $$join(TARGET,,,d)
}

DEFINES += QCUSTOMPLOT_USE_OPENGL

INSTALLS += target

INCLUDEPATH += $$PWD 

SOURCES += $$files($$PWD/*.cpp)

HEADERS  += $$files($$PWD/*.h)
