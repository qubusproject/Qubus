macro(CheckLTO)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        option(QUBUS_ENABLE_LTO "enable LTO (if supported)" On)
    else()
        option(QUBUS_ENABLE_LTO "enable LTO (if supported)" Off)
    endif()

    if (QUBUS_ENABLE_LTO)
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(QUBUS_LTO_FLAGS -flto)
        endif()
    endif()

endmacro()