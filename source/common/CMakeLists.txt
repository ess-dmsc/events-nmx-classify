cmake_minimum_required(VERSION 2.8)
project(nmx_common CXX)

# Boost
find_package(Boost COMPONENTS system filesystem timer program_options REQUIRED)
link_directories( ${BOOST_LIBRARY_DIRS} )
#add_definitions(-DBOOST_LOG_DYN_LINK)

file(GLOB_RECURSE common_SOURCES "*.cpp")
file(GLOB_RECURSE common_HEADERS "*.h")
foreach (_headerFile ${common_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND common_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES common_INCLUDE_DIRS)

include_directories(
  ${common_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${h5cc_INCLUDE_DIRS}
)

add_library(
  ${PROJECT_NAME}
  ${common_SOURCES}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
#  ${h5cc_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${common_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${h5cc_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    -Wl,--whole-archive ${PROJECT_NAME} -Wl,--no-whole-archive
    PARENT_SCOPE)
