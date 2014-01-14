macro(CheckSIMD)
include (CheckCXXSourceRuns)

option(USE_SIMD "enable or disable SIMD" ON)

if(NOT USE_SIMD)
  message(STATUS "skip test for SIMD support")
else(NOT USE_SIMD)

if(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_ICPC OR COMPILER_IS_CLANG)

  message(STATUS "test for SIMD support:")

  set(CMAKE_REQUIRED_FLAGS -msse)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  int main()
  {
    __m128 x;
    x = _mm_set_ps(1.0f,1.0f,1.0f,1.0f);
      
    return 0;
  }" SSE_RUN)

  if(${SSE_RUN})
      set(HAVE_SSE 1)
  endif(${SSE_RUN})

  if(${HAVE_SSE})
    message(STATUS "\tSSE support... yes")
  else()
    message(STATUS "\tSSE support... no")
  endif()


  set(CMAKE_REQUIRED_FLAGS -msse2)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  #include <emmintrin.h>
  int main()
  {
    __m128d x;
    x = _mm_set_pd(1.0,1.0);
      
    return 0;
  }" SSE2_RUN)

  if(${SSE2_RUN})
      set(HAVE_SSE2 1)
  endif(${SSE2_RUN})

  if(${HAVE_SSE2})
    message(STATUS "\tSSE2 support... yes")
  else()
    message(STATUS "\tSSE2 support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -msse3)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  #include <emmintrin.h>
  #include <pmmintrin.h>
  int main()
  {
    __m128d x;
    __m128d y;
    x = _mm_addsub_pd(x,y);
      
    return 0;
  }" SSE3_RUN)

  if(${SSE3_RUN})
      set(HAVE_SSE3 1)
  endif(${SSE3_RUN})

  if(${HAVE_SSE3})
    message(STATUS "\tSSE3 support... yes")
  else()
    message(STATUS "\tSSE3 support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -mssse3)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  #include <tmmintrin.h>
  int main()
  {
    __m128i x;
    x = _mm_abs_epi8(x);
      
    return 0;
  }" SSSE3_RUN)

  if(${SSSE3_RUN})
      set(HAVE_SSSE3 1)
  endif(${SSSE3_RUN})

  if(${HAVE_SSSE3})
    message(STATUS "\tSSSE3 support... yes")
  else()
    message(STATUS "\tSSSE3 support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -msse4)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  #include <emmintrin.h>
  #include <pmmintrin.h>
  #include <smmintrin.h>
  int main()
  {
    __m128d x;
    x = _mm_ceil_pd(x);
      
    return 0;
  }" SSE4_1_RUN)

  if(${SSE4_1_RUN})
      set(HAVE_SSE4_1 1)
  endif(${SSE4_1_RUN})

  if(${HAVE_SSE4_1})
    message(STATUS "\tSSE4.1 support... yes")
  else()
    message(STATUS "\tSSE4.1 support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -msse4)
  CHECK_CXX_SOURCE_RUNS(
  "#include <xmmintrin.h>
  #include <emmintrin.h>
  #include <pmmintrin.h>
  #include <smmintrin.h>
  int main()
  {
    unsigned int x = 0;
    x = _mm_crc32_u32(x,x);
      
    return 0;
  }" SSE4_2_RUN)

  if(${SSE4_2_RUN})
      set(HAVE_SSE4_2 1)
  endif(${SSE4_2_RUN})

  if(${HAVE_SSE4_2})
    message(STATUS "\tSSE4.2 support... yes")
  else()
    message(STATUS "\tSSE4.2 support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -mavx)
  CHECK_CXX_SOURCE_RUNS(
  "#include <immintrin.h>
  int main()
  {
    __m256d x;
    x = _mm256_add_pd(x,x);
      
    return 0;
  }" AVX_RUN)

  if(${AVX_RUN})
      set(HAVE_AVX 1)
  endif(${AVX_RUN})

  if(${HAVE_AVX})
    message(STATUS "\tAVX support... yes")
  else()
    message(STATUS "\tAVX support... no")
  endif()

  set(CMAKE_REQUIRED_FLAGS -mavx2)
  CHECK_CXX_SOURCE_RUNS(
  "#include <immintrin.h>
  int main()
  {
    __m256i x;
    x =  _mm256_add_epi32(x,x);
    
    return 0;
  }" AVX2_RUN)

  if(${AVX2_RUN})
      set(HAVE_AVX2 1)
  endif(${AVX2_RUN})

  if(${HAVE_AVX2})
    message(STATUS "\tAVX2 support... yes")
  else()
    message(STATUS "\tAVX2 support... no")
  endif()

  if(NOT USED_SIMD_UNIT)
    if(${HAVE_AVX2})
      set(SIMD_FLAGS "-mavx2")
    elseif(${HAVE_AVX})
      set(SIMD_FLAGS "-mavx")
    elseif(${HAVE_SSE4_2})
      set(SIMD_FLAGS "-msse4")
    elseif(${HAVE_SSE4_1})
      set(SIMD_FLAGS "-msse4")
    elseif(${HAVE_SSSE3})
      set(SIMD_FLAGS "-mssse3")
    elseif(${HAVE_SSE3})
      set(SIMD_FLAGS "-msse3")
    elseif(${HAVE_SSE2})
      set(SIMD_FLAGS "-msse2")
    elseif(${HAVE_SSE})
      set(SIMD_FLAGS "-msse")
    endif()
  else(NOT USED_SIMD_UNIT)
    if(${USED_SIMD_UNIT} MATCHES "AVX2")
       if(${HAVE_AVX2})
          set(SIMD_FLAGS "-mavx2")
       else(${HAVE_AVX2})
          message(SEND_ERROR "AVX2 support requested by user, but this feature is not supported.")
       endif(${HAVE_AVX2})
    elseif(${USED_SIMD_UNIT} MATCHES "AVX")
       if(${HAVE_AVX})
          set(SIMD_FLAGS "-mavx")

          set(HAVE_AVX2 0)
       else(${HAVE_AVX})
          message(SEND_ERROR "AVX support requested by user, but this feature is not supported.")
       endif(${HAVE_AVX})
    elseif(${USED_SIMD_UNIT} MATCHES "SSE4_2")
       if(${HAVE_SSE4_2})
          set(SIMD_FLAGS "-msse4")

          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
       else(${HAVE_SSE4_2})
          message(SEND_ERROR "SSE 4.2 support requested by user, but is this feature not supported.")
       endif(${HAVE_SSE4_2})
    elseif(${USED_SIMD_UNIT} MATCHES "SSE4_1")
       if(${HAVE_SSE4_1})
          set(SIMD_FLAGS "-msse4")
          
          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
          set(HAVE_SSE4_2 0)
       else(${HAVE_SSE4_1})
          message(SEND_ERROR "SSE 4.1 support requested by user, but is this feature not supported.")
       endif(${HAVE_SSE4_1})
    elseif(${USED_SIMD_UNIT} MATCHES "SSSE3")
       if(${HAVE_SSSE3})
          set(SIMD_FLAGS "-mssse3")
          
          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
          set(HAVE_SSE4_2 0)
          set(HAVE_SSE4_1 0)
       else(${HAVE_SSSE3})
          message(SEND_ERROR "SSSE 3 support requested by user, but is this feature not supported.")
       endif(${HAVE_SSSE3})
    elseif(${USED_SIMD_UNIT} MATCHES "SSE3")
       if(${HAVE_SSE3})
          set(SIMD_FLAGS "-msse3")
          
          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
          set(HAVE_SSE4_2 0)
          set(HAVE_SSE4_1 0)
          set(HAVE_SSSE3 0)
       else(${HAVE_SSE3})
          message(SEND_ERROR "SSE 3 support requested by user, but is this feature not supported.")
       endif(${HAVE_SSE3})
    elseif(${USED_SIMD_UNIT} MATCHES "SSE2")
       if(${HAVE_SSE2})
          set(SIMD_FLAGS "-msse2")
          
          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
          set(HAVE_SSE4_2 0)
          set(HAVE_SSE4_1 0)
          set(HAVE_SSSE3 0)
          set(HAVE_SSE3 0)
       else(${HAVE_SSE2})
          message(SEND_ERROR "SSE 2 support requested by user, but is this feature not supported.")
       endif(${HAVE_SSE2})
    elseif(${USED_SIMD_UNIT} MATCHES "SSE")
       if(${HAVE_SSE})
          set(SIMD_FLAGS "-msse")
          
          set(HAVE_AVX2 0)
          set(HAVE_AVX 0)
          set(HAVE_SSE4_2 0)
          set(HAVE_SSE4_1 0)
          set(HAVE_SSSE3 0)
          set(HAVE_SSE3 0)
          set(HAVE_SSE2 0)
       else(${HAVE_SSE})
          message(SEND_ERROR "SSE support requested by user, but is this feature not supported.")
       endif(${HAVE_SSE})
    endif(${USED_SIMD_UNIT} MATCHES "AVX")
  endif(NOT USED_SIMD_UNIT)

else(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_ICPC  OR COMPILER_IS_CLANG)
  message(STATUS "SIMD detection is currently only supported for gcc, Clang and Intel C++ compiler. SIMD disabled.")
endif(CMAKE_COMPILER_IS_GNUCXX OR COMPILER_IS_ICPC  OR COMPILER_IS_CLANG)

endif(NOT USE_SIMD)

endmacro(CheckSIMD) 
