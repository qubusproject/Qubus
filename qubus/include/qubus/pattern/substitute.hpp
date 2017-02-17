#ifndef QUBUS_PATTERN_SUBSTITUTE_HPP
#define QUBUS_PATTERN_SUBSTITUTE_HPP

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <vector>
#include <string>
#include <memory>

namespace qubus
{
namespace pattern
{
//TODO: Implement variants which can skip parts/subtrees of the expression. 

template <typename Matcher>
std::unique_ptr<expression> substitute(const expression& expr, const Matcher& matcher)
{
    auto new_expr = try_match(expr, matcher);

    if (new_expr)
    {
        std::vector<std::unique_ptr<expression>> transformed_subexprs;

        for (const auto& subexpr : (*new_expr)->sub_expressions())
        {
            transformed_subexprs.push_back(substitute(subexpr, matcher));
        }

        return (*new_expr)->substitute_subexpressions(std::move(transformed_subexprs));
    }
    else
    {
        std::vector<std::unique_ptr<expression>> transformed_subexprs;

        for (const auto& subexpr : expr.sub_expressions())
        {
            transformed_subexprs.push_back(substitute(subexpr, matcher));
        }

        return expr.substitute_subexpressions(std::move(transformed_subexprs));
    }
}

}
}

#endif