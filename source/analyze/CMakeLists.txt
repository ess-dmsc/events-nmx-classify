cmake_minimum_required(VERSION 3.1)
project(nmx_analyze CXX)

# Set C++11 flag.
set (CMAKE_CXX_STANDARD 11)

set(HDF5_USE_SHARED_LIBRARIES ON)

#Locate packages
find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(Boost COMPONENTS system filesystem thread timer program_options REQUIRED)

# Boost needs this
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK")

link_directories( ${BOOST_LIBRARY_DIRS} )



set (main_INCLUDE_DIRS "")

file(GLOB_RECURSE Common_SOURCES "../common/*.cpp")
file(GLOB_RECURSE Common_HEADERS "../common/*.h")
foreach (_headerFile ${Common_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND main_INCLUDE_DIRS ${_dir})
endforeach()

file(GLOB_RECURSE main_SOURCES "*.cpp")
file(GLOB_RECURSE main_HEADERS "*.h")

foreach (_headerFile ${main_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND main_INCLUDE_DIRS ${_dir})
endforeach()

list(REMOVE_DUPLICATES main_INCLUDE_DIRS)

include_directories(
  ${main_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

add_executable(
  nmx_analyze

  ${main_SOURCES}
  ${Common_SOURCES}
)

target_link_libraries(
  nmx_analyze

  ${HDF5_LIBRARIES}
  ${Boost_LIBRARIES}
)


if(UNIX)
  install(TARGETS nmx_analyze DESTINATION bin)
endif()
