#ifndef QBB_QUBUS_EXECUTION_STACK_HPP
#define QBB_QUBUS_EXECUTION_STACK_HPP

#include <qbb/util/align.hpp>
#include <qbb/util/make_unique.hpp>

#include <cstddef>
#include <vector>
#include <memory>
#include <type_traits>
#include <new>
#include <utility>

namespace qbb
{
namespace qubus
{

class stack_buffer
{
public:
    stack_buffer() = default;
    virtual ~stack_buffer() = default;

    stack_buffer(const stack_buffer&) = delete;
    stack_buffer& operator=(const stack_buffer&) = delete;
    
    virtual void* ptr() = 0;
    virtual std::size_t size() const = 0;
    virtual void commit() = 0;
    virtual void clear() = 0;
};

class cpu_stack_buffer : public stack_buffer
{
public:
    cpu_stack_buffer(std::size_t size_) : buffer_(size_)
    {
    }

    virtual ~cpu_stack_buffer() = default;

    void* ptr() override
    {
        return buffer_.data();
    }

    std::size_t size() const override
    {
        return buffer_.size();
    }

    void commit() override
    {
    }

    void clear() override
    {
    }
private:
    std::vector<char> buffer_;
};

class execution_stack
{
public:
    explicit execution_stack(std::size_t size_)
    : buffer_(util::make_unique<cpu_stack_buffer>(size_)), tip_(buffer_->ptr()),
      remaining_size_(buffer_->size())
    {
    }

    execution_stack(const execution_stack&) = delete;
    execution_stack& operator=(const execution_stack&) = delete;

    template <typename T>
    T* push(const T& value)
    {
        return emplace<T>(value);
    }

    template <typename T, typename... Args>
    T* emplace(Args&&... args)
    {
        static_assert(std::is_trivially_destructible<T>::value,
                      "Objects on the execution stack must be trivially destructible.");

        void* addr = util::align(alignof(T), sizeof(T), tip_, remaining_size_);

        // TODO: Don't use bad_alloc for this error.
        if (!addr)
            throw std::bad_alloc();

        T* typed_addr = new (addr) T{std::forward<Args>(args)...};

        tip_ = static_cast<char*>(tip_) + sizeof(T);
        remaining_size_ -= sizeof(T);

        return typed_addr;
    }

    void commit()
    {
        buffer_->commit();
    }

    void clear()
    {
        buffer_->clear();
        
        tip_ = buffer_->ptr();
        remaining_size_ = buffer_->size();
    }

private:
    std::unique_ptr<stack_buffer> buffer_;
    void* tip_;
    std::size_t remaining_size_;
};
}
}

#endif