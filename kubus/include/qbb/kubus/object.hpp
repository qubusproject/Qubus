#ifndef QBB_KUBUS_OBJECT_HPP
#define QBB_KUBUS_OBJECT_HPP

#include <qbb/kubus/generic_ptr.hpp>
#include <qbb/util/multi_method.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>

namespace qbb
{
namespace kubus
{

class object
{
public:
    template <typename T>
    object(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<object_wrapper<T>>(std::move(value), tag);
    }

    std::type_index rtti() const
    {
        return self_->rtti();
    }

    std::size_t size() const
    {
        return self_->size();
    }
    
    qbb::kubus::generic_ptr ptr() const
    {
        return self_->ptr();
    }
    
    bool is_pinned() const
    {
        return !self_.unique();
    }
    
    template <typename T>
    const T& as() const &
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<object_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }
    
    template <typename T>
    T& as() &
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<object_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    template <typename T>
    T as() &&
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return static_cast<object_wrapper<value_type>*>(self_.get())->get();
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
    class object_interface
    {
    public:
        virtual ~object_interface() = default;

        virtual std::size_t size() const = 0;
        virtual qbb::kubus::generic_ptr ptr() const = 0;

        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;
    };

    template <typename T>
    class object_wrapper final : public object_interface
    {
    public:
        explicit object_wrapper(T value_, qbb::util::index_t tag_) : value_(std::move(value_)), tag_(tag_)
        {
        }

        virtual ~object_wrapper()
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
        
        std::size_t size() const override
        {
            return value_.size();
        }

        qbb::kubus::generic_ptr ptr() const override
        {
            return value_.ptr();
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

    std::shared_ptr<object_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};
}
}
#endif