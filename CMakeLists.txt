cmake_minimum_required(VERSION 2.6)
project(lsx)

set(LS_VERSION_MAJOR 0)
set(LS_VERSION_MINOR 8)
set(LS_VERSION_PATCH 0)

OPTION(BUILD_GIT "Build git support?" ON)

set(BUILD_GIT_FLAG 0)

if (BUILD_GIT)
    set(BUILD_GIT_FLAG 1)
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/src/ls_config.h.in"
  "${PROJECT_BINARY_DIR}/src/ls_config.h"
)

include_directories("${PROJECT_BINARY_DIR}/src")

add_subdirectory(src)
add_subdirectory(ext)