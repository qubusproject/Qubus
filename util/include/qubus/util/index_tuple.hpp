#ifndef QUBUS_UTIL_INDEX_TUPLE_HPP
#define QUBUS_UTIL_INDEX_TUPLE_HPP

#include <boost/container/small_vector.hpp>

namespace qubus
{
namespace util
{

template <typename T>
using index_tuple = boost::small_vector<T, 10>;

}
}

#endif
