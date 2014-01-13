#ifndef QBB_UNREACHABLE_HPP
#define QBB_UNREACHABLE_HPP

#ifdef __GNUC__

#define QBB_UNREACHABLE() __builtin_unreachable()

#else

#error "unreachable builtin not supported"

#endif

#endif