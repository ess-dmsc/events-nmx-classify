include_directories(${PROJECT_SOURCE_DIR}/src/include)
set(WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)

include(${EXTRA_MODULES_DIR}/TestCoverage.cmake)

set(Common_SRC main.cpp
  EventletTest.cpp
  MicroclusterTest.cpp
  ../src/common/nmx/pipeline/Eventlet.cpp
  ../src/common/nmx/pipeline/Eventlet.h
  ../src/common/nmx/pipeline/Microcluster.cpp
  ../src/common/nmx/pipeline/Microcluster.h)
set(Common_INC
  ${nmx_common_HEADERS})
set(Common_DIRS
  ${nmx_common_INCLUDE_DIRS})


message(STATUS "test src " ${Common_SRC})
message(STATUS "test hpp " ${Common_INC})
message(STATUS "test dir " ${Common_DIRS})

create_test_executable(Common nmx_common)

finalize_tests()
