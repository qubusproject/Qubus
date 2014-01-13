cmake_minimum_required (VERSION 2.8.12)

project(QBB)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
 
message(STATUS "The install prefix is: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

message(STATUS "The cmake module path is: ${CMAKE_MODULE_PATH}")

set(QBB_ROOT_DIR ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(tools)