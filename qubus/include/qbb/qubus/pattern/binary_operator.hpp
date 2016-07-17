#ifndef QBB_QUBUS_PATTERN_BINARY_OPERATOR_HPP
#define QBB_QUBUS_PATTERN_BINARY_OPERATOR_HPP

#include <qbb/qubus/IR/binary_operator_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/pattern/value.hpp>

#include <utility>
#include <functional>

namespace qbb
{
namespace qubus
{
namespace pattern
{
template <typename Tag, typename LHS, typename RHS>
class binary_operator_pattern
{
public:
    binary_operator_pattern(Tag tag_, LHS lhs_, RHS rhs_)
    :tag_(std::move(tag_)), lhs_(std::move(lhs_)), rhs_(std::move(rhs_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const binary_operator_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<binary_operator_expr>())
        {
            if (tag_.match(concret_value->tag()))
            {
                if (lhs_.match(concret_value->left()) && rhs_.match(concret_value->right()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    void reset() const
    {
        tag_.reset();
        lhs_.reset();
        rhs_.reset();
    }
private:
    Tag tag_;
    LHS lhs_;
    RHS rhs_;
};

template <typename Tag, typename LHS, typename RHS>
binary_operator_pattern<Tag, LHS, RHS> binary_operator(Tag tag, LHS lhs, RHS rhs)
{
    return binary_operator_pattern<Tag, LHS, RHS>(tag, lhs, rhs);
}

template <typename LHS, typename RHS>
binary_operator_pattern<any, LHS, RHS> binary_operator(LHS lhs, RHS rhs)
{
    return binary_operator_pattern<any, LHS, RHS>(_, lhs, rhs);
}

template<typename LHS, typename RHS>
auto operator+(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::plus), lhs, rhs);
}

template<typename LHS, typename RHS>
auto operator-(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::minus), lhs, rhs);
}

template<typename LHS, typename RHS>
auto operator*(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::multiplies), lhs, rhs);
}

template<typename LHS, typename RHS>
auto operator/(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::divides), lhs, rhs);
}

template<typename LHS, typename RHS>
auto operator%(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::modulus), lhs, rhs);
}

template<typename LHS, typename RHS>
auto div_floor(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::div_floor), lhs, rhs);
}

template<typename LHS, typename RHS>
auto assign(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::assign), lhs, rhs);
}

template<typename LHS, typename RHS>
auto plus_assign(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::plus_assign), lhs, rhs);
}

template<typename LHS, typename RHS>
auto equal_to(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::equal_to), lhs, rhs);
}

template<typename LHS, typename RHS>
auto not_equal_to(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::not_equal_to), lhs, rhs);
}

template<typename LHS, typename RHS>
auto less(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::less), lhs, rhs);
}

template<typename LHS, typename RHS>
auto greater(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::greater), lhs, rhs);
}

template<typename LHS, typename RHS>
auto less_equal(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::less_equal), lhs, rhs);
}

template<typename LHS, typename RHS>
auto greater_equal(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::greater_equal), lhs, rhs);
}

template<typename LHS, typename RHS>
auto logical_and(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::logical_and), lhs, rhs);
}

template<typename LHS, typename RHS>
auto logical_or(LHS lhs, RHS rhs)
{
    return binary_operator(value(binary_op_tag::logical_or), lhs, rhs);
}

}
}
}

#endif