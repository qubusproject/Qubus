#ifndef QBB_QUBUS_PATTERN_FOR_EACH_HPP
#define QBB_QUBUS_PATTERN_FOR_EACH_HPP

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

inline namespace qbb
{
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
}

#endif