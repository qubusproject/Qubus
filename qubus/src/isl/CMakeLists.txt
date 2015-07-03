set(kubus_isl_header_file_names affine_expr.hpp ast.hpp ast_builder.hpp constraint.hpp
                                context.hpp flow.hpp local_space.hpp map.hpp
                                printer.hpp schedule.hpp set.hpp space.hpp value.hpp
                                id.hpp band.hpp)
               
set(kubus_isl_include_dir ${CMAKE_SOURCE_DIR}/qubus/include/qbb/qubus/isl)

foreach(file_name IN LISTS kubus_isl_header_file_names)
    list(APPEND kubus_isl_header_files ${kubus_isl_include_dir}/${file_name})
endforeach()

set(kubus_isl_source_files affine_expr.cpp ast.cpp ast_builder.cpp constraint.cpp
                           context.cpp flow.cpp local_space.cpp map.cpp
                           printer.cpp schedule.cpp set.cpp space.cpp value.cpp
                           id.cpp band.cpp schedule_node.cpp
                           multi_union_pw_affine_expr.cpp pw_aff.cpp pw_multi_aff.cpp)

find_package(ISL MODULE REQUIRED)               

add_library(kubus_isl SHARED ${kubus_isl_header_files} ${kubus_isl_source_files})

target_include_directories(kubus_isl PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include>)

target_compile_options(kubus_isl PUBLIC -std=c++14)
target_link_libraries(kubus_isl PUBLIC isl::isl qbb_util)

set_property(TARGET kubus_isl PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET kubus_isl PROPERTY SOVERSION 0)
set_property(TARGET kubus_isl PROPERTY INTERFACE_kubus_MAJOR_VERSION 0)
set_property(TARGET kubus_isl PROPERTY INTERFACE_kubus_MINOR_VERSION 1)
set_property(TARGET kubus_isl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MAJOR_VERSION)
set_property(TARGET kubus_isl APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MINOR_VERSION)

install(TARGETS kubus_isl EXPORT kubus-targets
                          RUNTIME DESTINATION bin
                          INCLUDES DESTINATION include
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)
