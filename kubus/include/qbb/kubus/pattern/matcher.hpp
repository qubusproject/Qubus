#ifndef QBB_KUBUS_PATTERN_MATCHER_HPP
#define QBB_KUBUS_PATTERN_MATCHER_HPP

#include <qbb/kubus/pattern/pattern.hpp>

#include <qbb/util/function_traits.hpp>
#include <qbb/util/unused.hpp>

#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>

// Workaround for Boost Fusion 1.58
#include <boost/version.hpp>
#if BOOST_VERSION >= 105800
#include <boost/fusion/support/detail/result_of.hpp>
#endif

#include <boost/fusion/algorithm/iteration/reverse_fold.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include <boost/optional.hpp>

#include <tuple>
#include <stdexcept>
#include <type_traits>

namespace qbb
{
namespace kubus
{
namespace pattern
{

namespace detail
{
template <typename ResultType, typename Action, typename BaseType>
ResultType call_action(Action& action, const BaseType& QBB_UNUSED(value),
                       std::false_type QBB_UNUSED(has_self_param))
{
    return ResultType(action());
}

template <typename ResultType, typename Action, typename BaseType>
ResultType call_action(Action& action, const BaseType& value,
                             std::true_type QBB_UNUSED(has_self_param))
{
    return ResultType(action(value));
}

template <typename ResultType, typename BaseType>
struct case_executor
{
    using result_type = boost::optional<ResultType>;

    template <typename F>
    result_type operator()(F& f, const BaseType& value) const
    {
        constexpr auto action_arity = util::function_traits<F>::arity;
        constexpr bool action_has_self_param = action_arity > 0;

        return boost::optional<ResultType>(call_action<ResultType>(
            f, value, std::integral_constant<bool, action_has_self_param>()));
    }
};

template <typename BaseType>
struct case_executor<void, BaseType>
{
    using result_type = bool;

    template <typename F>
    result_type operator()(F& f, const BaseType& value) const
    {
        constexpr auto action_arity = util::function_traits<F>::arity;
        constexpr bool action_has_self_param = action_arity > 0;
        
        call_action<void>(f, value, std::integral_constant<bool, action_has_self_param>());

        return true;
    }
};
}

template <typename BaseType, typename ResultType, typename Cases = boost::fusion::vector<>>
class matcher
{
public:
    using result_type = ResultType;
    using try_match_result_type = typename detail::case_executor<ResultType, BaseType>::result_type;

    explicit matcher(Cases cases_ = Cases()) : cases_(cases_)
    {
    }

    template <typename Pattern, typename Callback>
    auto case_(Pattern pattern, Callback callback) &&
    {
        auto new_cases = as_vector(push_back(cases_, std::make_tuple(pattern, callback)));

        return matcher<BaseType, ResultType, decltype(new_cases)>(new_cases);
    }

    ResultType match(const BaseType& value) const
    {
        auto f = [](const auto& continuation, const auto& case_)
        {
            constexpr auto action_arity = util::function_traits<
                typename std::decay<decltype(std::get<1>(case_))>::type>::arity;

            return [continuation, &case_](const BaseType& value)
            {
                if (std::get<0>(case_).match(value))
                {
                    constexpr bool action_has_self_param = action_arity > 0;
                    return detail::call_action<ResultType>(
                        std::get<1>(case_), value,
                        std::integral_constant<bool, action_has_self_param>());
                }
                else
                {
                    std::get<0>(case_).reset();

                    return continuation(value);
                }
            };
        };

        auto matcher =
            boost::fusion::reverse_fold(cases_,
                                        [](const BaseType&) -> ResultType
                                        {
                                            throw std::logic_error("no case is applicable");
                                        },
                                        f);

        return matcher(value);
    }

    try_match_result_type try_match(const BaseType& value) const
    {
        auto f = [](const auto& continuation, const auto& case_)
        {
            return [continuation, &case_](const BaseType& value)
            {
                if (std::get<0>(case_).match(value))
                {
                    return detail::case_executor<result_type, BaseType>()(std::get<1>(case_), value);
                }
                else
                {
                    std::get<0>(case_).reset();

                    return continuation(value);
                }
            };
        };

        auto matcher = boost::fusion::reverse_fold(cases_,
                                                   [](const BaseType&) -> try_match_result_type
                                                   {
                                                       return {};
                                                   },
                                                   f);

        return matcher(value);
    }

    void reset() const
    {
        boost::fusion::for_each(cases_, [](const auto& case_)
                                {
                                    std::get<0>(case_).reset();
                                });
    }

private:
    Cases cases_;
};

template <typename BaseType, typename ResultType>
auto make_matcher()
{
    return matcher<BaseType, ResultType>();
}
}
}
}

#endif