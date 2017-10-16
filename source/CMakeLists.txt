cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(events-nmx-classify CXX)
option(GUI "Build nmx_browse GUI" ON)

set(EXTRA_MODULES_DIR ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${EXTRA_MODULES_DIR})

#=============================================================================
# some general configuration
#=============================================================================

include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/OutputDirConfig.cmake)
include(${EXTRA_MODULES_DIR}/EnsureBuildType.cmake)
include(${EXTRA_MODULES_DIR}/PackageConfig.cmake)

#=============================================================================
# compiler and library configuration
#=============================================================================
include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)

add_definitions(-DBOOST_LOG_DYN_LINK)

add_subdirectory(common)

add_subdirectory(analyze)
add_subdirectory(cluster)
add_subdirectory(uncluster)
add_subdirectory(hists)
add_subdirectory(convert_apv)
add_subdirectory(convert_vmm)
add_subdirectory(merits)
add_subdirectory(dump)

if(${GUI})
  add_subdirectory(browse)
endif()
