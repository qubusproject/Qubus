set(qubus_qtl_header_file_names )

set(qubus_qtl_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qubus/qtl/)

foreach(file_name IN LISTS qubus_qtl_header_file_names)
    list(APPEND qubus_qtl_header_files ${qubus_qtl_include_dir}/${file_name})
endforeach()

set(qubus_qtl_source_files deduce_iteration_space.cpp index.cpp kronecker_delta_folding_pass.cpp
                           lower_abstract_indices.cpp lower_top_level_sums.cpp multi_index_handling.cpp
                           sparse_patterns.cpp IR/for_all_expr.cpp IR/kronecker_delta_expr.cpp IR/sum_expr.cpp)

add_library(qubus_qtl SHARED ${qubus_qtl_header_files} ${qubus_qtl_source_files})

target_include_directories(qubus_qtl PUBLIC ${Boost_INCLUDE_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_compile_options(qubus_qtl PUBLIC -std=c++14)
target_link_libraries(qubus_qtl PUBLIC qubus qubus_ir qubus_util)
target_link_libraries(qubus_qtl PUBLIC ${Boost_LIBRARIES})
hpx_setup_target(qubus_qtl)
qubus_set_output_name(qubus_qtl qubus_qtl)

set_property(TARGET qubus_qtl PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus_qtl PROPERTY SOVERSION 0)
set_property(TARGET qubus_qtl PROPERTY INTERFACE_qubus_qtl_MAJOR_VERSION 0)
set_property(TARGET qubus_qtl PROPERTY INTERFACE_qubus_qtl_MINOR_VERSION 1)
set_property(TARGET qubus_qtl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_qtl_MAJOR_VERSION)
set_property(TARGET qubus_qtl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_qtl_MINOR_VERSION)

install(TARGETS qubus_qtl EXPORT qubus-targets
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)