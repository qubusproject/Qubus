find_package(Boost 1.62.0 COMPONENTS log regex thread system REQUIRED)

find_package(HPX 0.9.12 REQUIRED)

find_package(Eigen3 REQUIRED)

set(QUBUS_BUILD_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(IR)
add_subdirectory(isl)
add_subdirectory(jit)
add_subdirectory(cuda)
add_subdirectory(backends)

add_library(qubus_build_prefix STATIC prefix.cpp)
target_include_directories(qubus_build_prefix PUBLIC ${Boost_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_link_libraries(qubus_build_prefix PUBLIC ${Boost_LIBRARIES})
set_target_properties(qubus_build_prefix PROPERTIES POSITION_INDEPENDENT_CODE True)
target_compile_definitions(qubus_build_prefix PRIVATE -DQUBUS_PREFIX=${CMAKE_CURRENT_BINARY_DIR})

add_library(qubus_prefix STATIC prefix.cpp)
target_include_directories(qubus_prefix PUBLIC ${Boost_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_link_libraries(qubus_prefix PUBLIC ${Boost_LIBRARIES})
set_target_properties(qubus_prefix PROPERTIES POSITION_INDEPENDENT_CODE True)
target_compile_definitions(qubus_prefix PRIVATE -DQUBUS_PREFIX=${CMAKE_INSTALL_PREFIX})

set(qubus_header_file_names allocator.hpp generic_ptr.hpp loop_optimizer.hpp
                            memory_block.hpp object.hpp)

set(qubus_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qubus/)

foreach(file_name IN LISTS qubus_header_file_names)
    list(APPEND qubus_header_files ${qubus_include_dir}/${file_name})
endforeach()

set(qubus_source_files allocator.cpp loop_optimizer.cpp
                       local_runtime.cpp backend_registry.cpp local_address_space.cpp evicting_allocator.cpp
                       abi_info.cpp logging.cpp object_factory.cpp
                       make_implicit_conversions_explicit.cpp scheduling/round_robin_scheduler.cpp object.cpp
                       runtime.cpp computelet.cpp host_allocator.cpp
                       local_object_factory.cpp vpu.cpp aggregate_vpu.cpp
                       architecture_identifier.cpp object_monitor.cpp token.cpp
                       pass_manager.cpp variable_access_analysis.cpp alias_analysis.cpp axiom_analysis.cpp
                       task_invariants_analysis.cpp affine_constraints.cpp value_set_analysis.cpp
                       value_range_analysis.cpp static_schedule.cpp static_schedule_analysis.cpp
                       performance_models/unified_performance_model.cpp performance_models/simple_statistical_performance_model.cpp
                       performance_models/symbolic_regression.cpp performance_models/regression_performance_model.cpp object_instance.cpp
                       virtual_address_space.cpp basic_address_space.cpp scheduling/uniform_fill_scheduler.cpp global_id.cpp foreign_computelet.cpp)

add_library(qubus SHARED ${qubus_header_files} ${qubus_source_files})

set_target_properties(qubus PROPERTIES INTERFACE_LINK_LIBRARIES "$<BUILD_INTERFACE:qubus_build_prefix>;$<INSTALL_INTERFACE:qubus_prefix>")
target_include_directories(qubus PUBLIC ${Boost_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_compile_features(qubus PUBLIC cxx_std_14)
target_link_libraries(qubus qubus_ir qubus_isl qubus_util ${Boost_LIBRARIES} qubus_cpu_backend hpx)

if (TARGET qubus_cuda_backend)
    target_link_libraries(qubus qubus_cuda_backend)
endif()

target_compile_definitions(qubus PUBLIC -DBOOST_LOG_DYN_LINK)
add_dependencies(qubus qubus_cpu_backend)

if (TARGET qubus_cuda_backend)
    add_dependencies(qubus qubus_cuda_backend)
endif()

set_target_properties(qubus PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${QUBUS_HAS_LTO_SUPPORT})

set_property(TARGET qubus PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus PROPERTY SOVERSION 0)
set_property(TARGET qubus PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
set_property(TARGET qubus PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
set_property(TARGET qubus APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
set_property(TARGET qubus APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)
        
install(TARGETS qubus qubus_prefix
        EXPORT qubus-targets
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

add_subdirectory(qtl)