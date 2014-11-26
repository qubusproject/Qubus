set(kubus_isl_header_file_names affine_expr.hpp ast.hpp ast_builder.hpp constraint.hpp
                                context.hpp flow.hpp local_space.hpp map.hpp
                                printer.hpp schedule.hpp set.hpp space.hpp value.hpp
                                id.hpp band.hpp)
               
set(kubus_isl_include_dir ${CMAKE_SOURCE_DIR}/kubus/include/qbb/kubus/isl)

foreach(file_name IN LISTS kubus_isl_header_file_names)
    list(APPEND kubus_isl_header_files ${kubus_isl_include_dir}/${file_name})
endforeach()

set(kubus_isl_source_files affine_expr.cpp ast.cpp ast_builder.cpp constraint.cpp
                           context.cpp flow.cpp local_space.cpp map.cpp
                           printer.cpp schedule.cpp set.cpp space.cpp value.cpp
                           id.cpp band.cpp schedule_node.cpp
                           multi_union_pw_affine_expr.cpp)

add_library(qbb_kubus_isl SHARED ${kubus_isl_header_files} ${kubus_isl_source_files})

target_include_directories(qbb_kubus_isl PUBLIC ${CMAKE_SOURCE_DIR}/kubus/include)

target_compile_options(qbb_kubus_isl PUBLIC -std=c++14)
target_link_libraries(qbb_kubus_isl PUBLIC isl qbb_util)

install(TARGETS qbb_kubus_isl LIBRARY DESTINATION lib)
