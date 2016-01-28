#ifndef QBB_UTIL_HPX_SERIALIZATION_SHARED_PTR_HPP
#define QBB_UTIL_HPX_SERIALIZATION_SHARED_PTR_HPP

#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/input_archive.hpp>
#include <hpx/runtime/serialization/output_archive.hpp>

#include <memory>

namespace hpx
{
namespace serialization
{
template <typename T>
void load(input_archive& ar, std::shared_ptr<T>& ptr, unsigned)
{
    detail::serialize_pointer_tracked(ar, ptr);
}

template <typename T>
void save(output_archive& ar, const std::shared_ptr<T>& ptr, unsigned)
{
    detail::serialize_pointer_tracked(ar, ptr);
}

HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE((template <class T>), (std::shared_ptr<T>));
}
}

#endif
