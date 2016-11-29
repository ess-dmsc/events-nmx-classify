cmake_minimum_required(VERSION 2.8.11)
project(nmx_util CXX)

#root
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#Locate packages
find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost COMPONENTS system filesystem thread log timer REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(ROOT REQUIRED COMPONENTS RIO Net)
find_package(OpenGL REQUIRED)

#ROOT needs this
include(${ROOT_USE_FILE})


# Set C++11 flag.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Boost needs this
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type")

add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

include_directories(${CMAKE_SOURCE_DIR}
                    common
                    common/h5cc
                    common/nmx
                    browse
                    browse/plots
                    browse/plots/qcustomplot
                    ${HDF5_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
)


# Make sure the compiler can find the pre-processed files from qt5_wrap_ui
include_directories("${CMAKE_BINARY_DIR}")

file(GLOB common_SRC
  "common/*.cpp"
  "common/*.h"

  "common/h5cc/*.cpp"
  "common/h5cc/*.h"
  "common/h5cc/*.tpp"

  "common/nmx/*.cpp"
  "common/nmx/*.h"
)

file(GLOB browse_SRC
     "browse/*.cpp"
     "browse/*.h"

     "browse/plots/*.cpp"
     "browse/plots/*.h"

     "browse/plots/qcustomplot/*.cpp"
     "browse/plots/qcustomplot/*.h"
)

add_executable(nmx_analyze
               analyze/main.cpp
               ${common_SRC}
)

add_executable(nmx_hists
               hists/main.cpp
               ${common_SRC}
)

add_executable(nmx_browse
               ${browse_SRC}
               ${common_SRC}
               browse/resources/tpccres.qrc
#              ${GENERATED_SOURCES}
)


target_link_libraries(
  nmx_analyze
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

target_link_libraries(
  nmx_hists
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

target_link_libraries(
  nmx_browse
  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
  ${ROOT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

qt5_use_modules(nmx_browse Widgets PrintSupport)
