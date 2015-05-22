cmake_minimum_required (VERSION 2.8.12)
enable_testing()

project(QBB)

set(KUBUS_VERSION 0.1.0)

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

set(KUBUS_CMAKE_MODULE_PATH ${CMAKE_INSTALL_PREFIX}/lib/cmake/kubus)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

message(STATUS "The cmake module path is: ${CMAKE_MODULE_PATH}")

set(QBB_ROOT_DIR ${PROJECT_SOURCE_DIR})

include (CheckSIMD)
include (CheckFMA)

CheckSIMD()
CheckFMA()

add_subdirectory(util)
add_subdirectory(kubus)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/kubus/kubus-config-version.cmake"
  VERSION ${KUBUS_VERSION}
  COMPATIBILITY AnyNewerVersion
)

set(ConfigPackageLocation lib/cmake/kubus)

install(EXPORT kubus-targets
  FILE
    kubus-targets.cmake
  NAMESPACE
    kubus::
  DESTINATION
    ${ConfigPackageLocation}
)

configure_file(cmake/kubus-macros.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/kubus-macros.cmake
               ESCAPE_QUOTES @ONLY)

install(
  FILES
    cmake/kubus-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/kubus-macros.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/kubus/kubus-config-version.cmake"
    cmake/FindISL.cmake
  DESTINATION
    ${ConfigPackageLocation})
    
add_custom_target(doc DEPENDS kubus_doc COMMENT "Generating the documentation.")