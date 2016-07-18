#ifndef QBB_QUBUS_PATTERN_DELTA_HPP
#define QBB_QUBUS_PATTERN_DELTA_HPP

#include <qbb/qubus/IR/kronecker_delta_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/any.hpp>

#include <utility>
#include <functional>

namespace qbb
{
namespace qubus
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
    bool match(const BaseType& value, const variable<const kronecker_delta_expr&>* var = nullptr) const
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
    return delta(_, first_index, second_index);
}
}
}
}

#endif
