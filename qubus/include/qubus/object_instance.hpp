#ifndef QUBUS_OBJECT_INSTANCE_HPP
#define QUBUS_OBJECT_INSTANCE_HPP

#include <qubus/IR/type.hpp>
#include <qubus/architecture_identifier.hpp>
#include <qubus/host_memory.hpp>

#include <hpx/include/serialization.hpp>

#include <qubus/util/unused.hpp>

#include <memory>
#include <utility>

namespace qubus
{

template <typename MemoryType>
class object_instance
{
public:
    using memory_block_t = typename MemoryType::memory_block_type;

    object_instance(type datatype_, memory_block_t data_)
    : datatype_(std::move(datatype_)), data_(std::move(data_))
    {
    }

    object_instance(const object_instance& other) = delete;
    object_instance& operator=(const object_instance& other) = delete;

    object_instance(object_instance&& other) noexcept = default;
    object_instance& operator=(object_instance&& other) noexcept = default;

    type object_type() const
    {
        return datatype_;
    }

    memory_block_t& data()
    {
        return data_;
    }

    const memory_block_t& data() const
    {
        return data_;
    }

private:
    type datatype_;
    memory_block_t data_;
};

template <typename MemoryType>
object_instance<MemoryType> copy(const object_instance<MemoryType>& other)
{
    return object_instance<MemoryType>(other.object_type(), copy(other.data()));
}

template <typename Archive, typename MemoryType>
void save(Archive& ar, const object_instance<MemoryType>& value, unsigned int QUBUS_UNUSED(version))
{
    ar << value.object_type();

    std::size_t data_size = value.data().size();

    ar << data_size;

    ar << value.data().alignment();

    ar << hpx::serialization::make_array(value.data().ptr(), data_size);
}

template <typename Archive, typename MemoryType>
void load(Archive& ar, object_instance<MemoryType>& value, unsigned int QUBUS_UNUSED(version))
{
    type datatype;

    ar >> datatype;

    std::size_t data_size;

    ar >> data_size;

    std::size_t alignment;

    ar >> alignment;

    typename object_instance<MemoryType>::memory_block_t data(data_size, alignment);

    ar >> hpx::serialization::make_array(data.ptr(), data_size);

    value = object_instance<MemoryType>(std::move(datatype), std::move(data));
}

template <typename Archive, typename MemoryType>
void save_construct_data(Archive& ar, const object_instance<MemoryType>* value,
                         unsigned int QUBUS_UNUSED(version))
{
    ar << value->object_type();

    std::size_t data_size = value->data().size();

    ar << data_size;

    ar << value->data().alignment();

    ar << hpx::serialization::make_array(value->data().ptr(), data_size);
}

template <typename Archive, typename MemoryType>
void load_construct_data(Archive& ar, object_instance<MemoryType>* value,
                         unsigned int QUBUS_UNUSED(version))
{
    type datatype;

    ar >> datatype;

    std::size_t data_size;

    ar >> data_size;

    std::size_t alignment;

    ar >> alignment;

    typename object_instance<MemoryType>::memory_block_t data(data_size, alignment);

    ar >> hpx::serialization::make_array(data.ptr(), data_size);

    ::new (value) object_instance<MemoryType>(std::move(datatype), std::move(data));
}

HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE((template <typename MemoryType>),
                                      (object_instance<MemoryType>));

extern template class object_instance<host_allocator>;

using host_object_instance = object_instance<host_allocator>;

} // namespace qubus

#endif
