include(${EXTRA_MODULES_DIR}/CompilerConfig.cmake)
include(${EXTRA_MODULES_DIR}/DirsOf.cmake)
include(${EXTRA_MODULES_DIR}/InstallConfig.cmake)
include(${EXTRA_MODULES_DIR}/BuildInfo.cmake)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)
find_package(h5cpp 0.0.5 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(common)

add_subdirectory(analyze)
add_subdirectory(cluster)
#add_subdirectory(uncluster)
#add_subdirectory(hists)
#add_subdirectory(convert_apv)
#add_subdirectory(convert_vmm)
#add_subdirectory(merits)
#add_subdirectory(dump)
#
#option(GUI "Build nmx_browse GUI" ON)
#if(${GUI})
#  add_subdirectory(browse)
#endif()
