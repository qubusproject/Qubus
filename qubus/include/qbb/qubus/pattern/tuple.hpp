#ifndef QBB_PATTERN_TUPLE_HPP
#define QBB_PATTERN_TUPLE_HPP

#include <qbb/qubus/pattern/variable.hpp>

#include <qbb/util/unused.hpp>

#include <tuple>
#include <utility>
#include <algorithm>
#include <iterator>
#include <functional>

inline namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename... Elements>
class tuple_pattern
{
public:
    explicit tuple_pattern(std::tuple<Elements...> elements_)
    : elements_(std::move(elements_))
    {
    }

    template <typename... ElementValues>
    bool match(const std::tuple<ElementValues...>& value, const variable<std::tuple<ElementValues...>>* var = nullptr) const
    {
        static_assert(sizeof...(ElementValues) == sizeof...(Elements), "The number of tuple elements has to match.");

        return match_impl(value, var, std::make_index_sequence<sizeof...(Elements)>());
    }

    void reset() const
    {
        reset_impl(std::make_index_sequence<sizeof...(Elements)>());
    }
private:
    template <typename... ElementValues, std::size_t... Indices>
    bool match_impl(const std::tuple<ElementValues...>& value, const variable<std::tuple<ElementValues...>>* var, std::index_sequence<Indices...>) const
    {
        bool results[] = {std::get<Indices>(elements_).match(std::get<Indices>(value))...};

        bool result = std::accumulate(std::begin(results), std::end(results), true, std::logical_and<bool>());

        if (result)
        {
            if (var)
            {
                var->set(value);
            }

            return true;
        }

        return false;
    }

    template<std::size_t... Indices>
    void reset_impl(std::index_sequence<Indices...>) const
    {
        auto QBB_UNUSED(dummy) = {(std::get<Indices>(elements_).reset(), 0)...};
    }

    std::tuple<Elements...> elements_;
};

template<typename... Elements>
tuple_pattern<Elements...> tuple(Elements... elements)
{
    return tuple_pattern<Elements...>(std::make_tuple(std::move(elements)...));
}

}
}
}

#endif
