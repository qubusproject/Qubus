#ifndef QBB_QUBUS_PATTERN_FOR_HPP
#define QBB_QUBUS_PATTERN_FOR_HPP

#include <qbb/qubus/IR/for_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/literal.hpp>
#include <qbb/qubus/pattern/value.hpp>

#include <utility>
#include <functional>

namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename Index, typename LowerBound, typename UpperBound, typename Increment,
          typename Body>
class for_pattern
{
public:
    for_pattern(Index index_, LowerBound lower_bound_, UpperBound upper_bound_,
                Increment increment_, Body body_)
    : index_(std::move(index_)), lower_bound_(std::move(lower_bound_)),
      upper_bound_(std::move(upper_bound_)), increment_(std::move(increment_)),
      body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const for_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_expr>())
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

        return false;
    }

    void reset() const
    {
        index_.reset();
        lower_bound_.reset();
        upper_bound_.reset();
        increment_.reset();
        body_.reset();
    }

private:
    Index index_;
    LowerBound lower_bound_;
    UpperBound upper_bound_;
    Increment increment_;
    Body body_;
};

template <typename Index, typename LowerBound, typename UpperBound, typename Increment,
          typename Body>
for_pattern<Index, LowerBound, UpperBound, Increment, Body>
for_(Index index, LowerBound lower_bound, UpperBound upper_bound, Increment increment, Body body)
{
    return for_pattern<Index, LowerBound, UpperBound, Increment, Body>(
        index, lower_bound, upper_bound, increment, body);
}

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
auto for_(Index index, LowerBound lower_bound, UpperBound upper_bound, Body body)
    -> decltype(for_(index, lower_bound, upper_bound, integer_literal(value(1)), body))
{
    return for_(index, lower_bound, upper_bound, integer_literal(value(1)), body);
}
}
}
}

#endif