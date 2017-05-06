cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(nmx_util CXX)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

#===========================================================
# Internal compiler options
#===========================================================
# C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 11)
  set(CMAKE_CXX_STANDARD 11)
endif()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#=========================================================
# Function for deducing include dirs
# retname - name of parent variable to fill
# file_list - list of dirs, pass to fn as "${list_var}"
#=========================================================
function(dirs_of retname file_list)
  set(dlist "")
  foreach (_file ${file_list})
      get_filename_component(_dir ${_file} PATH)
      list (APPEND dlist ${_dir})
  endforeach()
  list(REMOVE_DUPLICATES dlist)
  set (${retname} ${dlist} PARENT_SCOPE)
endfunction(dirs_of)


add_subdirectory(common)

add_subdirectory(analyze)
add_subdirectory(cluster)
add_subdirectory(uncluster)
add_subdirectory(hists)
add_subdirectory(browse)
add_subdirectory(convert_apv)
add_subdirectory(convert_vmm)
