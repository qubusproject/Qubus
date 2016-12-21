find_package(Boost 1.62.0 COMPONENTS log thread system REQUIRED)

find_package(CUDA 7.0)

find_package(HPX 0.9.12 REQUIRED)

set(QUBUS_BUILD_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(IR)
add_subdirectory(isl)
add_subdirectory(jit)
add_subdirectory(backends)

set(qubus_header_file_names allocator.hpp generic_ptr.hpp loop_optimizer.hpp
                            memory_block.hpp object.hpp)

set(qubus_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qbb/qubus/)

foreach(file_name IN LISTS qubus_header_file_names)
    list(APPEND qubus_header_files ${qubus_include_dir}/${file_name})
endforeach()

set(qubus_source_files allocator.cpp loop_optimizer.cpp
                       local_runtime.cpp backend_registry.cpp local_address_space.cpp evicting_allocator.cpp
                       abi_info.cpp logging.cpp object_factory.cpp
                       make_implicit_conversions_explicit.cpp scheduler.cpp object.cpp
                       runtime.cpp computelet.cpp host_allocator.cpp
                       local_object_factory.cpp vpu.cpp aggregate_vpu.cpp
                       architecture_identifier.cpp object_monitor.cpp token.cpp)

add_library(qubus SHARED ${qubus_header_files} ${qubus_source_files})

target_include_directories(qubus PUBLIC ${Boost_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_compile_options(qubus PUBLIC -std=c++14)
target_link_libraries(qubus PUBLIC qubus_ir qubus_isl qbb_util)
target_link_libraries(qubus PUBLIC ${Boost_LIBRARIES} qubus_cpu_backend dl)
target_compile_definitions(qubus PUBLIC -DBOOST_LOG_DYN_LINK)
add_dependencies(qubus qubus_cpu_backend)
hpx_setup_target(qubus COMPONENT_DEPENDENCIES iostreams)
qubus_set_output_name(qubus qubus)

set_property(TARGET qubus PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus PROPERTY SOVERSION 0)
set_property(TARGET qubus PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
set_property(TARGET qubus PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
set_property(TARGET qubus APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
set_property(TARGET qubus APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)
        
install(TARGETS qubus EXPORT qubus-targets
                      RUNTIME DESTINATION bin
                      INCLUDES DESTINATION include
                      LIBRARY DESTINATION lib
                      ARCHIVE DESTINATION lib)

add_subdirectory(qtl)