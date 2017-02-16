#ifndef QUBUS_UTIL_UNREACHABLE_HPP
#define QUBUS_UTIL_UNREACHABLE_HPP

#include <qubus/util/assert.hpp>

#ifdef __GNUC__

#define QUBUS_UNREACHABLE()                                                                        \
    do                                                                                             \
    {                                                                                              \
        QUBUS_ASSERT(false, "Reached unreachable location.");                                      \
        __builtin_unreachable();                                                                   \
    } while (false)

#define QUBUS_UNREACHABLE_BECAUSE(MESSAGE)                                                         \
    do                                                                                             \
    {                                                                                              \
        QUBUS_ASSERT(false, "Reached unreachable location: " MESSAGE);                             \
        __builtin_unreachable();                                                                   \
    } while (false)
#else

#warning "unreachable builtin not supported"

#endif

#endif