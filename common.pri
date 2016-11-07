BADDIRS = "debug release ui"

UI_DIR = $$PWD/ui

QMAKE_CLEAN += -r $$BADDIRS \
               $$files(tpcc*.log) \
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

CONFIG(debug, debug|release) {
   TARGET = $$join(TARGET,,,d)
   DEFINES += "TPCC_DBG_"
}

INCLUDEPATH += $$PWD/tpc \
               $$PWD/histogram \
               $$PWD/logger \
               $$PWD/h5cc

SOURCES += $$files($$PWD/tpc/*.cpp) \
           $$files($$PWD/logger/*.cpp) \
           $$files($$PWD/h5cc/*.tpp) \
           $$files($$PWD/h5cc/*.cpp)

HEADERS  += $$files($$PWD/tpc/*.h) \
            $$files($$PWD/histogram/*.h) \
            $$files($$PWD/logger/*.h) \
            $$files($$PWD/h5cc/*.h)

