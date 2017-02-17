#ifndef QUBUS_PATTERN_FOLD_HPP_HPP
#define QUBUS_PATTERN_FOLD_HPP_HPP

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/core.hpp>

namespace qubus
{
namespace pattern
{

template <typename Matcher, typename F>
typename Matcher::result_type fold(const expression& expr, const Matcher& matcher, F accumulator)
{
    auto state = match(expr, matcher);

    for (const auto& subexpr : expr.sub_expressions())
    {
        auto value = fold(subexpr, matcher, accumulator);
        state = accumulator(state, value);
    }

    return state;
}

}
}

#endif
