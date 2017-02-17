#ifndef QUBUS_UTIL_BENCHMARK_HPP
#define QUBUS_UTIL_BENCHMARK_HPP

namespace qubus
{
namespace util
{

// Implementations of Chandler Carruth's benchmark utility functions as seen during his CppCon 2015 talk
// "Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!".

#ifdef __GNUC__

inline void escape(void* p)
{
    asm volatile("" : : "g"(p) : "memory");
}

#else
#error "escape is not implemented."
#endif

#ifdef __GNUC__

inline void clobber()
{
    asm volatile("" : : : "memory");
}

#else
#error "clobber is not implemented."
#endif

}
}

#endif
