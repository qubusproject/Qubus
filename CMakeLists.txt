cmake_minimum_required(VERSION 2.8)
project(multi_method)

add_library(qbb_util SHARED src/handle.cpp )
target_compile_options(qbb_util PUBLIC -std=c++11 -Wall -Wextra -pthread)
target_include_directories(qbb_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(qbb_util -pthread)

install(TARGETS qbb_util PUBLIC_HEADER DESTINATION include LIBRARY DESTINATION lib)

add_subdirectory(examples)