find_package(Boost 1.55.0 COMPONENTS thread system REQUIRED)

find_package(CUDA 5.5)

find_package(LLVM 3.5 EXACT REQUIRED)

message(STATUS "found LLVM version: ${LLVM_PACKAGE_VERSION}")

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES mcjit ipo native NVPTX)

message(STATUS "LLVM libraries: ${REQ_LLVM_LIBRARIES}")

find_package(HPX 0.9.9 REQUIRED)

set(kubus_header_file_names allocator.hpp deduce_loop_bounds_pass.hpp emit_implicit_loops_pass.hpp
                            generic_ptr.hpp grammar.hpp index.hpp indexed_tensor_expr_context.hpp
                            IR_emitter.hpp loop_optimizer.hpp
                            lower_top_level_sums_pass.hpp managed_memory_block.hpp
                            memory_block.hpp memory_type.hpp object.hpp tensor_variable.hpp)

set(kubus_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qbb/kubus/)

foreach(file_name IN LISTS kubus_header_file_names)
    list(APPEND kubus_header_files ${kubus_include_dir}/${file_name})
endforeach()

set(kubus_source_files allocator.cpp deduce_loop_bounds_pass.cpp
                       emit_implicit_loops_pass.cpp
                       loop_optimizer.cpp lower_top_level_sums_pass.cpp
                       memory_type.cpp object.cpp)

add_library(kubus SHARED ${kubus_header_files} ${kubus_source_files})

target_include_directories(kubus PUBLIC ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include ${LLVM_INCLUDE_DIRS})
target_compile_options(kubus PUBLIC -std=c++14)
target_link_libraries(kubus PUBLIC ${Boost_LIBRARIES} qbb_kubus_ir qbb_kubus_isl qbb_util wall)

install(TARGETS kubus PUBLIC_HEADER DESTINATION include LIBRARY DESTINATION lib)