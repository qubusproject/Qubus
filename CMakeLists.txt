cmake_minimum_required (VERSION 2.8.12)
enable_testing()

project(QBB)

if(POLICY CMP0053)
  cmake_policy(SET CMP0053 NEW)
endif()

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
 
message(STATUS "The install prefix is: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

message(STATUS "The cmake module path is: ${CMAKE_MODULE_PATH}")

set(QBB_ROOT_DIR ${PROJECT_SOURCE_DIR})

include (CheckSIMD)
include (CheckFMA)

CheckSIMD()
CheckFMA()

add_subdirectory(util)
add_subdirectory(kubus)
