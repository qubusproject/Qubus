cmake_minimum_required(VERSION 2.8)

project(kubus)

add_library(asan INTERFACE)
target_compile_options(asan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=address -fno-omit-frame-pointer>)
target_link_libraries(asan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=address -fno-omit-frame-pointer>)

add_library(tsan INTERFACE)
target_compile_options(tsan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=thread -fPIC>)
target_link_libraries(tsan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=thread -fPIC>)

add_library(usan INTERFACE)
target_compile_options(usan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=undefined>)
target_link_libraries(usan INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fsanitize=undefined>)

add_library(wall INTERFACE)
target_compile_options(wall INTERFACE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra>)

include(FeatureSummary)

add_subdirectory(src/IR)
add_subdirectory(src/isl)
add_subdirectory(src)

add_executable(kubus_test main.cpp)
target_link_libraries(kubus_test kubus)

feature_summary(WHAT ALL)