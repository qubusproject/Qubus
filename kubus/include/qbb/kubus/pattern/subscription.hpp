#ifndef QBB_QUBUS_PATTERN_SUBSCRIPTION_HPP
#define QBB_QUBUS_PATTERN_SUBSCRIPTION_HPP

#include <qbb/kubus/IR/subscription_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
namespace pattern
{

template <typename IndexedExpr, typename Indices>
class subscription_pattern
{
public:
    subscription_pattern(IndexedExpr indexed_expr_, Indices indices_)
    : indexed_expr_(std::move(indexed_expr_)), indices_(std::move(indices_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<subscription_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<subscription_expr>())
        {
            if (indexed_expr_.match(concret_value->indexed_expr()))
            {
                if (indices_.match(concret_value->indices()))
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
        indexed_expr_.reset();
        indices_.reset();
    }
private:
    IndexedExpr indexed_expr_;
    Indices indices_;
};


template <typename IndexedExpr, typename Indices>
subscription_pattern<IndexedExpr, Indices> subscription(IndexedExpr indexed_expr, Indices indices)
{
    return subscription_pattern<IndexedExpr, Indices>(indexed_expr, indices);
}

template <typename IndexedExpr, typename... Indices>
auto subscription_n(IndexedExpr indexed_expr, Indices... indices)
{
    return subscription(indexed_expr, sequence(indices...));
}

}
}
}

#endif