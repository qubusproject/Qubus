set(qubus_ir_header_file_names annotations.hpp binary_operator_expr.hpp compound_expr.hpp
                               expression.hpp for_all_expr.hpp for_expr.hpp
                               intrinsic_function_expr.hpp intrinsic_function_table.hpp
                               literal_expr.hpp pretty_printer.hpp subscription_expr.hpp
                               sum_expr.hpp type.hpp
                               type_conversion_expr.hpp type_inference.hpp unary_operator_expr.hpp
                               variable_ref_expr.hpp variable_declaration.hpp function_declaration.hpp
                               qir.hpp)

set(qubus_ir_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qbb/qubus/IR/)

foreach(file_name IN LISTS qubus_ir_header_file_names)
    list(APPEND qubus_ir_header_files ${qubus_ir_include_dir}/${file_name})
endforeach()

set(qubus_ir_source_files annotations.cpp binary_operator_expr.cpp compound_expr.cpp
                          expression.cpp for_all_expr.cpp for_expr.cpp
                          intrinsic_function_expr.cpp intrinsic_function_table.cpp
                          literal_expr.cpp pretty_printer.cpp subscription_expr.cpp
                          sum_expr.cpp type.cpp
                          type_conversion_expr.cpp type_inference.cpp unary_operator_expr.cpp
                          variable_ref_expr.cpp variable_declaration.cpp
                          function_declaration.cpp macro_expr.cpp spawn_expr.cpp
                          local_variable_def_expr.cpp deduce_expression_environment.cpp construct_expr.cpp
                          if_expr.cpp kronecker_delta_expr.cpp member_access_expr.cpp foreign_call_expr.cpp)
        
set(qubus_pattern_header_file_names any.hpp binary_operator.hpp bind_to.hpp compound.hpp core.hpp
                                    for.hpp for_all.hpp index.hpp intrinsic_function.hpp IR.hpp
                                    literal.hpp match.hpp matcher.hpp pattern.hpp sequence.hpp subscription.hpp
                                    sum.hpp tensor.hpp type_conversion.hpp unary_operator.hpp variable.hpp value.hpp
                                    variable_ref.hpp)

set(qubus_pattern_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qbb/qubus/pattern/)

foreach(file_name IN LISTS qubus_pattern_header_file_names)
    list(APPEND qubus_pattern_header_files ${qubus_pattern_include_dir}/${file_name})
endforeach()

add_library(qubus_ir SHARED ${qubus_pattern_header_files} ${qubus_ir_header_files} ${qubus_ir_source_files}
            ../../include/qbb/qubus/IR/extract.hpp extract.cpp
            ../../include/qbb/qubus/pattern/contains.hpp
            ../../include/qbb/qubus/pattern/variable_scope.hpp)

target_include_directories(qubus_ir PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_compile_options(qubus_ir PUBLIC -std=c++14)
target_link_libraries(qubus_ir qbb_util)
hpx_setup_target(qubus_ir)
qubus_set_output_name(qubus_ir qubus_ir)

set_property(TARGET qubus_ir PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus_ir PROPERTY SOVERSION 0)
set_property(TARGET qubus_ir PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
set_property(TARGET qubus_ir PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
set_property(TARGET qubus_ir APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
set_property(TARGET qubus_ir APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)

install(TARGETS qubus_ir EXPORT qubus-targets
                         RUNTIME DESTINATION bin
                         INCLUDES DESTINATION include
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib)