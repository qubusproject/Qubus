find_package(CUDA 7.0)

if (CUDA_FOUND)

    add_library(qubus_cuda_backend SHARED cuda_backend.cpp)

    target_include_directories(qubus_cuda_backend PUBLIC ${Boost_LIBRARY_DIRS}
                               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
    target_compile_options(qubus_cuda_backend PUBLIC -std=c++14)
    target_link_libraries(qubus_cuda_backend ${Boost_LIBRARIES} qubus_ir qubus_util qubus_jit)
    hpx_setup_target(qubus_cuda_backend COMPONENT_DEPENDENCIES iostreams)
    hpx_set_lib_name(qubus_cuda_backend qubus_cuda_backend)
    qubus_set_output_name(qubus_cuda_backend qubus_cuda_backend)

    set_target_properties(qubus_cuda_backend PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${QUBUS_BUILD_PREFIX}/qubus/backends")
    set_target_properties(qubus_cuda_backend PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(qubus_cuda_backend PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

    set_property(TARGET qubus_cuda_backend PROPERTY VERSION ${QUBUS_VERSION})
    set_property(TARGET qubus_cuda_backend PROPERTY SOVERSION 0)
    set_property(TARGET qubus_cuda_backend PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
    set_property(TARGET qubus_cuda_backend PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
    set_property(TARGET qubus_cuda_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
    set_property(TARGET qubus_cuda_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)

    install(TARGETS qubus_cuda_backend EXPORT qubus-targets
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)

endif()