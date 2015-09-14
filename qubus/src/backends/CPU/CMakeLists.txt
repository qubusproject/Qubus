
add_library(qubus_cpu_backend SHARED cpu_backend.cpp llvm_environment.cpp cpuinfo.cpp cpu_object_factory.cpp
                                     cpu_memory_block.cpp cpu_allocator.cpp local_array_alias_analysis.cpp)

target_include_directories(qubus_cpu_backend PUBLIC ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}> PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(qubus_cpu_backend PUBLIC -std=c++14)
target_compile_definitions(qubus_cpu_backend PRIVATE ${LLVM_DEFINITIONS})
target_link_libraries(qubus_cpu_backend PUBLIC ${Boost_LIBRARIES} qubus_ir qbb_util wall PRIVATE ${REQ_LLVM_LIBRARIES})

set_target_properties(qubus_cpu_backend PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${QUBUS_BUILD_PREFIX}/qubus/backends")
set_target_properties(qubus_cpu_backend PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(qubus_cpu_backend PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

set_property(TARGET qubus_cpu_backend PROPERTY VERSION ${QUBUS_VERSION})
set_property(TARGET qubus_cpu_backend PROPERTY SOVERSION 0)
set_property(TARGET qubus_cpu_backend PROPERTY INTERFACE_qubus_MAJOR_VERSION 0)
set_property(TARGET qubus_cpu_backend PROPERTY INTERFACE_qubus_MINOR_VERSION 1)
set_property(TARGET qubus_cpu_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MAJOR_VERSION)
set_property(TARGET qubus_cpu_backend APPEND PROPERTY COMPATIBLE_INTERFACE_STRING qubus_MINOR_VERSION)

install(TARGETS qubus_cpu_backend EXPORT qubus-targets
                                  RUNTIME DESTINATION bin
                                  INCLUDES DESTINATION include
                                  LIBRARY DESTINATION lib
                                  ARCHIVE DESTINATION lib)