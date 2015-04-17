set(kubus_ir_header_file_names annotations.hpp binary_operator_expr.hpp compound_expr.hpp
                               expression.hpp for_all_expr.hpp for_expr.hpp
                               intrinsic_function_expr.hpp intrinsic_function_table.hpp
                               literal_expr.hpp pretty_printer.hpp subscription_expr.hpp
                               sum_expr.hpp type.hpp
                               type_conversion_expr.hpp type_inference.hpp unary_operator_expr.hpp
                               variable_ref_expr.hpp variable_declaration.hpp function_declaration.hpp
                               kir.hpp)

set(kubus_ir_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qbb/kubus/IR/)

foreach(file_name IN LISTS kubus_ir_header_file_names)
    list(APPEND kubus_ir_header_files ${kubus_ir_include_dir}/${file_name})
endforeach()

set(kubus_ir_source_files annotations.cpp binary_operator_expr.cpp compound_expr.cpp
                          expression.cpp for_all_expr.cpp for_expr.cpp
                          intrinsic_function_expr.cpp intrinsic_function_table.cpp
                          literal_expr.cpp pretty_printer.cpp subscription_expr.cpp
                          sum_expr.cpp type.cpp
                          type_conversion_expr.cpp type_inference.cpp unary_operator_expr.cpp
                          variable_ref_expr.cpp variable_declaration.cpp
                          function_declaration.cpp macro_expr.cpp spawn_expr.cpp
                          local_variable_def_expr.cpp)
        
set(kubus_pattern_header_file_names any.hpp binary_operator.hpp bind_to.hpp compound.hpp core.hpp
                                    for.hpp for_all.hpp index.hpp intrinsic_function.hpp IR.hpp
                                    literal.hpp match.hpp matcher.hpp pattern.hpp sequence.hpp subscription.hpp
                                    sum.hpp tensor.hpp type_conversion.hpp unary_operator.hpp variable.hpp value.hpp
                                    variable_ref.hpp)

set(kubus_pattern_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qbb/kubus/pattern/)

foreach(file_name IN LISTS kubus_pattern_header_file_names)
    list(APPEND kubus_pattern_header_files ${kubus_pattern_include_dir}/${file_name})
endforeach()

add_library(kubus_ir SHARED ${kubus_pattern_header_files} ${kubus_ir_header_files} ${kubus_ir_source_files})

target_include_directories(kubus_ir PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/kubus/include>)
target_compile_definitions(kubus_ir PUBLIC -DFUSION_MAX_VECTOR_SIZE=20)
target_compile_options(kubus_ir PUBLIC -std=c++14)
target_link_libraries(kubus_ir PUBLIC qbb_util)

set_property(TARGET kubus_ir PROPERTY VERSION ${KUBUS_VERSION})
set_property(TARGET kubus_ir PROPERTY SOVERSION 0)
set_property(TARGET kubus_ir PROPERTY INTERFACE_kubus_MAJOR_VERSION 0)
set_property(TARGET kubus_ir PROPERTY INTERFACE_kubus_MINOR_VERSION 1)
set_property(TARGET kubus_ir APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MAJOR_VERSION)
set_property(TARGET kubus_ir APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MINOR_VERSION)

install(TARGETS kubus_ir EXPORT kubus-targets
                         RUNTIME DESTINATION bin
                         INCLUDES DESTINATION include
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib)