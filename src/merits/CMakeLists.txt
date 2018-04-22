set(this_target nmx_merits)

include(${EXTRA_MODULES_DIR}/BoostLibraryConfig.cmake)

add_executable(
  ${this_target}
  main.cpp
)

target_link_libraries(
  ${this_target}
  PRIVATE ${PROJECT_NAME}_core
  PRIVATE ${Boost_LIBRARIES}
)
