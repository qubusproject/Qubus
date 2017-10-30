find_package(CUDA 7.0)

if (CUDA_FOUND)

    set(qubus_cuda_source_files core.cpp support.cpp)

    add_library(qubus_cuda STATIC ${qubus_cuda_source_files})

    target_include_directories(qubus_cuda PUBLIC ${CUDA_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>
                                                  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
    target_compile_features(qubus_cuda PUBLIC cxx_std_17)
    target_link_libraries(qubus_cuda qubus_util ${CUDA_CUDA_LIBRARY} hpx)
    #set_target_properties(qubus_cuda PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${QUBUS_HAS_LTO_SUPPORT})

    set_target_properties(qubus_cuda PROPERTIES POSITION_INDEPENDENT_CODE ON)

    install(TARGETS qubus_cuda EXPORT qubus-targets
            RUNTIME DESTINATION bin
            INCLUDES DESTINATION include
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)

endif()