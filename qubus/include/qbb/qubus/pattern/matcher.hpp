#ifndef QBB_QUBUS_PATTERN_MATCHER_HPP
#define QBB_QUBUS_PATTERN_MATCHER_HPP

#include <qbb/qubus/pattern/pattern.hpp>

#include <qbb/util/function_traits.hpp>
#include <qbb/util/unused.hpp>

#include <boost/optional.hpp>

#include <qbb/util/make_unique.hpp>

#include <tuple>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

namespace qbb
{
namespace qubus
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

template <typename BaseType, typename ResultType>
class case_type
{
public:
    using result_type = ResultType;
    using try_match_result_type = typename detail::case_executor<ResultType, BaseType>::result_type;

    template <typename Pattern, typename Callback>
    explicit case_type(Pattern pattern_, Callback callback_)
    : self_(util::make_unique<case_wrapper<Pattern, Callback>>(std::move(pattern_),
                                                               std::move(callback_)))
    {
    }

    case_type(const case_type&) = delete;
    case_type& operator=(const case_type&) = delete;

    case_type(case_type&&) noexcept = default;
    case_type& operator=(case_type&&)  noexcept = default;

    try_match_result_type try_match(const BaseType& value) const
    {
        return self_->try_match(value);
    }

    void reset() const
    {
        self_->reset();
    }

private:
    class case_interface
    {
    public:
        case_interface() = default;
        virtual ~case_interface() = default;

        case_interface(const case_interface&) = delete;
        case_interface& operator=(const case_interface&) = delete;

        virtual try_match_result_type try_match(const BaseType& value) const = 0;
        virtual void reset() const = 0;
    };

    template <typename Pattern, typename Callback>
    class case_wrapper final : public case_interface
    {
    public:
        case_wrapper(Pattern pattern_, Callback callback_)
        : pattern_(std::move(pattern_)), callback_(std::move(callback_))
        {
        }

        virtual ~case_wrapper() = default;

        try_match_result_type try_match(const BaseType& value) const override
        {
            if (pattern_.match(value))
            {
                return detail::case_executor<result_type, BaseType>()(callback_, value);
            }
            else
            {
                return {};
            }
        }

        void reset() const override
        {
            pattern_.reset();
        }

    private:
        Pattern pattern_;
        Callback callback_;
    };

private:
    std::unique_ptr<case_interface> self_;
};

template <typename T>
T unwrap_case_result(T value)
{
    return value;
}

template <typename T>
T unwrap_case_result(boost::optional<T> value)
{
    return *value;
}
}

template <typename BaseType, typename ResultType>
class matcher
{
public:
    using result_type = ResultType;
    using try_match_result_type =
        typename detail::case_executor<result_type, BaseType>::result_type;

    template <typename Pattern, typename Callback>
    matcher<BaseType, result_type> case_(Pattern pattern, Callback callback) &&
    {
        cases_.emplace_back(std::move(pattern), std::move(callback));

        return std::move(*this);
    }

    ResultType match(const BaseType& value) const
    {
        for (const auto& case_ : cases_)
        {
            if (auto result = case_.try_match(value))
            {
                return detail::unwrap_case_result(result);
            }
            else
            {
                case_.reset();
            }
        }

        throw std::logic_error("no case is applicable");
    }

    try_match_result_type try_match(const BaseType& value) const
    {
        for (const auto& case_ : cases_)
        {
            if (auto result = case_.try_match(value))
            {
                return detail::unwrap_case_result(result);
            }
            else
            {
                case_.reset();
            }
        }

        return {};
    }

    void reset() const
    {
        for (const auto& case_ : cases_)
        {
            case_.reset();
        }
    }

private:
    std::vector<detail::case_type<BaseType, result_type>> cases_;
};

template <typename BaseType>
class matcher<BaseType, void>
{
public:
    using result_type = void;
    using try_match_result_type =
        typename detail::case_executor<result_type, BaseType>::result_type;

    template <typename Pattern, typename Callback>
    matcher<BaseType, result_type> case_(Pattern pattern, Callback callback) &&
    {
        cases_.emplace_back(std::move(pattern), std::move(callback));

        return std::move(*this);
    }

    result_type match(const BaseType& value) const
    {
        for (const auto& case_ : cases_)
        {
            if (auto result = case_.try_match(value))
            {
                detail::unwrap_case_result(result);

                return;
            }
            else
            {
                case_.reset();
            }
        }

        throw std::logic_error("no case is applicable");
    }

    try_match_result_type try_match(const BaseType& value) const
    {
        for (const auto& case_ : cases_)
        {
            if (auto result = case_.try_match(value))
            {
                return detail::unwrap_case_result(result);
            }
            else
            {
                case_.reset();
            }
        }

        return {};
    }

    void reset() const
    {
        for (const auto& case_ : cases_)
        {
            case_.reset();
        }
    }

private:
    std::vector<detail::case_type<BaseType, result_type>> cases_;
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