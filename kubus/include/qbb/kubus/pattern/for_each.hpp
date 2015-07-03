#ifndef QBB_KUBUS_PATTERN_FOR_EACH_HPP
#define QBB_KUBUS_PATTERN_FOR_EACH_HPP

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/IR.hpp>
#include <qbb/kubus/pattern/core.hpp>

namespace qbb
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