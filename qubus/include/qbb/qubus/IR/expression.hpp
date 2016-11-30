#ifndef QUBUS_EXPRESSION_HPP
#define QUBUS_EXPRESSION_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/annotations.hpp>

#include <qbb/util/multi_method.hpp>

#include <hpx/include/serialization.hpp>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/any_range.hpp>
#include <boost/range/irange.hpp>

#include <type_traits>
#include <utility>
#include <vector>

namespace qbb
{
namespace qubus
{

class expression;

class expression_cursor
{
public:
    expression_cursor() = default;

    expression_cursor(const expression& expr_);

    expression_cursor& move_up();
    expression_cursor& move_down(std::size_t index);

    std::size_t arity() const;

    explicit operator bool() const;

    const expression& operator*() const;
    const expression* operator->() const;

private:
    const expression* expr_ = nullptr;
};

bool operator==(const expression_cursor& lhs, const expression_cursor& rhs);
bool operator!=(const expression_cursor& lhs, const expression_cursor& rhs);

/** \brief Base class for all Qubus IR expression nodes.
 */
class expression
{
public:
    using cursor_type = expression_cursor;

    expression() = default;
    virtual ~expression() = default;

    expression(const expression& other) = delete;
    expression& operator=(const expression& other) = delete;

    expression(expression&& other) = delete;
    expression& operator=(expression&& other) = delete;

    cursor_type cursor() const
    {
        return cursor_type(*this);
    }

    virtual const expression* parent() const = 0;
    virtual void set_parent(expression& parent) = 0;

    virtual const expression& child(std::size_t index) const = 0;
    virtual std::size_t arity() const = 0;

    virtual boost::any_range<const expression&, boost::forward_traversal_tag>
    sub_expressions() const = 0;

    virtual std::unique_ptr<expression>
    substitute_subexpressions(std::vector<std::unique_ptr<expression>> new_children) const = 0;

    virtual expression* clone() const = 0;

    virtual annotation_map& annotations() const = 0;

    template <typename T>
    const T& as() const
    {
        using value_type = typename std::decay<T>::type;

        return dynamic_cast<const value_type&>(*this);
    }

    template <typename T>
    const T* try_as() const
    {
        using value_type = typename std::decay<T>::type;

        if (auto casted_ptr = dynamic_cast<const value_type*>(this))
        {
            return casted_ptr;
        }
        else
        {
            return nullptr;
        }
    }

    static const qbb::util::implementation_table& get_implementation_table()
    {
        return implementation_table_;
    }

    static std::size_t number_of_implementations()
    {
        return implementation_table_.number_of_implementations();
    }

    virtual qbb::util::index_t type_tag() const = 0;

    template <typename Archive>
    void serialize(Archive& QBB_UNUSED(ar), unsigned QBB_UNUSED(version))
    {
    }

    HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(expression);

protected:
    static qbb::util::implementation_table implementation_table_;
};

bool operator==(const expression& lhs, const expression& rhs);
bool operator!=(const expression& lhs, const expression& rhs);

std::unique_ptr<expression> clone(const expression& expr);

template <typename Expression, typename Enabled = typename std::enable_if<std::is_base_of<expression, Expression>::value>::type>
std::unique_ptr<Expression> clone(const Expression& expr)
{
    return std::unique_ptr<Expression>(static_cast<Expression*>(expr.clone()));
}

std::vector<std::unique_ptr<expression>>
clone(const std::vector<std::unique_ptr<expression>>& expressions);
std::vector<std::unique_ptr<expression>>
clone(const std::vector<std::reference_wrapper<expression>>& expressions);

template <typename Expression, typename ExpressionBase = expression>
class expression_base : public ExpressionBase
{
public:
    static_assert(std::is_base_of<expression, ExpressionBase>::value,
                  "expression needs to be a base class of ExpressionBase.");

    expression_base()
    {
        tag_ = this->implementation_table_.template register_type<Expression>();
    }

    virtual ~expression_base() = default;

    const expression* parent() const override final
    {
        return parent_;
    }

    void set_parent(expression& parent) override final
    {
        parent_ = &parent;
    }

    boost::any_range<const expression&, boost::forward_traversal_tag>
    sub_expressions() const override final
    {
        return boost::irange<std::size_t>(0, this->arity()) |
               boost::adaptors::transformed(
                   [this](std::size_t index) -> decltype(auto) { return this->child(index); });
    }

    qbb::util::index_t type_tag() const override final
    {
        return tag_;
    }

    annotation_map& annotations() const override final
    {
        return annotations_;
    }

protected:
    template <typename ChildExpression>
    std::unique_ptr<ChildExpression>&& take_over_child(std::unique_ptr<ChildExpression>& child)
    {
        child->set_parent(*this);

        return std::move(child);
    }

    template <typename ChildExpression>
    std::unique_ptr<ChildExpression>&& take_over_child(std::unique_ptr<ChildExpression>&& child)
    {
        child->set_parent(*this);

        return std::move(child);
    }

    template <typename ChildExpression>
    std::vector<std::unique_ptr<ChildExpression>>&&
    take_over_children(std::vector<std::unique_ptr<ChildExpression>>& children)
    {
        for (const auto& child : children)
        {
            child->set_parent(*this);
        }

        return std::move(children);
    }

private:
    qbb::util::index_t tag_;
    expression* parent_ = nullptr;

    mutable annotation_map annotations_;
};

template <typename Expression>
std::unique_ptr<Expression> clone(const expression_base<Expression>& expr)
{
    return std::unique_ptr<Expression>(static_cast<const Expression&>(expr).clone());
}

extern qbb::util::multi_method<bool(const qbb::util::virtual_<expression>&,
                                    const qbb::util::virtual_<expression>&)>
    equal;
}
}

#endif
