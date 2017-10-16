include_directories(${PROJECT_SOURCE_DIR}/src/include)
set(WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/test)

include(${EXTRA_MODULES_DIR}/TestCoverage.cmake)

set(Common_SRC main.cpp
    ../src/common/nmx/pipeline/Eventlet.cpp)
set(Common_INC
  EventletTest.hpp
  ${nmx_common_HEADERS})
create_test_executable(Common h5cc)

finalize_tests()
