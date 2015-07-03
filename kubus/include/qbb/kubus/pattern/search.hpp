#ifndef QBB_KUBUS_PATTERN_SEARCH_HPP
#define QBB_KUBUS_PATTERN_SEARCH_HPP

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/pattern/IR.hpp>
#include <qbb/kubus/pattern/core.hpp>

#include <boost/optional.hpp>

namespace qbb
{
namespace qubus
{
namespace pattern
{

//TODO: implement a version for result type void
template <typename Matcher>
inline boost::optional<typename Matcher::result_type> search(const expression& expr, const Matcher& matcher)
{
    auto result = try_match(expr, matcher);
    
    if(result)
    {
        return *result;
    }
    else
    {
        for(const auto& subexpr : expr.sub_expressions())
        {
            if(auto result = search(subexpr, matcher))
            {
                return result;
            }
        }
        
        return boost::none;
    }
}

}
}
}

#endif
