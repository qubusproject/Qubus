#ifndef QUBUS_PATTERN_SEARCH_HPP
#define QUBUS_PATTERN_SEARCH_HPP

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <boost/optional.hpp>

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
        return std::move(*result);
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

#endif
