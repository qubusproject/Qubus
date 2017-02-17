#ifndef QUBUS_UTIL_UNUSED_HPP
#define QUBUS_UTIL_UNUSED_HPP

#ifdef __GNUC__

#define QUBUS_UNUSED(x) x __attribute__((unused))

#ifdef NDEBUG

#define QUBUS_UNUSED_RELEASE(x) QUBUS_UNUSED(x)

#else

#define QUBUS_UNUSED_RELEASE(x) x

#endif

#else

#error "unused attribute not supported"

#endif

#endif
