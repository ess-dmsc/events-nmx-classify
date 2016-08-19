CONFIG += debug_and_release

BADDIRS = "debug release ui"

UI_DIR = $$PWD/ui

QMAKE_CLEAN += -r $$BADDIRS \
               $$files(tpcClassify.pro.user*) \
               install

CONFIG -= warn_off warn_on
CONFIG += static

QMAKE_CFLAGS_RELEASE += -w
QMAKE_CXXFLAGS_DEBUG += -O0 -Wextra

QMAKE_CXXFLAGS += -DBOOST_LOG_DYN_LINK

unix {
  !android {
     QMAKE_CC = g++
  }

  LIBS += -lm -ldl -lz \
          -lboost_system -lboost_date_time -lboost_thread -lboost_log \
          -lboost_filesystem -lboost_log_setup -lboost_timer -lboost_regex -lhdf5_cpp

  target.path = /usr/local/bin/
      
  LIBPATH += /usr/local/lib

  !mac {
    H5SERIAL = $$system(ldconfig -p | grep hdf5_serial)
    CONFIG -= c++11
    QMAKE_CXXFLAGS += -std=c++11
    INCLUDEPATH += /usr/include/hdf5/serial
  }

  contains (H5SERIAL, libhdf5_serial.so) {
    LIBS += -lhdf5_serial -lhdf5_serial_hl
  }

  !contains (H5SERIAL, libhdf5_serial.so) {
    LIBS += -lhdf5 -lhdf5_hl_cpp
  }
}

mac {
     CONFIG += c++11
     QMAKE_LFLAGS += -lc++
     LIBPATH += /usr/local/lib
     INCLUDEPATH += /usr/local/include
     QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing
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
               $$PWD/gui/plots \
               $$PWD/gui/plots/qcustomplot \
               $$PWD/tpc

SOURCES += $$files($$PWD/gui/*.cpp) \
           $$files($$PWD/gui/plots/*.cpp) \
           $$files($$PWD/gui/plots/qcustomplot/*.cpp) \
           $$files($$PWD/tpc/*.cpp)

HEADERS  += $$files($$PWD/gui/*.h) \
            $$files($$PWD/gui/plots/*.h) \
            $$files($$PWD/gui/plots/qcustomplot/*.h) \
            $$files($$PWD/tpc/*.h)

FORMS   += $$files($$PWD/gui/*.ui) \
           $$files($$PWD/gui/plots/*.ui) \
           $$files($$PWD/tpc/*.ui)

RESOURCES += $$files($$PWD/resources/*.qrc)
