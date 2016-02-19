option(QUBUS_BUILD_BENCHMARKS "build the Qubus benchmark suite" OFF)

if(QUBUS_BUILD_BENCHMARKS)
    find_package(Threads REQUIRED)

    add_executable(mm_bench mm_bench.cpp)
    target_link_libraries(mm_bench PUBLIC qubus hpx_init)
    target_compile_options(mm_bench PRIVATE -mavx2 -mfma)
    target_link_libraries(mm_bench PRIVATE)
    target_compile_definitions(mm_bench PRIVATE -DEIGEN_DONT_PARALLELIZE)

    add_executable(spmv_bench spmv.cpp)
    target_link_libraries(spmv_bench PUBLIC qubus hpx_init)
    target_compile_options(spmv_bench PRIVATE -mavx2 -mfma)
    target_link_libraries(spmv_bench PRIVATE)
    target_compile_definitions(spmv_bench PRIVATE -DEIGEN_DONT_PARALLELIZE)

    add_executable(expression_equality_benchmark expression_equality.cpp)
    target_include_directories(expression_equality_benchmark PUBLIC ${CMAKE_SOURCE_DIR}/external/nonius/include)
    target_compile_features(expression_equality_benchmark PUBLIC cxx_generic_lambdas)
    target_link_libraries(expression_equality_benchmark PUBLIC qubus_ir ${CMAKE_THREAD_LIBS_INIT})
endif()