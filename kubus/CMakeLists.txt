cmake_minimum_required(VERSION 2.8.12)
enable_testing()

project(kubus)

if(POLICY CMP0053)
  cmake_policy(SET CMP0053 NEW)
endif()

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

add_library(asan INTERFACE)
target_compile_options(asan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=address -fno-omit-frame-pointer>)
target_link_libraries(asan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=address -fno-omit-frame-pointer>)

add_library(tsan INTERFACE)
target_compile_options(tsan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=thread -fPIC>)
target_link_libraries(tsan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=thread -fPIC>)

add_library(usan INTERFACE)
target_compile_options(usan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=undefined>)
target_link_libraries(usan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=undefined>)

add_library(wall INTERFACE)
target_compile_options(wall INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra>)

install(TARGETS asan tsan usan wall EXPORT kubus-targets)

include(GenerateExportHeader)
include(FeatureSummary)

add_subdirectory(src)
add_subdirectory(tests)

generate_export_header(kubus BASE_NAME QBB_KUBUS)

add_executable(kubus_test main.cpp)
target_link_libraries(kubus_test PUBLIC kubus hpx_init)
target_compile_options(kubus_test PRIVATE)
target_link_libraries(kubus_test PRIVATE)
target_compile_definitions(kubus_test PRIVATE -DEIGEN_DONT_VECTORIZE)

feature_summary(WHAT ALL)

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

# Build documentation
find_package(Sphinx)

if(SPHINX_FOUND)
 
  if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME default)
  endif()
  if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR)
  endif()

  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/html/user_guide")
  set(SPHINX_LATEX_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/latex/user_guide")
  configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/doc/user-guide/conf.py.in"
        "${BINARY_BUILD_DIR}/conf.py"
        @ONLY)

  add_custom_target(user_guide
        ${SPHINX_EXECUTABLE}
        -q -b html
        -c "${BINARY_BUILD_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/doc/user-guide"
        "${SPHINX_HTML_DIR}"
        COMMENT "Building user guide with Sphinx.")

  list(APPEND KUBUS_DOC_TARGETS user_guide)
  
  add_custom_target(user_guide_latex
        ${SPHINX_EXECUTABLE}
        -q -b latex
        -c "${BINARY_BUILD_DIR}"
        -d "${SPHINX_CACHE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/doc/user-guide"
        "${SPHINX_LATEX_DIR}"
        COMMENT "Building user guide (latex version) with Sphinx.")

endif(SPHINX_FOUND)

add_custom_target(doc DEPENDS ${KUBUS_DOC_TARGETS} COMMENT "Generating the documentation.")