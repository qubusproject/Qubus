find_package(LLVM REQUIRED)

if (${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} VERSION_EQUAL 5.0)
    message(WARNING "Support for LLVM 5.0 is experimental.")
elseif (${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} VERSION_LESS 3.8)
    message(SEND_ERROR "Only LLVM >= 3.8 is supported. Please choose a suitable version.")
endif()

message(STATUS "found LLVM version: ${LLVM_PACKAGE_VERSION}")

list(FIND LLVM_AVAILABLE_LIBS LLVMIntelJITEvents LLVMIntelJITEvents_FIND_RESULT)

if (${LLVMIntelJITEvents_FIND_RESULT} LESS 0)
    set(LLVM_HAS_INTEL_JITEVENTS FALSE)
else ()
    set(LLVM_HAS_INTEL_JITEVENTS TRUE)
endif ()

if (LLVM_HAS_INTEL_JITEVENTS)
    set(REQ_LLVM_TARGETS IntelJITEvents executionengine orcjit mcjit transformutils linker analysis ipo vectorize native NVPTX)
else ()
    set(REQ_LLVM_TARGETS executionengine orcjit mcjit transformutils linker analysis ipo vectorize native NVPTX)
endif ()

add_library(qubus_llvm SHARED)
target_include_directories(qubus_llvm PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(qubus_llvm PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(qubus_llvm PRIVATE -Wl,--whole-archive ${LLVM_AVAILABLE_LIBS} -Wl,--no-whole-archive)

add_library(qubus_jit SHARED compiler.cpp compile.cpp llvm_environment.cpp entry_block_alloca.cpp load_store.cpp
            loops.cpp control_flow.cpp operators.cpp type_conversion.cpp array_access.cpp optimization_pipeline.cpp
            compilation_context.cpp reference.cpp jit_engine.cpp cpuinfo.cpp)
target_include_directories(qubus_jit PUBLIC ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_compile_options(qubus_jit PUBLIC -std=c++14)
target_link_libraries(qubus_jit ${Boost_LIBRARIES} qubus_ir qubus_util qubus_llvm)
hpx_setup_target(qubus_jit)
qubus_set_output_name(qubus_jit qubus_jit)

install(TARGETS qubus_llvm qubus_jit EXPORT qubus-targets
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)