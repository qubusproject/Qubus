#ifndef KUBUS_MEMORY_BLOCK_HPP
#define KUBUS_MEMORY_BLOCK_HPP

#include <memory>
#include <exception>
#include <utility>

class wrong_memory_type_exception : public std::exception
{
public:
    wrong_memory_type_exception()
    : what_("cast to the wrong memory type")
    {
    }

    const char* what() const noexcept override
    {
        return what_.c_str();
    }
private:
    std::string what_;
};

class memory_block
{
public:
    virtual ~memory_block() = default;
    
    virtual std::size_t size() const = 0;
};

#endif