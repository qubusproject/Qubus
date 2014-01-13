#ifndef QBB_MACROS_HPP
#define QBB_MACROS_HPP

#ifdef __GNUC__

#define QBB_UNUSED(x) x __attribute__((unused))

#ifdef NDEBUG

#define QBB_UNUSED_RELEASE(x) QBB_UNUSED(x)

#else

#define QBB_UNUSED_RELEASE(x) x

#endif

#else

#error "unused attribute not supported"

#endif

#endif
