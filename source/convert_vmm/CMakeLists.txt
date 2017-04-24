cmake_minimum_required(VERSION 2.8)
project(nmx_convert_vmm CXX)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "*.cpp")
file(GLOB_RECURSE ${PROJECT_NAME}_HEADERS "*.h")
foreach (_headerFile ${${PROJECT_NAME}_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND ${PROJECT_NAME}_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ${PROJECT_NAME}_INCLUDE_DIRS)

include_directories(
  ${${PROJECT_NAME}_INCLUDE_DIRS}
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
