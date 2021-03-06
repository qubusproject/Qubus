#ifndef QUBUS_UTIL_NOINLINE_HPP
#define QUBUS_UTIL_NOINLINE_HPP

#ifdef __GNUC__

#define QUBUS_NOINLINE(...)  __VA_ARGS__ __attribute__((noinline))

#else

#warning "noinline attribute not supported"

#endif

#endif
