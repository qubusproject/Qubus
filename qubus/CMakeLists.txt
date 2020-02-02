include(Qubus_SetOutputName)

find_package(HPX 1.1.0 REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(examples)

generate_export_header(qubus BASE_NAME QUBUS)

add_executable(qubus_test main.cpp)
target_link_libraries(qubus_test PUBLIC qubus qubus_qtl)
target_compile_options(qubus_test PRIVATE -mavx2 -mfma)
target_compile_definitions(qubus_test PRIVATE -DEIGEN_DONT_VECTORIZE -DEIGEN_DONT_PARALLELIZE)
hpx_setup_target(qubus_test COMPONENT_DEPENDENCIES iostreams)

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

# Build documentation
find_package(Doxygen)
find_package(Sphinx)

if(DOXYGEN_FOUND)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile @ONLY)

    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/html)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/latex)
      
    add_custom_target(qubus_api_doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
        COMMAND 
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
        COMMENT "Generating API documentation with Doxygen." VERBATIM)

    list(APPEND QUBUS_DOC_TARGETS qubus_api_doc)

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

        add_custom_target(qubus_user_guide
                ${SPHINX_EXECUTABLE}
                -q -b html
                -c "${BINARY_BUILD_DIR}"
                -d "${SPHINX_CACHE_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/doc/user-guide"
                "${SPHINX_HTML_DIR}"
                COMMENT "Building user guide with Sphinx.")

        list(APPEND QUBUS_DOC_TARGETS qubus_user_guide)
        
        add_custom_target(qubus_user_guide_latex
                ${SPHINX_EXECUTABLE}
                -q -b latex
                -c "${BINARY_BUILD_DIR}"
                -d "${SPHINX_CACHE_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/doc/user-guide"
                "${SPHINX_LATEX_DIR}"
                COMMENT "Building user guide (latex version) with Sphinx.")

    endif(SPHINX_FOUND)
endif(DOXYGEN_FOUND)

add_custom_target(qubus_doc DEPENDS ${QUBUS_DOC_TARGETS} COMMENT "Generating the Qubus documentation.")