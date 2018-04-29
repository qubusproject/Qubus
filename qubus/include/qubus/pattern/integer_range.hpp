#ifndef QUBUS_PATTERN_INTEGER_RANGE_HPP
#define QUBUS_PATTERN_INTEGER_RANGE_HPP

#include <qubus/IR/integer_range_expr.hpp>

#include <qubus/pattern/any.hpp>
#include <qubus/pattern/value.hpp>
#include <qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace pattern
{
template <typename LowerBound, typename UpperBound, typename Stride>
class integer_range_pattern
{
public:
    integer_range_pattern(LowerBound lower_bound_, UpperBound upper_bound_, Stride stride_)
    : lower_bound_(std::move(lower_bound_)),
      upper_bound_(std::move(upper_bound_)),
      stride_(std::move(stride_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const variable<const integer_range_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<integer_range_expr>())
        {
            if (lower_bound_.match(concret_value->lower_bound()))
            {
                if (upper_bound_.match(concret_value->upper_bound()))
                {
                    if (stride_.match(concret_value->stride()))
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

        return false;
    }

    void reset() const
    {
        lower_bound_.reset();
        upper_bound_.reset();
        stride_.reset();
    }

private:
    LowerBound lower_bound_;
    UpperBound upper_bound_;
    Stride stride_;
};

template <typename LowerBound, typename UpperBound, typename Stride>
integer_range_pattern<LowerBound, UpperBound, Stride>
integer_range(LowerBound lower_bound, UpperBound upper_bound, Stride stride)
{
    return integer_range_pattern<LowerBound, UpperBound, Stride>(
        std::move(lower_bound), std::move(upper_bound), std::move(stride));
}

} // namespace pattern
} // namespace qubus

#endif
