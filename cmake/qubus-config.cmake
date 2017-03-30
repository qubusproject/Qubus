include(CMakeFindDependencyMacro)

include(${CMAKE_CURRENT_LIST_DIR}/qubus-macros.cmake)

find_dependency(Threads)
find_dependency(ISL)
find_dependency(LLVM)
find_dependency(HPX 1.0.0)
find_dependency(Boost 1.62.0)

find_dependency(CUDA 7.0)

include("${CMAKE_CURRENT_LIST_DIR}/qubus-targets.cmake")