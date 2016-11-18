CONFIG += debug_and_release

!include( $$PWD/../common/common.pri ) {
  error( "Couldn't find the common.pri file!" )
}

TARGET   = $$PWD/../nmx_browse
TEMPLATE = app

UI_DIR = $$PWD/ui


CONFIG(debug, debug|release) {
  TARGET = $$join(TARGET,,,d)
}

DEFINES += QCUSTOMPLOT_USE_OPENGL

INSTALLS += target

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

INCLUDEPATH += $$PWD \
               $$PWD/qtcolorpicker \
               $$PWD/plots \
               $$PWD/plots/qcustomplot

SOURCES += $$files($$PWD/*.cpp) \
           $$files($$PWD/qtcolorpicker/*.cpp) \
           $$files($$PWD/plots/*.cpp) \
           $$files($$PWD/plots/qcustomplot/*.cpp)

HEADERS  += $$files($$PWD/*.h) \
            $$files($$PWD/qtcolorpicker/*.h) \
            $$files($$PWD/plots/*.h) \
            $$files($$PWD/plots/qcustomplot/*.h)

FORMS   += $$files($$PWD/*.ui) \
           $$files($$PWD/plots/*.ui)

RESOURCES += $$files($$PWD/resources/*.qrc)
