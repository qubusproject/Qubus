find_package(Boost 1.55.0 COMPONENTS log thread system REQUIRED)

find_package(CUDA 7.0)

find_package(LLVM 3.6.2 EXACT REQUIRED)

message(STATUS "found LLVM version: ${LLVM_PACKAGE_VERSION}")

list(FIND LLVM_AVAILABLE_LIBS LLVMIntelJITEvents LLVMIntelJITEvents_FIND_RESULT)

if(${LLVMIntelJITEvents_FIND_RESULT} LESS 0)
    set(LLVM_HAS_INTEL_JITEVENTS FALSE)
else()
    set(LLVM_HAS_INTEL_JITEVENTS TRUE)
endif()

if(LLVM_HAS_INTEL_JITEVENTS)
    set(REQ_LLVM_TARGETS IntelJITEvents mcjit ipa ipo vectorize native)
else()
    set(REQ_LLVM_TARGETS mcjit ipa ipo vectorize native)
endif()

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${REQ_LLVM_TARGETS})

message(STATUS "LLVM libraries: ${REQ_LLVM_LIBRARIES}")

find_package(HPX 0.9.9 REQUIRED)

set(QUBUS_BUILD_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(IR)
add_subdirectory(isl)
add_subdirectory(backends)

set(qubus_header_file_names allocator.hpp lower_abstract_indices.hpp
                            generic_ptr.hpp grammar.hpp index.hpp indexed_tensor_expr_context.hpp
                            IR_emitter.hpp loop_optimizer.hpp
                            lower_top_level_sums.hpp
                            memory_block.hpp object.hpp tensor_variable.hpp)

set(qubus_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qbb/qubus/)

foreach(file_name IN LISTS qubus_header_file_names)
    list(APPEND qubus_header_files ${qubus_include_dir}/${file_name})
endforeach()

set(qubus_source_files allocator.cpp lower_abstract_indices.cpp
                       loop_optimizer.cpp lower_top_level_sums.cpp deduce_iteration_space.cpp
                       runtime.cpp backend_registry.cpp local_address_space.cpp evicting_allocator.cpp
                       plan.cpp abi_info.cpp logging.cpp object_factory.cpp metadata_builder.cpp
                       make_implicit_conversions_explicit.cpp scheduler.cpp user_defined_plan.cpp object.cpp
                       tensor_expr_info.cpp multi_index_handling.cpp kronecker_delta_folding_pass.cpp)

add_library(qubus SHARED ${qubus_header_files} ${qubus_source_files})

target_include_directories(qubus PUBLIC ${HPX_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> ${LLVM_INCLUDE_DIRS})
target_include_directories(qubus PUBLIC $<BUILD_INTERFACE:${QBB_EXTERNAL_DIR}/boost/dll/include>)
target_compile_options(qubus PUBLIC -std=c++14)
target_link_libraries(qubus PUBLIC qubus_ir qubus_isl qbb_util
                                   hpx hpx_serialization ${Boost_LIBRARIES} dl wall)
target_compile_definitions(qubus PUBLIC -DBOOST_LOG_DYN_LINK)
add_dependencies(qubus qubus_cpu_backend)

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