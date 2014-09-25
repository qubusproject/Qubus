#ifndef QBB_KUBUS_EXPRESSION_HPP
#define QBB_KUBUS_EXPRESSION_HPP

#include <qbb/util/multi_method.hpp>
#include <qbb/kubus/IR/annotations.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <utility>

namespace qbb
{
namespace kubus
{
class expression
{
public:
    template <typename T>
    expression(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_shared<expression_wrapper<T>>(value, tag);
    }

    expression(const expression&) = default;
    expression(expression&) = default;

    annotation_map& annotations() const
    {
        return self_->annotations();
    }
    
    annotation_map& annotations()
    {
        return self_->annotations();
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
            return static_cast<expression_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            throw std::bad_cast();
        }
    }

    template <typename T>
    const T* try_as() const
    {
        using value_type = typename std::decay<T>::type;

        if (self_->rtti() == typeid(value_type))
        {
            return &static_cast<expression_wrapper<value_type>*>(self_.get())->get();
        }
        else
        {
            return nullptr;
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
    class expression_interface
    {
    public:
        virtual ~expression_interface() = default;   
        
        virtual annotation_map& annotations() const = 0;
        virtual annotation_map& annotations() = 0;
        
        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;
    };

    template <typename T>
    class expression_wrapper final : public expression_interface
    {
    public:
        explicit expression_wrapper(T value_, qbb::util::index_t tag_) : value_(value_), tag_(tag_)
        {
        }

        virtual ~expression_wrapper()
        {
        }

        annotation_map& annotations() const override
        {
            return value_.annotations();
        }
        
        annotation_map& annotations() override
        {
            return value_.annotations();
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

        qbb::util::index_t tag() const override
        {
            return tag_;
        }

    private:
        T value_;
        qbb::util::index_t tag_;
    };

    std::shared_ptr<expression_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};
}
}

#endif