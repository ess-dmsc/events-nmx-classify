set(this_target nmx_convert_vmm)

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
