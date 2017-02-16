cmake_minimum_required (VERSION 3.5.0)
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

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qubus is an autoparallelizing framework for tensor arithmetic
                                       supporting distributed and heterogeneous execution environments")
SET(CPACK_PACKAGE_VENDOR "Christopher Hinz")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Readme.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_SOURCE_PACKAGE_FILE_NAME "Qubus-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-src")
set(CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}/;/.git/;~$;${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)
include(GenerateExportHeader)
include(FeatureSummary)

include (CheckSIMD)
include (CheckFMA)

CheckSIMD()
CheckFMA()

if (CXX_COMPILER_ID STREQUAL "GNU" OR CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    option(QUBUS_ENABLE_ADDRESS_SANITIZER "Enable the address sanitizer" OFF)
else()
    option(QUBUS_ENABLE_ADDRESS_SANITIZER "Enable the address sanitizer" OFF)
endif()

if(QUBUS_ENABLE_ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

option(QBB_RUN_IWYU "Toggle if CMake should run include-what-you-use during compilation." OFF)

if (QBB_RUN_IWYU)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use)
endif()

if (NOT CMAKE_VERSION VERSION_LESS 3.6.0)
    option(QBB_RUN_CLANG_TIDY "Toggle if CMake (>= 3.6.0) should run clang-tidy during compilation." OFF)
endif()

if (QBB_RUN_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy;-p;${CMAKE_BINARY_DIR})
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

feature_summary(WHAT ALL)
