cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(nmx_common CXX)

# Boost
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS
  system filesystem thread timer date_time log log_setup
  REQUIRED)

# h5cc
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/external/h5cc/source/h5cc)

# docopt
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)
list(APPEND docopt_LIBRARIES docopt)
list(APPEND docopt_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)

# json++
list (APPEND json_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/external/json/src)

message(STATUS "h5ccdir " ${h5cc_INCLUDE_DIRS})

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)
file(GLOB_RECURSE nmx_SOURCES nmx/*.cpp)
set(${PROJECT_NAME}_SOURCES
  ${${PROJECT_NAME}_SOURCES} ${nmx_SOURCES})

file(GLOB ${PROJECT_NAME}_HEADERS *.h *.tpp)
file(GLOB_RECURSE nmx_HEADERS nmx/*.h)
set(${PROJECT_NAME}_HEADERS
  ${${PROJECT_NAME}_HEADERS} ${nmx_HEADERS})

dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

add_library(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
  PUBLIC ${Boost_INCLUDE_DIRS}
  PUBLIC ${h5cc_INCLUDE_DIRS}
  PUBLIC ${json_INCLUDE_DIRS}
  PUBLIC ${docopt_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${h5cc_LIBRARIES}
  ${docopt_LIBRARIES}
)

message(STATUS "H5CC LIBS ===== " ${h5cc_INCLUDE_DIRS})

set(${PROJECT_NAME}_LIBRARIES
    ${PROJECT_NAME}
    PARENT_SCOPE)
