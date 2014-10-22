#ifndef QBB_KUBUS_PATTERN_MATCHER_HPP
#define QBB_KUBUS_PATTERN_MATCHER_HPP

#include <qbb/kubus/pattern/pattern.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
#include <boost/fusion/algorithm/iteration/reverse_fold.hpp>

#include <tuple>
#include <stdexcept>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template<typename BaseType, typename ResultType, typename Cases = boost::fusion::vector<>>
class matcher
{
public:
    explicit matcher(Cases cases_ = Cases())
    : cases_(cases_)
    {
    }
    
    template<typename Pattern, typename Callback>
    auto case_(Pattern pattern, Callback callback) &&
    {
        auto new_cases = as_vector(push_back(cases_, std::make_tuple(pattern, callback)));
        
        return matcher<BaseType, ResultType, decltype(new_cases)>(new_cases);
    }
    
    ResultType match(const BaseType& value) const
    {
        auto f = [](const auto& continuation, const auto& case_)
        {
            return [continuation, &case_](const BaseType& value)
            {
                if(std::get<0>(case_).match(value))
                {
                    return ResultType(std::get<1>(case_)());
                }
                else
                {
                    return ResultType(continuation(value));
                }
            };
        };
        
        auto matcher = boost::fusion::reverse_fold(cases_, [](const BaseType&) { throw std::logic_error("no case is applicable"); }, f);
        
        return matcher(value);
    }
private:
    Cases cases_;
};

template<typename BaseType, typename ResultType>
auto make_matcher()
{
    return matcher<BaseType, ResultType>();
}

}
}
}

#endif