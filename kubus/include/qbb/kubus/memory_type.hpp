#ifndef QBB_KUBUS_MEMORY_TYPE_HPP
#define QBB_KUBUS_MEMORY_TYPE_HPP

#include <qbb/util/multi_method.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>

namespace qbb
{
namespace kubus
{
    
class memory_type
{
public:
    template <typename T>
    memory_type(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<memory_type_wrapper<T>>(value, tag);
    }
    
    std::type_index rtti() const
    {
        return self_->rtti();
    }

    template <typename T>
    T as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<memory_type_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }
    
    qbb::util::index_t tag() const
    {
        return self_->tag();
    }

    static const qbb::util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

private:
    class memory_type_interface
    {
    public:
        virtual ~memory_type_interface() = default;

        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;
    };

    template <typename T>
    class memory_type_wrapper final : public memory_type_interface
    {
    public:
        explicit memory_type_wrapper(T value_, qbb::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~memory_type_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }

        std::type_index rtti() const override
        {
            return typeid(T);
        }

        qbb::util::index_t tag() const override
        {
            return tag_;
        }

    private:
        T value_;
        qbb::util::index_t tag_;
    };

    std::shared_ptr<memory_type_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};

struct cpu_memory
{
};

struct disk_memory
{
};

struct gpu_memory
{
};

}
}

#endif