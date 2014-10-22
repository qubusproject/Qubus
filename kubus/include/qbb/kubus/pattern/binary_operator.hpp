#ifndef QBB_KUBUS_PATTERN_BINARY_OPERATOR_HPP
#define QBB_KUBUS_PATTERN_BINARY_OPERATOR_HPP

#include <qbb/kubus/IR/binary_operator_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename Tag, typename LHS, typename RHS>
class binary_operator_pattern
{
public:
    binary_operator_pattern(Tag tag_, LHS lhs_, RHS rhs_)
    :tag_{std::move(tag_)}, lhs_{std::move(lhs_)}, rhs_{std::move(rhs_)}
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<binary_operator_expr>* var = nullptr) const
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
}
}
}

#endif