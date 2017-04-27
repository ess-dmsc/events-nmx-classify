cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(nmx_util CXX)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Common flags
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Import h5cc
include(DownloadProject)
download_project(PROJ                h5cc
                 GIT_REPOSITORY      https://github.com/ess-dmsc/h5cc.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)
add_subdirectory(${h5cc_SOURCE_DIR}/source/h5cc ${h5cc_BINARY_DIR})

add_subdirectory(common)

add_subdirectory(analyze)
add_subdirectory(cluster)
add_subdirectory(uncluster)
add_subdirectory(hists)
add_subdirectory(browse)
add_subdirectory(convert_apv)
add_subdirectory(convert_vmm)
