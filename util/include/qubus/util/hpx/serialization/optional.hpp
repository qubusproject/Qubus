#ifndef QUBUS_UTIL_HPX_SERIALIZATION_OPTIONAL_HPP
#define QUBUS_UTIL_HPX_SERIALIZATION_OPTIONAL_HPP

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/input_archive.hpp>
#include <hpx/runtime/serialization/output_archive.hpp>

#include <boost/optional.hpp>

namespace hpx
{
namespace serialization
{
template <typename T>
void load(input_archive& ar, boost::optional<T>& value, unsigned)
{
    bool initialized;

    ar >> initialized;

    if (initialized)
    {
        T aux;

        ar >> aux;
        value.reset(aux);
    }
    else
    {
        value.reset();
    }
}

template <typename T>
void save(output_archive& ar, const boost::optional<T>& value, unsigned)
{
    bool initialized = value.is_initialized();

    ar << initialized;

    if (initialized)
    {
        ar << *value;
    }
}

HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE((template <class T>), (boost::optional<T>));
}
}

#endif
