cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(events-nmx-classify CXX)

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

add_subdirectory(source)
