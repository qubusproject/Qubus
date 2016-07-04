cmake_minimum_required(VERSION 2.8)
project(qbb_util)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")