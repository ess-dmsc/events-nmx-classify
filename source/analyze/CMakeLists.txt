cmake_minimum_required(VERSION 2.8)
project(nmx_analyze CXX)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp")

include_directories(
  ${nmx_common_INCLUDE_DIRS}
)

add_executable(
  ${PROJECT_NAME}
  ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(
  ${PROJECT_NAME}
  ${nmx_common_LIBRARIES}
)

if(UNIX)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
