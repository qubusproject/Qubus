find_package(LLVM 3.8.0 EXACT REQUIRED)

message(STATUS "found LLVM version: ${LLVM_PACKAGE_VERSION}")

list(FIND LLVM_AVAILABLE_LIBS LLVMIntelJITEvents LLVMIntelJITEvents_FIND_RESULT)

if (${LLVMIntelJITEvents_FIND_RESULT} LESS 0)
    set(LLVM_HAS_INTEL_JITEVENTS FALSE)
else ()
    set(LLVM_HAS_INTEL_JITEVENTS TRUE)
endif ()

if (LLVM_HAS_INTEL_JITEVENTS)
    set(REQ_LLVM_TARGETS IntelJITEvents mcjit analysis ipo vectorize native)
else ()
    set(REQ_LLVM_TARGETS mcjit analysis ipo vectorize native)
endif ()

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES ${REQ_LLVM_TARGETS})

message(STATUS "LLVM libraries: ${REQ_LLVM_LIBRARIES}")

add_library(qubus_jit SHARED compiler.cpp compile.cpp llvm_environment.cpp local_array_alias_analysis.cpp entry_block_alloca.cpp load_store.cpp
            loops.cpp control_flow.cpp operators.cpp type_conversion.cpp array_access.cpp optimization_pipeline.cpp compilation_context.cpp reference.cpp)
target_include_directories(qubus_jit PUBLIC ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}> ${LLVM_INCLUDE_DIRS})
target_compile_options(qubus_jit PUBLIC -std=c++14)
target_compile_definitions(qubus_jit PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(qubus_jit PUBLIC ${Boost_LIBRARIES} qubus_ir qbb_util wall ${REQ_LLVM_LIBRARIES})