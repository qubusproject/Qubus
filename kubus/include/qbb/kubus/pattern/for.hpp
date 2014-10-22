#ifndef QBB_KUBUS_PATTERN_FOR_HPP
#define QBB_KUBUS_PATTERN_FOR_HPP

#include <qbb/kubus/IR/for_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
class for_pattern
{
public:
    for_pattern(Index index_, LowerBound lower_bound_, UpperBound upper_bound_, Body body_)
    : index_(std::move(index_)), lower_bound_(std::move(lower_bound_)),
      upper_bound_(std::move(upper_bound_)), body_(std::move(body_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<for_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<for_expr>())
        {
            if (index_.match(concret_value->index()))
            {
                if (lower_bound_.match(concret_value->lower_bound()))
                {
                    if (upper_bound_.match(concret_value->upper_bound()))
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

        return false;
    }

private:
    Index index_;
    LowerBound lower_bound_;
    UpperBound upper_bound_;
    Body body_;
};

template <typename Index, typename LowerBound, typename UpperBound, typename Body>
for_pattern<Index, LowerBound, UpperBound, Body> for_(Index index, LowerBound lower_bound,
                                                      UpperBound upper_bound, Body body)
{
    return for_pattern<Index, LowerBound, UpperBound, Body>(index, lower_bound, upper_bound, body);
}
}
}
}

#endif