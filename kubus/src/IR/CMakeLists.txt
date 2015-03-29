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
                          function_declaration.cpp macro_expr.cpp)
        
set(kubus_pattern_header_file_names any.hpp binary_operator.hpp bind_to.hpp compound.hpp core.hpp
                                    for.hpp for_all.hpp index.hpp intrinsic_function.hpp IR.hpp
                                    literal.hpp match.hpp matcher.hpp pattern.hpp sequence.hpp subscription.hpp
                                    sum.hpp tensor.hpp type_conversion.hpp unary_operator.hpp variable.hpp value.hpp
                                    variable_ref.hpp)

set(kubus_pattern_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qbb/kubus/pattern/)

foreach(file_name IN LISTS kubus_pattern_header_file_names)
    list(APPEND kubus_pattern_header_files ${kubus_pattern_include_dir}/${file_name})
endforeach()

add_library(qbb_kubus_ir SHARED ${kubus_pattern_header_files} ${kubus_ir_header_files} ${kubus_ir_source_files})

target_include_directories(qbb_kubus_ir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_definitions(qbb_kubus_ir PUBLIC -DFUSION_MAX_VECTOR_SIZE=20)
target_compile_options(qbb_kubus_ir PUBLIC -std=c++14)
target_link_libraries(qbb_kubus_ir PUBLIC qbb_util)

install(TARGETS qbb_kubus_ir PUBLIC_HEADER DESTINATION include LIBRARY DESTINATION lib)