#ifndef QUBUS_QTL_PATTERN_DELTA_HPP
#define QUBUS_QTL_PATTERN_DELTA_HPP

#include <qubus/qtl/IR/kronecker_delta_expr.hpp>

#include <qubus/pattern/any.hpp>
#include <qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace pattern
{
template <typename Extent, typename FirstIndex, typename SecondIndex>
class kronecker_delta_pattern
{
public:
    kronecker_delta_pattern(Extent extent_, FirstIndex first_index_, SecondIndex second_index_)
    : extent_(std::move(extent_)), first_index_(std::move(first_index_)),
      second_index_(std::move(second_index_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const qubus::pattern::variable<const kronecker_delta_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<kronecker_delta_expr>())
        {
            if (extent_.match(concret_value->extent()))
            {
                if (first_index_.match(concret_value->first_index()) &&
                    second_index_.match(concret_value->second_index()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    void reset() const
    {
        extent_.reset();
        first_index_.reset();
        second_index_.reset();
    }

private:
    Extent extent_;
    FirstIndex first_index_;
    SecondIndex second_index_;
};

template <typename Extent, typename FirstIndex, typename SecondIndex>
kronecker_delta_pattern<Extent, FirstIndex, SecondIndex>
delta(Extent extent, FirstIndex first_index, SecondIndex second_index)
{
    return kronecker_delta_pattern<Extent, FirstIndex, SecondIndex>(extent, first_index,
                                                                    second_index);
}

template <typename FirstIndex, typename SecondIndex>
auto delta(FirstIndex first_index, SecondIndex second_index)
{
    using qubus::pattern::_;

    return delta(_, first_index, second_index);
}
}
}
}

#endif
