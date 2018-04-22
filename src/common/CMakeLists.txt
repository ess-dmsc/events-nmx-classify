set(this_target ${PROJECT_NAME}_core)

# docopt
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)
list(APPEND docopt_LIBRARIES docopt)
list(APPEND docopt_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)

file(GLOB ${this_target}_SOURCES *.cpp nmx/*.cpp nmx/pipeline/*.cpp)
file(GLOB ${this_target}_HEADERS *.h nmx/*.h nmx/pipeline/*.cpp)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_library(
  ${this_target}
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PUBLIC ${${this_target}_INCLUDE_DIRS}
  PUBLIC ${Boost_INCLUDE_DIRS}
  PUBLIC ${ROOT_INCLUDE_DIRS}
  PUBLIC ${docopt_INCLUDE_DIRS}
)

target_link_libraries(
  ${this_target}
  PUBLIC ${Boost_LIBRARIES}
  PUBLIC ${COVERAGE_LIBRARIES}
  PUBLIC ${ROOT_LIBRARIES}
  PUBLIC ${docopt_LIBRARIES}
  PUBLIC h5cpp
  PRIVATE Threads::Threads
)

enable_coverage(${this_target})
