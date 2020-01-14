#ifndef QUBUS_INSTANCE_HANDLE_HPP
#define QUBUS_INSTANCE_HANDLE_HPP

#include <qubus/IR/type.hpp>
#include <qubus/object_instance.hpp>

#include <qubus/util/assert.hpp>

#include <memory>
#include <utility>

namespace qubus
{

template <typename MemoryType>
class instance_handle
{
public:
    using memory_block_t = typename object_instance<MemoryType>::memory_block_t;

    instance_handle() = default;

    explicit instance_handle(std::shared_ptr<object_instance<MemoryType>> entry_) : entry_(std::move(entry_))
    {
    }

    object_instance<MemoryType>& get_instance() const
    {
        QUBUS_ASSERT(entry_ != nullptr, "Invalid handle.");

        return *entry_;
    }

    memory_block_t& data() const
    {
        return entry_->data();
    }

    explicit operator bool() const
    {
        return static_cast<bool>(entry_);
    }

    bool unique() const
    {
        return entry_.unique();
    }

    void free()
    {
        entry_.reset();
    }

private:
    std::shared_ptr<object_instance<MemoryType>> entry_;
};

} // namespace qubus

#endif
