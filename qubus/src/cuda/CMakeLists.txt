find_package(CUDA 7.0)

if (CUDA_FOUND)

    set(qubus_cuda_source_files core.cpp support.cpp)

    add_library(qubus_cuda STATIC ${qubus_cuda_source_files})

    target_include_directories(qubus_cuda PUBLIC ${CUDA_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>
                                                  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
    target_compile_options(qubus_cuda PUBLIC -std=c++14)
    target_link_libraries(qubus_cuda qubus_util ${CUDA_CUDA_LIBRARY})
    hpx_setup_target(qubus_cuda)
    hpx_set_lib_name(qubus_cuda qubus_cuda)
    qubus_set_output_name(qubus_cuda qubus_cuda)

    set_target_properties(qubus_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)

    install(TARGETS qubus_cuda EXPORT qubus-targets
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)

endif()