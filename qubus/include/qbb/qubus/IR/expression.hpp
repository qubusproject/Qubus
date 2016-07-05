#ifndef QBB_QUBUS_EXPRESSION_HPP
#define QBB_QUBUS_EXPRESSION_HPP

#include <hpx/config.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/qubus/IR/annotations.hpp>

#include <qbb/qubus/IR/expression_traits.hpp>
#include <qbb/util/unused.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/runtime/serialization/base_object.hpp>


#include <memory>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <utility>
#include <type_traits>

namespace qbb
{
namespace qubus
{
class expression
{
public:
    expression() = default;

    template <typename T, typename Enabler = typename std::enable_if<is_expression<T>::value>::type>
    expression(T value)
    {
        auto tag = implementation_table_.register_type<T>();

        self_ = std::make_unique<expression_wrapper<T>>(std::move(value), tag);
    }

    expression(expression& other)
    : self_(other.self_ ? other.self_->clone() : nullptr)
    {
    }

    expression(const expression& other)
    : self_(other.self_ ? other.self_->clone() : nullptr)
    {
    }

    expression& operator=(const expression& other)
    {
        self_ = other.self_ ? other.self_->clone() : nullptr;

        return *this;
    }

    expression(expression&& other) = default;
    expression& operator=(expression&& other) = default;

    annotation_map& annotations() const
    {
        return self_->annotations();
    }

    annotation_map& annotations()
    {
        return self_->annotations();
    }

    std::vector<expression> sub_expressions() const
    {
        return self_->sub_expressions();
    }

    expression substitute_subexpressions(const std::vector<expression>& new_subexprs) const
    {
        return self_->substitute_subexpressions(new_subexprs);
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

    explicit operator bool() const
    {
        return static_cast<bool>(self_);
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

    template<typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar & self_;
    }
private:
    class expression_interface
    {
    public:
        virtual ~expression_interface() = default;

        virtual annotation_map& annotations() const = 0;
        virtual annotation_map& annotations() = 0;

        virtual std::vector<expression> sub_expressions() const = 0;
        virtual expression
        substitute_subexpressions(const std::vector<expression>& new_subexprs) const = 0;

        virtual std::type_index rtti() const = 0;
        virtual qbb::util::index_t tag() const = 0;

        virtual std::unique_ptr<expression_interface> clone() const = 0;

        template<typename Archive>
        void serialize(Archive& QBB_UNUSED(ar), unsigned QBB_UNUSED(version))
        {
        }

        HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(expression_interface);
    };

    template <typename T>
    class expression_wrapper : public expression_interface
    {
    public:
        expression_wrapper() = default;

        explicit expression_wrapper(T value_, qbb::util::index_t tag_) : value_(std::move(value_)), tag_(tag_)
        {
        }

        virtual ~expression_wrapper()
        {
        }

        annotation_map& annotations() const override final
        {
            return value_.annotations();
        }

        annotation_map& annotations() override final
        {
            return value_.annotations();
        }

        std::vector<expression> sub_expressions() const override final
        {
            return value_.sub_expressions();
        }

        expression
        substitute_subexpressions(const std::vector<expression>& new_subexprs) const override final
        {
            return value_.substitute_subexpressions(new_subexprs);
        }

        const T& get() const
        {
            return value_;
        }

        T& get()
        {
            return value_;
        }

        std::type_index rtti() const override final
        {
            return typeid(T);
        }

        qbb::util::index_t tag() const override final
        {
            return tag_;
        }

        std::unique_ptr<expression_interface> clone() const override final
        {
            return std::make_unique<expression_wrapper<T>>(value_, tag_);
        }

        template<typename Archive>
        void serialize(Archive& ar, unsigned QBB_UNUSED(version))
        {
            ar & hpx::serialization::base_object<expression_interface>(*this);

            ar & value_;
            ar & tag_;
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(expression_wrapper);
    private:
        T value_;
        qbb::util::index_t tag_;
    };

    std::unique_ptr<expression_interface> self_;

    static qbb::util::implementation_table implementation_table_;
};

bool operator==(const expression& lhs, const expression& rhs);
bool operator!=(const expression& lhs, const expression& rhs);

}
}

#endif