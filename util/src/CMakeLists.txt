set(util_header_file_names assert.hpp function_traits.hpp handle.hpp integer_sequence.hpp integers.hpp make_array.hpp
                           make_unique.hpp multi_array.hpp multi_method.hpp nested_for_each.hpp push_back.hpp
                           push_front.hpp unique_name_generator.hpp unreachable.hpp unused.hpp
                           detail/dispatch_table.hpp detail/virtual.hpp
                           meta/contains.hpp meta/expand.hpp meta/type_sequence.hpp meta/unique.hpp)

set(util_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qbb/util/)

foreach(file_name IN LISTS util_header_file_names)
    list(APPEND util_header_files ${util_include_dir}/${file_name})
endforeach()

set(util_source_files handle.cpp unique_name_generator.cpp)

add_library(qbb_util SHARED ${util_header_files} ${util_source_files})
target_compile_options(qbb_util PUBLIC -std=c++14 -Wall -Wextra -pthread)
target_include_directories(qbb_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
target_link_libraries(qbb_util PUBLIC -pthread)

install(TARGETS qbb_util PUBLIC_HEADER DESTINATION include LIBRARY DESTINATION lib)