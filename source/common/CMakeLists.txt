cmake_minimum_required(VERSION 2.8)
project(nmx_common CXX)

# Set C++11 flag.
set (CMAKE_CXX_STANDARD 11)

#Locate packages
set(HDF5_USE_SHARED_LIBRARIES ON)
find_package(HDF5 COMPONENTS CXX REQUIRED)

# Boost
find_package(Boost COMPONENTS system filesystem timer program_options REQUIRED)
#add_definitions(-DBOOST_LOG_DYN_LINK)
link_directories( ${BOOST_LIBRARY_DIRS} )

file(GLOB_RECURSE common_SOURCES "*.cpp")
file(GLOB_RECURSE common_TEMPLATES "*.tpp")
file(GLOB_RECURSE common_HEADERS "*.h")
foreach (_headerFile ${common_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND common_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES common_INCLUDE_DIRS)

include_directories(
  ${common_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_library(
  ${PROJECT_NAME}

  ${common_SOURCES}
  ${common_TEMPLATES}
)

target_link_libraries(
  ${PROJECT_NAME}

  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)

# Expose public includes
set(${PROJECT_NAME}_INCLUDE_DIRS
    ${common_INCLUDE_DIRS}
    ${HDF5_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(${PROJECT_NAME}_LIBRARIES
    -Wl,--whole-archive ${PROJECT_NAME} -Wl,--no-whole-archive
    PARENT_SCOPE)
