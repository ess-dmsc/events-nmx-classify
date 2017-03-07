cmake_minimum_required(VERSION 3.1)
project(nmx_browse CXX)

# Set C++11 flag.
set (CMAKE_CXX_STANDARD 11)

#root
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#For Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HDF5_USE_SHARED_LIBRARIES ON)

#Locate packages
find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost COMPONENTS system filesystem thread timer program_options REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(ROOT REQUIRED COMPONENTS RIO Net)
find_package(OpenGL REQUIRED)

#ROOT needs this
include(${ROOT_USE_FILE})

# Boost needs this
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")

link_directories( ${BOOST_LIBRARY_DIRS} )

#add_definitions(-DQCUSTOMPLOT_USE_OPENGL)





set (main_INCLUDE_DIRS "")

file(GLOB_RECURSE Common_SOURCES "../common/*.cpp")
file(GLOB_RECURSE Common_TEMPLATES "../common/*.tpp")
file(GLOB_RECURSE Common_HEADERS "../common/*.h")
foreach (_headerFile ${Common_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND main_INCLUDE_DIRS ${_dir})
endforeach()

file(GLOB_RECURSE main_SOURCES *.cpp *.h *.ui)
file(GLOB_RECURSE main_HEADERS *.h ${PROJECT_BINARY_DIR}/*.h)

foreach (_headerFile ${main_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND main_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES main_INCLUDE_DIRS)

include_directories(
  ${PROJECT_BINARY_DIR}
  ${main_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${ROOT_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)

add_executable(
  nmx_browse

  ${main_SOURCES}
  ${Common_SOURCES}
  ${Common_TEMPLATES}
  resources/tpccres.qrc
  ColorWidgets/color_widgets.qrc
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
  install(TARGETS nmx_browse DESTINATION bin)
endif()
