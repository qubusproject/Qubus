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
    tensor_info(object_client data_)
            : data_(std::move(data_))
    {
    }

    hpx::shared_future<void> when_ready() const
    {
        return data_.get_last_modification();
    }

    object_client get_object() const
    {
        return data_;
    }
private:
    object_client data_;
};
}
}

#endif //QBB_TENSOR_INFO_HPP
