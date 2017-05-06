if(QUBUS_BUILD_BENCHMARKS)

    add_executable(address_resolution_benchmark address_resolution.cpp)
    target_include_directories(address_resolution_benchmark PUBLIC ${CMAKE_SOURCE_DIR}/external/nonius/include)
    target_compile_options(address_resolution_benchmark PUBLIC -std=c++14)
    target_link_libraries(address_resolution_benchmark PUBLIC qubus_qtl qubus ${CMAKE_THREAD_LIBS_INIT})

endif()