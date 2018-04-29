set(CMAKE_REQUIRED_FLAGS -fsanitize=fuzzer)
set(CMAKE_REQUIRED_LIBRARIES -fsanitize=fuzzer)
check_cxx_source_compiles("#include <cstdint>
                           extern \"C\" int LLVMFuzzerTestOneInput(const std::uint8_t* Data, std::size_t Size){return 0;}" QUBUS_HAS_FUZZING_SUPPORT)
unset(CMAKE_REQUIRED_FLAGS)
unset(CMAKE_REQUIRED_LIBRARIES)

# Disable fuzzing support since it is currently broken.
set(QUBUS_HAS_FUZZING_SUPPORT OFF)

if (QUBUS_HAS_FUZZING_SUPPORT)
    message(STATUS "Fuzzing is supported.")

    set(QUBUS_FUZZING_FLAGS -fsanitize=fuzzer)
    set(QUBUS_FUZZING_LINK_FLAGS -fsanitize=fuzzer)
else()
    message(STATUS "Fuzzing is not supported.")
endif()