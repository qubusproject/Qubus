#ifndef QUBUS_ADDRESS_ALLOCATOR_HPP
#define QUBUS_ADDRESS_ALLOCATOR_HPP

#include <qubus/object_id.hpp>

namespace qubus
{

class address_allocator
{
public:
    address_allocator() = default;
    virtual ~address_allocator() = default;

    virtual object_id acquire_new_id() = 0;

    address_allocator(const address_allocator&) = delete;
    address_allocator(address_allocator&&) = delete;

    address_allocator& operator=(const address_allocator&) = delete;
    address_allocator& operator=(address_allocator&&) = delete;
};

} // namespace qubus

#endif
