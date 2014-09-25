#ifndef QBB_KUBUS_ALLOCATOR_HPP
#define QBB_KUBUS_ALLOCATOR_HPP

#include <qbb/kubus/memory_block.hpp>
#include <qbb/util/make_unique.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>

namespace qbb
{
namespace kubus
{

class allocator
{
public:
    template <typename T>
    allocator(T value)
    {
        self_ = std::make_shared<allocator_wrapper<T>>(std::move(value));
    }

    allocator(allocator&) = default;
    allocator(const allocator&) = default;

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    const T& as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<allocator_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    template <typename T>
    T& as()
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<allocator_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    memory_block allocate(std::size_t size)
    {
        return self_->allocate(size);
    }

    void deallocate(memory_block& mem_block)
    {
        self_->deallocate(mem_block);
    }

private:
    class allocator_interface
    {
    public:
        virtual ~allocator_interface() = default;

        virtual std::type_index rtti() const = 0;

        virtual memory_block allocate(std::size_t size) = 0;

        virtual void deallocate(memory_block& mem_block) = 0;
    };

    template <typename T>
    class allocator_wrapper final : public allocator_interface
    {
    public:
        explicit allocator_wrapper(T value_) : value_(std::move(value_))
        {
        }

        virtual ~allocator_wrapper()
        {
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

        memory_block allocate(std::size_t size) override
        {
            return value_.allocate(size);
        }

        void deallocate(memory_block& mem_block) override
        {
            value_.deallocate(mem_block);
        }

    private:
        T value_;
    };

    std::shared_ptr<allocator_interface> self_;
};

class allocator_view
{
public:
    template <typename T>
    allocator_view(T& value)
    {
        self_ = qbb::util::make_unique<allocator_view_wrapper<T>>(value);
    }

    allocator_view(allocator_view& other) : self_{other.self_->clone()}
    {
    }

    allocator_view(const allocator_view& other) : self_{other.self_->clone()}
    {
    }

    allocator_view& operator=(const allocator_view& other)
    {
        self_ = other.self_->clone();

        return *this;
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    const T& as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<allocator_view_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    template <typename T>
    T& as()
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<allocator_view_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    memory_block allocate(std::size_t size)
    {
        return self_->allocate(size);
    }

    void deallocate(memory_block& mem_block)
    {
        self_->deallocate(mem_block);
    }

private:
    class allocator_view_interface
    {
    public:
        virtual ~allocator_view_interface() = default;

        virtual std::type_index rtti() const = 0;

        virtual std::unique_ptr<allocator_view_interface> clone() const = 0;

        virtual memory_block allocate(std::size_t size) = 0;

        virtual void deallocate(memory_block& mem_block) = 0;
    };

    template <typename T>
    class allocator_view_wrapper final : public allocator_view_interface
    {
    public:
        explicit allocator_view_wrapper(T& value_) : value_(&value_)
        {
        }

        virtual ~allocator_view_wrapper()
        {
        }

        const T& get() const
        {
            return *value_;
        }

        T& get()
        {
            return *value_;
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        std::unique_ptr<allocator_view_interface> clone() const override
        {
            return qbb::util::make_unique<allocator_view_wrapper<T>>(*value_);
        }

        memory_block allocate(std::size_t size) override
        {
            return value_->allocate(size);
        }

        void deallocate(memory_block& mem_block) override
        {
            value_->deallocate(mem_block);
        }

    private:
        T* value_;
    };

    std::unique_ptr<allocator_view_interface> self_;
};
}
}

#endif