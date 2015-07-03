#ifndef QBB_QUBUS_PATTERN_SUBSTITUTE_HPP
#define QBB_QUBUS_PATTERN_SUBSTITUTE_HPP

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <vector>
#include <string>

namespace qbb
{
namespace qubus
{
namespace pattern
{
//TODO: Implement variants which can skip parts/subtrees of the expression. 

template <typename Matcher>
inline expression substitute(const expression& expr, const Matcher& matcher)
{
    auto new_expr = try_match(expr, matcher);

    if (new_expr)
    {
        std::vector<expression> transformed_subexprs;

        for (const auto& subexpr : (*new_expr).sub_expressions())
        {
            transformed_subexprs.push_back(substitute(subexpr, matcher));
        }

        return (*new_expr).substitute_subexpressions(transformed_subexprs);
    }
    else
    {
        std::vector<expression> transformed_subexprs;

        for (const auto& subexpr : expr.sub_expressions())
        {
            transformed_subexprs.push_back(substitute(subexpr, matcher));
        }

        return expr.substitute_subexpressions(transformed_subexprs);
    }
}

}
}
}

#endif