cmake_minimum_required(VERSION 2.8)

project(kubus)

find_package(Boost COMPONENTS thread system REQUIRED)

add_executable(kubus main.cpp)

target_include_directories(kubus PUBLIC ${Boost_LIBRARY_DIRS})
target_compile_options(kubus PUBLIC -std=c++11 -Wall -Wextra)
target_link_libraries(kubus PUBLIC ${Boost_LIBRARIES})
