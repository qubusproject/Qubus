cmake_minimum_required(VERSION 2.8)
project(multi_method)

add_executable(multi_method examples/multi_method/main.cpp)
target_compile_options(multi_method PUBLIC -std=c++11 -Wall -Wextra -pthread -funroll-loops)
target_include_directories(multi_method PUBLIC ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(multi_method -pthread)

install(TARGETS multi_method RUNTIME DESTINATION bin)
