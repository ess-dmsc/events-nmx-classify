cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(nmx_browse CXX)

add_definitions(-DBOOST_LOG_DYN_LINK)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5Widgets REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB ${PROJECT_NAME}_RESOURCES resources/*.qrc ColorWidgets/*.qrc)
file(GLOB ${PROJECT_NAME}_SOURCES *.cpp *.ui ColorWidgets/*.cpp)
file(GLOB ${PROJECT_NAME}_HEADERS *.h ${PROJECT_BINARY_DIR}/*.h ColorWidgets/*.hpp)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

file(GLOB_RECURSE QPLOT_SOURCES qplot/src/qplot/*.cpp)
file(GLOB_RECURSE QPLOT_HEADERS qplot/src/qplot/*.h)
dirs_of(QPLOT_INCLUDE_DIRS "${QPLOT_HEADERS}")

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_RESOURCES}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
  ${QPLOT_SOURCES}
  ${QPLOT_HEADERS}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${${PROJECT_NAME}_INCLUDE_DIRS}
  PRIVATE ${QPLOT_INCLUDE_DIRS}
  PRIVATE ${nmx_common_INCLUDE_DIRS}
  PRIVATE ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${nmx_common_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

qt5_use_modules(${PROJECT_NAME} Widgets PrintSupport)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
