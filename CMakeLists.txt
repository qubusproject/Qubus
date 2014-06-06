cmake_minimum_required(VERSION 2.8)

project(kubus)

find_package(Boost COMPONENTS thread system REQUIRED)

add_executable(kubus main.cpp src/type.cpp src/object.cpp src/memory_type.cpp)

target_include_directories(kubus PUBLIC ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(kubus PUBLIC -std=c++11 -Wall -Wextra)
target_link_libraries(kubus PUBLIC ${Boost_LIBRARIES} qbb_util)
