#ifndef QBB_UTIL_HPX_SERIALIZATION_CHRONO_HPP
#define QBB_UTIL_HPX_SERIALIZATION_CHRONO_HPP

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/input_archive.hpp>
#include <hpx/runtime/serialization/output_archive.hpp>

#include <chrono>

namespace hpx
{
namespace serialization
{

template <typename Rep, typename Period>
void load(input_archive& ar, std::chrono::duration<Rep, Period>& value, unsigned)
{
    Rep count;

    ar >> count;

    value = std::chrono::duration<Rep, Period>(count);
}

template <typename Rep, typename Period>
void save(output_archive& ar, const std::chrono::duration<Rep, Period>& value, unsigned)
{
    ar << value.count();
}

HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE((template <typename Rep, typename Period>), (std::chrono::duration<Rep, Period>));

}
}

#endif
