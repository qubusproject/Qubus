find_package(LLVM REQUIRED)

if (${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} VERSION_EQUAL 9.0)
    message(WARNING "Support for LLVM 9.0 is experimental.")
elseif (${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} VERSION_LESS 6.0)
    message(SEND_ERROR "Only LLVM >= 6.0 is supported. Please choose a suitable version.")
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

if (NOT TARGET LLVM)
    message(ERROR "The shared version of the LLVM library is not available.")
endif()

add_library(qubus_llvm SHARED llvm_version.cpp)
target_include_directories(qubus_llvm PUBLIC ${LLVM_INCLUDE_DIRS} ${Boost_LIBRARY_DIRS})
target_compile_definitions(qubus_llvm PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(qubus_llvm PUBLIC LLVM ${Boost_LIBRARIES})

add_library(qubus_jit SHARED compiler.cpp compile.cpp llvm_environment.cpp entry_block_alloca.cpp load_store.cpp
            loops.cpp control_flow.cpp operators.cpp type_conversion.cpp array_access.cpp optimization_pipeline.cpp
            compilation_context.cpp reference.cpp jit_engine.cpp cpuinfo.cpp)
target_include_directories(qubus_jit PUBLIC ${HPX_INCLUDE_DIRS} ${Boost_LIBRARY_DIRS} $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/qubus/include> $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_compile_features(qubus_jit PUBLIC cxx_std_17)
target_link_libraries(qubus_jit ${Boost_LIBRARIES} qubus_ir qubus_util qubus_llvm hpx)
#set_target_properties(qubus_jit PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${QUBUS_HAS_LTO_SUPPORT})
set_property(TARGET qubus_jit PROPERTY POSITION_INDEPENDENT_CODE ON)

install(TARGETS qubus_llvm qubus_jit EXPORT qubus-targets
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)