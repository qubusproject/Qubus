#ifndef QBB_QUBUS_PATTERN_FOR_HPP
#define QBB_QUBUS_PATTERN_FOR_HPP

#include <qbb/qubus/IR/for_expr.hpp>

#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/pattern/literal.hpp>
#include <qbb/qubus/pattern/value.hpp>
#include <qbb/qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace pattern
{

template <typename Order, typename Index, typename LowerBound, typename UpperBound,
          typename Increment, typename Body>
class for_pattern
{
public:
    for_pattern(Order order_, Index index_, LowerBound lower_bound_, UpperBound upper_bound_,
                Increment increment_, Body body_)
    : order_(std::move(order_)), index_(std::move(index_)), lower_bound_(std::move(lower_bound_)),
      upper_bound_(std::move(upper_bound_)), increment_(std::move(increment_)),
      body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const for_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_expr>())
        {
            if (order_.match(concret_value->order()))
            {
                if (index_.match(concret_value->loop_index()))
                {
                    if (lower_bound_.match(concret_value->lower_bound()))
                    {
                        if (upper_bound_.match(concret_value->upper_bound()))
                        {
                            if (increment_.match(concret_value->increment()))
                            {
                                if (body_.match(concret_value->body()))
                                {
                                    if (var)
                                    {
                                        var->set(*concret_value);
                                    }

                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        return false;
    }

    void reset() const
    {
        order_.reset();
        index_.reset();
        lower_bound_.reset();
        upper_bound_.reset();
        increment_.reset();
        body_.reset();
    }

private:
    Order order_;
    Index index_;
    LowerBound lower_bound_;
    UpperBound upper_bound_;
    Increment increment_;
    Body body_;
};

template <typename Order, typename Index, typename LowerBound, typename UpperBound,
          typename Increment, typename Body>
auto for_(Order order, Index index, LowerBound lower_bound, UpperBound upper_bound,
          Increment increment, Body body)
{
    return for_pattern<Order, Index, LowerBound, UpperBound, Increment, Body>(
        order, index, lower_bound, upper_bound, increment, body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Increment,
          typename Body>
auto for_(Index index, LowerBound lower_bound, UpperBound upper_bound, Increment increment,
          Body body)
{
    return for_pattern<any, Index, LowerBound, UpperBound, Increment, Body>(
        _, index, lower_bound, upper_bound, increment, body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
auto for_(Index index, LowerBound lower_bound, UpperBound upper_bound, Body body)
{
    return for_(index, lower_bound, upper_bound, integer_literal(value(1)), body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Increment,
          typename Body>
auto sequential_for(Index index, LowerBound lower_bound, UpperBound upper_bound,
                    Increment increment, Body body)
{
    return for_pattern<value_pattern<execution_order>, Index, LowerBound, UpperBound, Increment,
                       Body>(value(execution_order::sequential), index, lower_bound, upper_bound,
                             increment, body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
auto sequential_for(Index index, LowerBound lower_bound, UpperBound upper_bound, Body body)
{
    return sequential_for(index, lower_bound, upper_bound, integer_literal(value(1)), body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Increment,
          typename Body>
auto unordered_for(Index index, LowerBound lower_bound, UpperBound upper_bound, Increment increment,
                   Body body)
{
    return for_pattern<value_pattern<execution_order>, Index, LowerBound, UpperBound, Increment,
                       Body>(value(execution_order::unordered), index, lower_bound, upper_bound,
                             increment, body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
auto unordered_for(Index index, LowerBound lower_bound, UpperBound upper_bound, Body body)
{
    return unordered_for(index, lower_bound, upper_bound, integer_literal(value(1)), body);
}
}
}

#endif