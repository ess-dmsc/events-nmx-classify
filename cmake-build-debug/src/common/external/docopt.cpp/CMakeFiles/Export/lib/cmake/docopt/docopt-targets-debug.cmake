#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "docopt" for configuration "Debug"
set_property(TARGET docopt APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(docopt PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libdocopt.so"
  IMPORTED_SONAME_DEBUG "libdocopt.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS docopt )
list(APPEND _IMPORT_CHECK_FILES_FOR_docopt "${_IMPORT_PREFIX}/lib/libdocopt.so" )

# Import target "docopt_s" for configuration "Debug"
set_property(TARGET docopt_s APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(docopt_s PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libdocopt.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS docopt_s )
list(APPEND _IMPORT_CHECK_FILES_FOR_docopt_s "${_IMPORT_PREFIX}/lib/libdocopt.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
