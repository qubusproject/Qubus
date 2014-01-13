file(GLOB hfme_header_files ./*.hpp)
file(GLOB hfme_source_files ./*.cpp)

find_package(Boost 1.50.0 COMPONENTS program_options filesystem system REQUIRED)

add_executable(hfme ${hfme_header_files} ${hfme_source_files})

target_include_directories(qbb PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(hfme qbb ${Boost_LIBRARIES} echelon echelon_hdf5 hdf5_hl hdf5)