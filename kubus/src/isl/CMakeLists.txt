file(GLOB qbb_kubus_isl_header_files ${CMAKE_SOURCE_DIR}/kubus/include/qbb/kubus/isl/*.hpp)
file(GLOB qbb_kubus_isl_source_files *.cpp)

add_library(qbb_kubus_isl SHARED ${qbb_kubus_isl_header_files} ${qbb_kubus_isl_source_files})

target_include_directories(qbb_kubus_isl PUBLIC ${CMAKE_SOURCE_DIR}/kubus/include)

target_compile_options(qbb_kubus_isl PUBLIC $<$<CXX_COMPILER_ID:GNU>:-flto> $<$<CXX_COMPILER_ID:Clang>:-flto> -std=c++11)
target_link_libraries(qbb_kubus_isl PUBLIC $<$<CXX_COMPILER_ID:GNU>:-flto> $<$<CXX_COMPILER_ID:Clang>:-flto> isl)