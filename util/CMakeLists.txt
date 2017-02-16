project(qubus_util)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")