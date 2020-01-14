if(QUBUS_BUILD_BENCHMARKS)

    add_executable(component_creation_benchmark component_creation.cpp)
    target_compile_features(component_creation_benchmark PRIVATE cxx_std_17)
    target_include_directories(component_creation_benchmark PRIVATE SYSTEM ${HPX_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/external/nonius/include)
    target_link_libraries(component_creation_benchmark PRIVATE hpx hpx_init ${CMAKE_THREAD_LIBS_INIT})

endif()