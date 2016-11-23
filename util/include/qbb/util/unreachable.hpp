#ifndef QBB_UTIL_UNREACHABLE_HPP
#define QBB_UTIL_UNREACHABLE_HPP

#include <qbb/util/assert.hpp>

#ifdef __GNUC__

#define QBB_UNREACHABLE()                                                                          \
    do                                                                                             \
    {                                                                                              \
        QBB_ASSERT(false, "Reached unreachable location.");                                        \
        __builtin_unreachable();                                                                   \
    } while (false)

#define QBB_UNREACHABLE_BECAUSE(MESSAGE)                                                           \
    do                                                                                             \
    {                                                                                              \
        QBB_ASSERT(false, "Reached unreachable location: " MESSAGE);                               \
        __builtin_unreachable();                                                                   \
    } while (false)
#else

#warning "unreachable builtin not supported"

#endif

#endif