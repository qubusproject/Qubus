set(util_header_file_names assert.hpp function_traits.hpp handle.hpp integer_sequence.hpp integers.hpp make_array.hpp
                           make_unique.hpp multi_array.hpp multi_method.hpp nested_for_each.hpp push_back.hpp
                           push_front.hpp unique_name_generator.hpp unreachable.hpp unused.hpp optional_ref.hpp
                           hash.hpp
                           detail/dispatch_table.hpp detail/virtual.hpp
                           meta/contains.hpp meta/expand.hpp meta/type_sequence.hpp meta/unique.hpp
                           numeric/bisection.hpp)

set(util_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/../include/qbb/util/)

foreach(file_name IN LISTS util_header_file_names)
    list(APPEND util_header_files ${util_include_dir}/${file_name})
endforeach()

set(util_source_files handle.cpp unique_name_generator.cpp dll.cpp get_prefix.cpp)

find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(Threads REQUIRED)

add_library(qbb_util ${util_header_files} ${util_source_files})
target_compile_options(qbb_util PUBLIC -std=c++14 -Wall -Wextra)
target_include_directories(qbb_util PUBLIC ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include/)
target_link_libraries(qbb_util PUBLIC Threads::Threads ${Boost_LIBRARIES})
set_target_properties(qbb_util PROPERTIES POSITION_INDEPENDENT_CODE True)

install(TARGETS qbb_util PUBLIC_HEADER DESTINATION include ARCHIVE DESTINATION lib)