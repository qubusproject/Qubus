
add_library(kubus_cpu_backend SHARED cpu_backend.cpp llvm_environment.cpp cpuinfo.cpp cpu_object_factory.cpp
                                     cpu_memory_block.cpp cpu_allocator.cpp local_array_alias_analysis.cpp)

target_include_directories(kubus_cpu_backend PUBLIC ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/kubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}> PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(kubus_cpu_backend PUBLIC -std=c++14)
target_compile_definitions(kubus_cpu_backend PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(kubus_cpu_backend PUBLIC ${Boost_LIBRARIES} kubus_ir qbb_util wall PRIVATE ${REQ_LLVM_LIBRARIES})

set_target_properties(kubus_cpu_backend PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${QUBUS_BUILD_PREFIX}/kubus/backends")
set_target_properties(kubus_cpu_backend PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(kubus_cpu_backend PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

set_property(TARGET kubus_cpu_backend PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET kubus_cpu_backend PROPERTY SOVERSION 0)
set_property(TARGET kubus_cpu_backend PROPERTY INTERFACE_kubus_MAJOR_VERSION 0)
set_property(TARGET kubus_cpu_backend PROPERTY INTERFACE_kubus_MINOR_VERSION 1)
set_property(TARGET kubus_cpu_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MAJOR_VERSION)
set_property(TARGET kubus_cpu_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING kubus_MINOR_VERSION)

install(TARGETS kubus_cpu_backend EXPORT kubus-targets
                                  RUNTIME DESTINATION bin
                                  INCLUDES DESTINATION include
                                  LIBRARY DESTINATION lib
                                  ARCHIVE DESTINATION lib)