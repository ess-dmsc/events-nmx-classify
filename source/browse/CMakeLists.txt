cmake_minimum_required(VERSION 2.8)
project(nmx_browse CXX)

# ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES *.cpp *.h *.ui *.qrc)
file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS *.h ${PROJECT_BINARY_DIR}/*.h)
foreach (_headerFile ${${PROJECT_NAME}_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ${PROJECT_NAME}_INCLUDE_DIRS)

include_directories(
  ${${PROJECT_NAME}_INCLUDE_DIRS}
  ${nmx_common_INCLUDE_DIRS}
  ${ROOT_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
)

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
#  resources/tpccres.qrc
#  ColorWidgets/color_widgets.qrc
)

target_link_libraries(
  ${PROJECT_NAME}
  ${nmx_common_LIBRARIES}
  ${ROOT_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

qt5_use_modules(${PROJECT_NAME} Widgets PrintSupport)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
