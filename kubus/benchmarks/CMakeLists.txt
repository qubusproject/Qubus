option(KUBUS_BUILD_BENCHMARKS "build the kubus benchmark suite" OFF)

if(KUBUS_BUILD_BENCHMARKS)
    add_executable(mm_bench mm_bench.cpp)
    target_link_libraries(mm_bench PUBLIC kubus hpx_init)
    target_compile_options(mm_bench PRIVATE -mavx2 -mfma)
    target_link_libraries(mm_bench PRIVATE)
    target_compile_definitions(mm_bench PRIVATE -DEIGEN_DONT_PARALLELIZE)
endif()