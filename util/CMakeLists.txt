cmake_minimum_required(VERSION 2.8)
project(multi_method)

file(GLOB util_header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/qbb/kubus/*.hpp)
file(GLOB util_source_files src/*.cpp)

add_library(qbb_util SHARED ${util_header_files} ${util_source_files})
target_compile_options(qbb_util PUBLIC -std=c++14 -Wall -Wextra -pthread)
target_include_directories(qbb_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(qbb_util -pthread)

install(TARGETS qbb_util PUBLIC_HEADER DESTINATION include LIBRARY DESTINATION lib)

add_subdirectory(examples)