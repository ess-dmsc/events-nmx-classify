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


add_definitions(-DQCUSTOMPLOT_USE_OPENGL)

include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/common
                    ${CMAKE_SOURCE_DIR}/common/h5cc
                    ${CMAKE_SOURCE_DIR}/common/nmx
                    ${CMAKE_SOURCE_DIR}/browse
                    ${CMAKE_SOURCE_DIR}/browse/ColorWidgets
                    ${CMAKE_SOURCE_DIR}/browse/plots
                    ${CMAKE_SOURCE_DIR}/browse/plots/qcustomplot
                    ${HDF5_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
                    ${OPENGL_INCLUDE_DIR}
)


# Make sure the compiler can find the pre-processed files from qt5_wrap_ui
include_directories("${CMAKE_BINARY_DIR}")

file(GLOB sources_H5CC common/h5cc/*.cpp common/h5cc/*.h common/h5cc/*.tpp)
file(GLOB sources_NMX common/nmx/*.cpp common/nmx/*.h)
file(GLOB sources_common_r common/*.cpp common/*.h)

set(
  sources_common

  ${sources_common_r}
  ${sources_H5CC}
  ${sources_NMX}
)


add_executable(nmx_analyze analyze/main.cpp ${sources_common})
target_link_libraries(
  nmx_analyze

  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

add_executable(nmx_hists hists/main.cpp ${sources_common})
target_link_libraries(
  nmx_hists

  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)


file(GLOB_RECURSE sources_browse browse/*.cpp browse/*.h browse/*.ui)

add_executable(
  nmx_browse

  ${sources_browse}
  ${sources_common}
  browse/resources/tpccres.qrc
  browse/ColorWidgets/color_widgets.qrc
)

target_link_libraries(
  nmx_browse

  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
  ${ROOT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

qt5_use_modules(nmx_browse Widgets PrintSupport)


if(UNIX)
  install(TARGETS nmx_analyze DESTINATION bin)
  install(TARGETS nmx_hists DESTINATION bin)
  install(TARGETS nmx_browse DESTINATION bin)
endif()

#HACK!! Until H5CC is made better
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type")
