cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(nmx_analyze CXX)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp")

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${nmx_common_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${nmx_common_LIBRARIES}
)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
