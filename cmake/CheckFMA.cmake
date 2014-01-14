macro(CheckFMA)
include (CheckCXXSourceRuns)

option(USE_FMA "enable or disable FMA" ON)

if(NOT USE_FMA)
  message(STATUS "skip test for FMA support")
else(NOT USE_FMA)

if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_ICPC OR COMPILER_IS_CLANG)

message(STATUS "test for FMA support:")

set(CMAKE_REQUIRED_FLAGS -mfma)
  CHECK_CXX_SOURCE_RUNS(
  "#include <immintrin.h>
  int main()
  {
    __m128d x;
    x = _mm_fmadd_pd(x,x,x);
    
    return 0;
  }" FMA_RUN)

  if(${FMA_RUN})
      set(HAVE_FMA 1)
  endif(${FMA_RUN})

  if(${HAVE_FMA})
    message(STATUS "\tFMA support... yes")
  else()
    message(STATUS "\tFMA support... no")
  endif()

  if(${HAVE_FMA})
      set(FMA_FLAGS "-mfma")
  endif()

else()
  message(STATUS "FMA detection is currently only supported for gcc, Clang and Intel C++ compiler. FMA disabled.")
endif()

endif()

endmacro()