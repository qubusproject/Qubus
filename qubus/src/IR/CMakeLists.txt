find_package(carrot 0.2.0 REQUIRED)

set(qubus_ir_header_files access.hpp access_qualifier.hpp annotations.hpp
                          binary_operator_expr.hpp compound_expr.hpp constant_folding.hpp
                          construct_expr.hpp execution_order.hpp expression.hpp expression_traits.hpp
                          for_expr.hpp function.hpp if_expr.hpp intrinsic_function_expr.hpp
                          intrinsic_function_table.hpp literal_expr.hpp local_variable_def_expr.hpp
                          macro_expr.hpp member_access_expr.hpp module.hpp parsing.hpp pretty_printer.hpp
                          subscription_expr.hpp symbol_id.hpp type.hpp type_conversion_expr.hpp type_inference.hpp
                          unary_operator_expr.hpp variable_ref_expr.hpp variable_declaration.hpp function.hpp
                          integer_range_expr.hpp qir.hpp)

set(qubus_ir_source_files annotations.cpp binary_operator_expr.cpp compound_expr.cpp
                          for_expr.cpp intrinsic_function_expr.cpp intrinsic_function_table.cpp
                          literal_expr.cpp subscription_expr.cpp type.cpp
                          type_conversion_expr.cpp unary_operator_expr.cpp
                          variable_ref_expr.cpp variable_declaration.cpp
                          function.cpp macro_expr.cpp
                          local_variable_def_expr.cpp construct_expr.cpp
                          if_expr.cpp member_access_expr.cpp expression.cpp
                          pretty_printer.cpp type_inference.cpp
                          constant_folding.cpp module.cpp symbol_id.cpp parsing.cpp unique_variable_generator.cpp
                          integer_range_expr.cpp)

set(qubus_pattern_header_files access.hpp all_of.hpp any.hpp any_of.hpp binary_operator.hpp
                               bind_to.hpp compound.hpp construct.hpp contains.hpp control_flow.hpp core.hpp
                               fold.hpp for.hpp for_each.hpp if.hpp intrinsic_function.hpp IR.hpp
                               literal.hpp local_variable_def.hpp macro.hpp match.hpp matcher.hpp member_access.hpp
                               or.hpp pattern.hpp pattern_traits.hpp protect.hpp search.hpp sequence.hpp subscription.hpp
                               substitute.hpp tuple.hpp type.hpp type_conversion.hpp unary_operator.hpp value.hpp variable.hpp
                               variable_ref.hpp variable_scope.hpp typeof.hpp)

add_library(qubus_ir SHARED ${qubus_ir_source_files})
qubus_target_header_files(TARGET qubus_ir ROOT ${qubus_include_dir}/IR FILES ${qubus_ir_header_files})
qubus_target_header_files(TARGET qubus_ir ROOT ${qubus_include_dir}/pattern FILES ${qubus_pattern_header_files})

target_include_directories(qubus_ir PUBLIC ${HPX_INCLUDE_DIRS} ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)
target_compile_features(qubus_ir PUBLIC cxx_std_17)
target_link_libraries(qubus_ir qubus_util hpx carrot::carrot)
#set_target_properties(qubus_ir PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${QUBUS_HAS_LTO_SUPPORT})
set_property(TARGET qubus_ir PROPERTY POSITION_INDEPENDENT_CODE ON)

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