cmake_minimum_required(VERSION 2.8)

project(kubus)

add_subdirectory(src/IR)
add_subdirectory(src/isl)

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

find_package(Boost 1.55.0 COMPONENTS thread system REQUIRED)

find_package(CUDA 5.5)

find_package(LLVM 3.5 EXACT REQUIRED)

message(STATUS "found LLVM version: ${LLVM_PACKAGE_VERSION}")

llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES mcjit ipo native NVPTX)

message(STATUS "LLVM libraries: ${REQ_LLVM_LIBRARIES}")

file(GLOB kubus_header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/qbb/kubus/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/include/qbb/kubus/IR/*.hpp)
file(GLOB kubus_source_files src/*.cpp src/IR/*.cpp)

add_executable(kubus ${kubus_header_files}  main.cpp ${kubus_source_files})

target_include_directories(kubus PUBLIC ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(kubus PUBLIC -std=c++11)
target_link_libraries(kubus PUBLIC ${Boost_LIBRARIES} qbb_kubus_isl qbb_util wall)

feature_summary(WHAT ALL)