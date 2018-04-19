set(this_target ${PROJECT_NAME}_core)

# ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

# docopt
add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)
list(APPEND docopt_LIBRARIES docopt)
list(APPEND docopt_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/external/docopt.cpp)

file(GLOB ${this_target}_SOURCES *.cpp nmx/*.cpp nmx/pipeline/*.cpp)
file(GLOB ${this_target}_HEADERS *.h nmx/*.h nmx/pipeline/*.cpp)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

#file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)
#file(GLOB_RECURSE nmx_SOURCES nmx/*.cpp)
#set(${PROJECT_NAME}_SOURCES
#  ${${PROJECT_NAME}_SOURCES} ${nmx_SOURCES})
#
#file(GLOB ${PROJECT_NAME}_HEADERS *.h *.tpp)
#file(GLOB_RECURSE nmx_HEADERS nmx/*.h)
#set(${PROJECT_NAME}_HEADERS
#  ${${PROJECT_NAME}_HEADERS} ${nmx_HEADERS})
#
#dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")


add_library(
  ${this_target} STATIC
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


#target_include_directories(
#  ${PROJECT_NAME}
#  PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS}
#  PUBLIC ${Boost_INCLUDE_DIRS}
#  PUBLIC ${ROOT_INCLUDE_DIRS}
#  PUBLIC ${h5cc_INCLUDE_DIRS}
#  PUBLIC ${json_INCLUDE_DIRS}
#  PUBLIC ${docopt_INCLUDE_DIRS}
#)
#
#target_link_libraries(
#  ${PROJECT_NAME}
#  ${Boost_LIBRARIES}
#  ${ROOT_LIBRARIES}
#  ${h5cc_LIBRARIES}
#  ${docopt_LIBRARIES}
#  ${CMAKE_THREAD_LIBS_INIT}
#)
#
#set(${PROJECT_NAME}_LIBRARIES
#    ${PROJECT_NAME}
#    PARENT_SCOPE)
#
#set(${PROJECT_NAME}_HEADERS "${${PROJECT_NAME}_HEADERS}" CACHE INTERNAL "")
#set(${PROJECT_NAME}_SOURCES "${${PROJECT_NAME}_SOURCES}" CACHE INTERNAL "")
#set(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_INCLUDE_DIRS}" CACHE INTERNAL "")
