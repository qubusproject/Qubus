#ifndef QUBUS_QTL_PATTERN_CAPTURED_MULTI_INDEX_HPP
#define QUBUS_QTL_PATTERN_CAPTURED_MULTI_INDEX_HPP

#include <qubus/pattern/variable.hpp>
#include <qubus/qtl/IR/multi_index_expr.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace pattern
{
template <typename MultiIndex, typename ElementIndices>
class captured_multi_index_pattern
{
public:
    explicit captured_multi_index_pattern(MultiIndex multi_index_, ElementIndices element_indices_)
    : multi_index_(multi_index_), element_indices_(element_indices_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value,
               const qubus::pattern::variable<const multi_index_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<multi_index_expr>())
        {
            if (multi_index_.match(concret_value->multi_index()))
            {
                if (element_indices_.match(concret_value->element_indices()))
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
        multi_index_.reset();
        element_indices_.reset();
    }

private:
    MultiIndex multi_index_;
    ElementIndices element_indices_;
};

template <typename MultiIndex, typename ElementIndices>
auto captured_multi_index(MultiIndex multi_index, ElementIndices element_indices)
{
    return captured_multi_index_pattern<MultiIndex, ElementIndices>(std::move(multi_index),
                                                                    std::move(element_indices));
}
}
}
}

#endif
