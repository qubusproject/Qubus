//
// Created by christopher on 04.04.16.
//

#ifndef QBB_QUBUS_TENSOR_INFO_HPP
#define QBB_QUBUS_TENSOR_INFO_HPP

#include <qbb/qubus/object.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
template <typename T, long int Rank>
class tensor_info
{
public:
    tensor_info(object data_) : data_(std::move(data_))
    {
    }

    hpx::future<void> when_ready()
    {
        // Wait on all previous write operations by scheduling a read.
        // We can safely discard the obtained access token since we will not access the object.

        return data_.acquire_read_access();
    }

    object get_object() const
    {
        return data_;
    }

private:
    object data_;
};
}
}

#endif
