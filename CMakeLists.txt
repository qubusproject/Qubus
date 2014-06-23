cmake_minimum_required(VERSION 2.8)

project(kubus)

include(FeatureSummary)

find_package(Boost 1.55.0 COMPONENTS thread system REQUIRED)

find_package(CUDA 5.5)

file(GLOB kubus_header_files ${CMAKE_CURRENT_SOURCE_DIR}/include/qbb/kubus/*.hpp)
file(GLOB kubus_source_files src/*.cpp)

add_executable(kubus ${kubus_header_files}  main.cpp ${kubus_source_files})

target_include_directories(kubus PUBLIC ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(kubus PUBLIC -std=c++11 -Wall -Wextra -fsanitize=address)
target_link_libraries(kubus PUBLIC ${Boost_LIBRARIES} qbb_util -fsanitize=address)

feature_summary(WHAT ALL)