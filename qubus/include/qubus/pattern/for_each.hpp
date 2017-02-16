#ifndef QUBUS_PATTERN_FOR_EACH_HPP
#define QUBUS_PATTERN_FOR_EACH_HPP

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

namespace qubus
{
namespace pattern
{

template <typename Matcher>
inline void for_each(const expression& expr, const Matcher& matcher)
{
    try_match(expr, matcher);

    for (const auto& subexpr : expr.sub_expressions())
    {
        for_each(subexpr, matcher);
    }
}
}
}

#endif