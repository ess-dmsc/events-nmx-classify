cmake_minimum_required(VERSION 2.8)
project(nmx_util CXX)

# Common flags
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_subdirectory(common)

add_subdirectory(analyze)
add_subdirectory(cluster)
add_subdirectory(uncluster)
add_subdirectory(hists)
add_subdirectory(browse)
add_subdirectory(convert_apv)
add_subdirectory(convert_vmm)
