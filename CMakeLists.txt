cmake_minimum_required (VERSION 3.7.0)
enable_testing()

project(Qubus)

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

set(QUBUS_ROOT_DIR ${PROJECT_SOURCE_DIR})
set(QUBUS_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

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
include (CheckLTO)

CheckSIMD()
CheckFMA()
CheckLTO()

add_compile_options(${QUBUS_LTO_FLAGS})
link_libraries(${QUBUS_LTO_FLAGS})

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

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    option(QUBUS_ENABLE_UB_SANITIZER "Enable the undefined behaviour sanitizer" OFF)
else()
    option(QUBUS_ENABLE_UB_SANITIZER "Enable the undefined behaviourq sanitizer" OFF)
endif()

if(QUBUS_ENABLE_UB_SANITIZER)
    add_compile_options(-fsanitize=undefined)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
endif()

option(QUBUS_RUN_IWYU "Toggle if CMake should run include-what-you-use during compilation." OFF)

if (QUBUS_RUN_IWYU)
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE include-what-you-use)
endif()

if (NOT CMAKE_VERSION VERSION_LESS 3.6.0)
    option(QUBUS_RUN_CLANG_TIDY "Toggle if CMake (>= 3.6.0) should run clang-tidy during compilation." OFF)
endif()

if (QUBUS_RUN_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy;-p;${CMAKE_BINARY_DIR})
endif()

option(QUBUS_ANALYSE_COVERAGE "Enable code coverage analysis support." OFF)

if (QUBUS_ANALYSE_COVERAGE)
    if(CMAKE_COMPILER_IS_GNUCXX)
        include(CodeCoverage)
        add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
        setup_target_for_coverage(coverage ctest coverage)
        add_compile_options(--coverage)
        link_libraries(--coverage)
    else()
        message(WARNING "Code coverage analysis is only supported with GCC.")
    endif()
endif()

add_subdirectory(util)
add_subdirectory(qubus)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/qubus/qubus-config-version.cmake"
  VERSION ${QUBUS_VERSION}
  COMPATIBILITY AnyNewerVersion
)

set(ConfigPackageLocation lib/cmake/Qubus)

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
