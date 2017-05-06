cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(nmx_convert_apv CXX)

#root
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)
file(GLOB ${PROJECT_NAME}_HEADERS *.h)
dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
  ${${PROJECT_NAME}_HEADERS}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${nmx_common_INCLUDE_DIRS}
  PRIVATE ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${nmx_common_LIBRARIES}
  ${ROOT_LIBRARIES}
)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
