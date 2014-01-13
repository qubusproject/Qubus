#ifndef QBB_UTILITY_INTEGERS_HPP
#define QBB_UTILITY_INTEGERS_HPP

#include <boost/numeric/conversion/cast.hpp>

#include <cstddef>
#include <cassert>
#include <type_traits>
#include <limits>

#include <QBB/utility/assert.hpp>
#include <QBB/utility/unreachable.hpp>

namespace qbb
{

using index_t = std::ptrdiff_t;

template <typename Destination, typename Source>
inline Destination integer_cast(const Source& source)
{
    static_assert(
        std::is_integral<Destination>::value,
        "integer_cast: the destination type must be an integral type");
    static_assert(std::is_integral<Source>::value,
                  "integer_cast: the source type must be an integral type");

    #ifndef NDEBUG
    try
    {
        return boost::numeric_cast<Destination>(source);
    }
    catch(const boost::bad_numeric_cast&)
    {
        QBB_ASSERT(true,"bad integer cast");
        QBB_UNREACHABLE();
    }
    #else
    return static_cast<Destination>(source);
    #endif
}

template <typename Source>
inline index_t to_index(const Source source)
{ return integer_cast<index_t>(source); }

template <typename Source>
inline index_t to_uindex(const Source source)
{
    assert(source >= 0);

    return integer_cast<index_t>(source);
}

}

#endif