
add_library(kubus_cpu_backend SHARED cpu_backend.cpp llvm_environment.cpp cpuinfo.cpp cpu_object_factory.cpp
                                     cpu_memory_block.cpp cpu_allocator.cpp)

target_include_directories(kubus_cpu_backend PUBLIC ${Boost_LIBRARY_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(kubus_cpu_backend PUBLIC -std=c++14)
target_compile_definitions(kubus_cpu_backend PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(kubus_cpu_backend PUBLIC ${Boost_LIBRARIES} qbb_kubus_ir qbb_util wall PRIVATE ${REQ_LLVM_LIBRARIES})

set_target_properties(kubus_cpu_backend PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${KUBUS_BUILD_PREFIX}/qbb/kubus/backends")

install(TARGETS kubus_cpu_backend LIBRARY DESTINATION lib/qbb/kubus/backends)