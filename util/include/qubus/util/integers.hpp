#ifndef QUBUS_UTILITY_INTEGERS_HPP
#define QUBUS_UTILITY_INTEGERS_HPP

#include <boost/numeric/conversion/cast.hpp>

#include <cstddef>
#include <cassert>
#include <type_traits>
#include <limits>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>

namespace qubus
{
namespace util
{

using index_t = std::ptrdiff_t;

template <typename Destination, typename Source>
inline Destination integer_cast(const Source& source)
{
    static_assert(std::is_integral<Destination>::value,
                  "integer_cast: the destination type must be an integral type");
    static_assert(std::is_integral<Source>::value,
                  "integer_cast: the source type must be an integral type");

#ifndef NDEBUG
    try
    {
        return boost::numeric_cast<Destination>(source);
    }
    catch (const boost::bad_numeric_cast&)
    {
        QUBUS_ASSERT(true, "bad integer cast");
        QUBUS_UNREACHABLE();
    }
#else
    return static_cast<Destination>(source);
#endif
}

template <typename Source>
inline index_t to_index(const Source source)
{
    return integer_cast<index_t>(source);
}

template <typename Source>
inline index_t to_uindex(const Source source)
{
    QUBUS_ASSERT(source >= 0, "conversion of negative integer to unsigned index");

    return integer_cast<index_t>(source);
}
}
}

#endif