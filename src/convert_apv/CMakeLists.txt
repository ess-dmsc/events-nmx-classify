set(this_target nmx_convert_apv)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

file(GLOB ${this_target}_SOURCES *.cpp)
file(GLOB ${this_target}_HEADERS *.h)
dirs_of(${this_target}_INCLUDE_DIRS "${${this_target}_HEADERS}")

add_executable(
  ${this_target}
  ${${this_target}_SOURCES}
  ${${this_target}_HEADERS}
)

target_include_directories(
  ${this_target}
  PRIVATE ${${this_target}_INCLUDE_DIRS}
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${Boost_LIBRARIES}
)

#
#
#
#file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)
#file(GLOB ${PROJECT_NAME}_HEADERS *.h)
#dirs_of(${PROJECT_NAME}_INCLUDE_DIRS "${${PROJECT_NAME}_HEADERS}")
#
#add_executable(
#  ${PROJECT_NAME}
#  ${${PROJECT_NAME}_SOURCES}
#  ${${PROJECT_NAME}_HEADERS}
#)
#
#target_include_directories(
#  ${PROJECT_NAME}
#  PRIVATE ${nmx_common_INCLUDE_DIRS}
#  PRIVATE ${ROOT_INCLUDE_DIRS}
#)
#
#target_link_libraries(
#  ${PROJECT_NAME}
#  ${nmx_common_LIBRARIES}
#  ${ROOT_LIBRARIES}
#)
#
#if(UNIX)
#  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#endif()
