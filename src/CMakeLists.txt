file(GLOB_RECURSE QBB_header_files ${PROJECT_SOURCE_DIR}/QBB/*.hpp)
file(GLOB QBB_source_files ./*.cpp)

find_package(Boost 1.50.0 REQUIRED)
find_package(Eigen3 3.2.0 REQUIRED)

add_library(qbb ${QBB_header_files} ${QBB_source_files})

target_include_directories(qbb PUBLIC ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${QBB_ROOT_DIR})
target_compile_options(qbb PUBLIC -std=c++11 -pedantic -Wall -Wextra)
target_link_libraries(qbb ${Boost_LIBRARIES})