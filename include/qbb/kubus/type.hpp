#ifndef QBB_KUBUS_TYPE_HPP
#define QBB_KUBUS_TYPE_HPP

#include <qbb/util/multi_method.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>

namespace qbb
{
namespace kubus
{
    
class type
{
public:
    template <typename T>
    type(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<type_wrapper<T>>(value, tag);
    }
    
    bool is_primitive() const
    {
        return self_->is_primitive();
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
            return static_cast<type_wrapper<value_type>*>(self_.get())->get();
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
    class type_interface
    {
    public:
        virtual ~type_interface() = default;
        
        virtual bool is_primitive() const = 0;

        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;
    };

    template <typename T>
    class type_wrapper final : public type_interface
    {
    public:
        explicit type_wrapper(T value_, qbb::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~type_wrapper()
        {
        }

        const T& get() const
        {
            return value_;
        }
        
        bool is_primitive() const override
        {
            return value_.is_primitive();
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

    std::shared_ptr<type_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};

namespace types
{

class double_
{
public:
    bool is_primitive() const
    {
        return true;
    }
};

class tensor
{
public:
    explicit tensor(type value_type_)
    : value_type_{std::move(value_type_)}
    {
    }
    
    const type& value_type() const
    {
        return value_type_;
    }
    
    bool is_primitive() const
    {
        return false;
    }
private:
    type value_type_;
};

class sparse_tensor
{
public:
    explicit sparse_tensor(type value_type_)
    : value_type_{std::move(value_type_)}
    {
    }
    
    const type& value_type() const
    {
        return value_type_;
    }
    
    bool is_primitive() const
    {
        return false;
    }
private:
    type value_type_;
};

}

}
}
#endif
