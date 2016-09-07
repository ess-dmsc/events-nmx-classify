CONFIG += debug_and_release

BADDIRS = "debug release ui"

UI_DIR = $$PWD/ui

QMAKE_CLEAN += -r $$BADDIRS \
               $$files(tpcClassify.pro.user*) \
               install

CONFIG -= warn_off warn_on
CONFIG += static

QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK
QMAKE_CXXFLAGS_DEBUG += -O0 -Wextra

unix {
  LIBPATH += /usr/local/lib
  LIBS += -lm -ldl -lz \
          -lboost_system -lboost_thread -lboost_log \
          -lboost_log_setup -lhdf5_cpp

  target.path = /usr/local/bin/
      
  !mac {
    CONFIG -= c++11
    QMAKE_CXXFLAGS += -std=c++11
    H5SERIAL = $$system(ldconfig -p | grep hdf5_serial)
  }

  mac {
   CONFIG += c++11
   QMAKE_LFLAGS += -lc++
   INCLUDEPATH += /usr/local/include
   QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing
  }

  contains (H5SERIAL, libhdf5_serial.so) {
    LIBS += -lhdf5_serial -lhdf5_serial_hl
    INCLUDEPATH += /usr/include/hdf5/serial
  }

  !contains (H5SERIAL, libhdf5_serial.so) {
    LIBS += -lhdf5 -lhdf5_hl_cpp
  }
}

TARGET   = $$PWD/tpcc
TEMPLATE = app

CONFIG(debug, debug|release) {
   TARGET = $$join(TARGET,,,d)
   DEFINES += "TPCC_DBG_"
}

INSTALLS += target

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

INCLUDEPATH += $$PWD/gui \
               $$PWD/gui/qtcolorpicker \
               $$PWD/gui/plots \
               $$PWD/gui/plots/qcustomplot \
               $$PWD/tpc

SOURCES += $$files($$PWD/gui/*.cpp) \
           $$files($$PWD/gui/qtcolorpicker/*.cpp) \
           $$files($$PWD/gui/plots/*.cpp) \
           $$files($$PWD/gui/plots/qcustomplot/*.cpp) \
           $$files($$PWD/tpc/*.cpp) \
           $$files($$PWD/tpc/*.tpp)

HEADERS  += $$files($$PWD/gui/*.h) \
            $$files($$PWD/gui/qtcolorpicker/*.h) \
            $$files($$PWD/gui/plots/*.h) \
            $$files($$PWD/gui/plots/qcustomplot/*.h) \
            $$files($$PWD/tpc/*.h)

FORMS   += $$files($$PWD/gui/*.ui) \
           $$files($$PWD/gui/plots/*.ui) \
           $$files($$PWD/tpc/*.ui)

RESOURCES += $$files($$PWD/resources/*.qrc)
