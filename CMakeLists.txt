cmake_minimum_required (VERSION 2.8.12)
enable_testing()

project(QBB)

set(QUBUS_VERSION 0.1.0)

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

set(QUBUS_CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/lib/cmake/qubus)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

message(STATUS "The cmake module path is: ${CMAKE_MODULE_PATH}")

set(QBB_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(QBB_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

include (CheckSIMD)
include (CheckFMA)

CheckSIMD()
CheckFMA()

if (NOT CMAKE_VERSION VERSION_LESS 3.3.0)
  option(QBB_RUN_IWYU "Toggle if CMake (>= 3.3.0) should run include-what-you-use during compilation." OFF)
endif()

if (QBB_RUN_IWYU)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use)
endif()

add_subdirectory(util)
add_subdirectory(qubus)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qubus/qubus-config-version.cmake"
  VERSION ${QUBUS_VERSION}
  COMPATIBILITY AnyNewerVersion
)

set(ConfigPackageLocation lib/cmake/qubus)

install(EXPORT qubus-targets
  FILE
    qubus-targets.cmake
  NAMESPACE
    qubus::
  DESTINATION
    ${ConfigPackageLocation}
)

configure_file(cmake/qubus-macros.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/qubus-macros.cmake
               ESCAPE_QUOTES @ONLY)

install(
  FILES
    cmake/qubus-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/qubus-macros.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/qubus/qubus-config-version.cmake"
    cmake/FindISL.cmake
  DESTINATION
    ${ConfigPackageLocation})
    
add_custom_target(doc DEPENDS qubus_doc COMMENT "Generating the documentation.")