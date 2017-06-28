set(qubus_isl_header_file_names affine_expr.hpp ast.hpp ast_builder.hpp constraint.hpp
                                context.hpp flow.hpp local_space.hpp map.hpp
                                printer.hpp schedule.hpp set.hpp space.hpp value.hpp
                                id.hpp band.hpp)
               
set(qubus_isl_include_dir ${CMAKE_SOURCE_DIR}/qubus/include/qubus/isl)

foreach(file_name IN LISTS qubus_isl_header_file_names)
    list(APPEND qubus_isl_header_files ${qubus_isl_include_dir}/${file_name})
endforeach()

set(qubus_isl_source_files affine_expr.cpp ast.cpp ast_builder.cpp constraint.cpp
                           context.cpp flow.cpp local_space.cpp map.cpp
                           printer.cpp schedule.cpp set.cpp space.cpp value.cpp
                           id.cpp band.cpp schedule_node.cpp
                           multi_union_pw_affine_expr.cpp pw_aff.cpp pw_multi_aff.cpp multi_affine_expr.cpp)

find_package(ISL MODULE REQUIRED)               

add_library(qubus_isl SHARED ${qubus_isl_header_files} ${qubus_isl_source_files})

target_include_directories(qubus_isl PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)

target_compile_features(qubus_isl PUBLIC cxx_std_14)
target_link_libraries(qubus_isl PUBLIC isl::isl qubus_util)

set_property(TARGET qubus_isl PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus_isl PROPERTY SOVERSION 0)
set_property(TARGET qubus_isl PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
set_property(TARGET qubus_isl PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
set_property(TARGET qubus_isl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
set_property(TARGET qubus_isl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)

install(TARGETS qubus_isl EXPORT qubus-targets
                          RUNTIME DESTINATION bin
                          INCLUDES DESTINATION include
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
