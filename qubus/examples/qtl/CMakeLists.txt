option(QUBUS_BUILD_EXAMPLES "Build the Qubus exmaples" OFF)

if (QUBUS_BUILD_EXAMPLES)
    add_executable(vector_addition vector_addition.cpp)
    target_link_libraries(vector_addition PRIVATE qubus_qtl hpx_init)

    install(TARGETS vector_addition EXPORT qubus-targets
            RUNTIME DESTINATION share/qubus/examples)
endif()