#ifndef KUBUS_MEMORY_BLOCK_HPP
#define KUBUS_MEMORY_BLOCK_HPP

#include <qbb/util/make_unique.hpp>

#include <memory>
#include <typeindex>

class memory_block
{
public:
    memory_block() = default;

    template <typename T>
    memory_block(T value)
    {
        self_ = qbb::util::make_unique<memory_block_wrapper<T>>(std::move(value));
    }

    memory_block(memory_block& other) : self_{other.self_ ? other.self_->clone() : nullptr}
    {
    }

    memory_block(const memory_block& other) : self_{other.self_ ? other.self_->clone() : nullptr}
    {
    }

    memory_block& operator=(const memory_block& other)
    {
        self_ = other.self_ ? other.self_->clone() : nullptr;

        return *this;
    }

    std::type_index rtti() const
    {
        // FIXME: what should we do here if self_ == nullptr ?

        return self_->rtti();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (!self_)
            return T{};

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<memory_block_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    std::size_t size() const
    {
        return self_ ? self_->size() : 0;
    }

    explicit operator bool() const
    {
        return self_ && self_->is_valid();
    }

private:
    class memory_block_interface
    {
    public:
        virtual ~memory_block_interface() = default;

        virtual std::type_index rtti() const = 0;

        virtual std::unique_ptr<memory_block_interface> clone() const = 0;

        virtual std::size_t size() const = 0;

        virtual bool is_valid() const = 0;
    };

    template <typename T>
    class memory_block_wrapper final : public memory_block_interface
    {
    public:
        explicit memory_block_wrapper(T value_) : value_(std::move(value_))
        {
        }

        virtual ~memory_block_wrapper()
        {
        }

        std::unique_ptr<memory_block_interface> clone() const override
        {
            return qbb::util::make_unique<memory_block_wrapper<T>>(value_);
        }

        const T& get() const
        {
            return value_;
        }

        T& get()
        {
            return value_;
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        std::size_t size() const override
        {
            return value_.size();
        }

        bool is_valid() const override
        {
            return !!value_;
        }

    private:
        T value_;
    };

    std::unique_ptr<memory_block_interface> self_;
};

#endif