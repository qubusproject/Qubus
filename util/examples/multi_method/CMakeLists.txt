add_executable(multi_method main.cpp)
target_compile_options(multi_method PUBLIC -funroll-loops)
target_link_libraries(multi_method PUBLIC qubus_util)

install(TARGETS multi_method RUNTIME DESTINATION bin)
