include(CMakeFindDependencyMacro)

include(${CMAKE_CURRENT_LIST_DIR}/qubus-macros.cmake)

find_dependency(Threads)
find_dependency(ISL)
find_dependency(LLVM 3.6 EXACT)
find_dependency(HPX 0.9.9)
find_dependency(Boost 1.55.0)

find_dependency(CUDA 5.5)

include("${CMAKE_CURRENT_LIST_DIR}/qubus-targets.cmake")