#ifndef KUBUS_MEMORY_BLOCK_HPP
#define KUBUS_MEMORY_BLOCK_HPP

#include <cstddef>

namespace qbb
{
namespace kubus
{

class memory_block
{
public:
    memory_block() = default;
    virtual ~memory_block() = default;

    memory_block(const memory_block&) = delete;
    memory_block& operator=(const memory_block&) = delete;

    virtual std::size_t size() const = 0;

    virtual void* ptr() const = 0;
};
}
}

#endif