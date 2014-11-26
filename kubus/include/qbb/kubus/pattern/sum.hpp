#ifndef QBB_KUBUS_PATTERN_SUM_HPP
#define QBB_KUBUS_PATTERN_SUM_HPP

#include <qbb/kubus/IR/sum_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Body, typename Index>
class sum_pattern
{
public:
    sum_pattern(Body body_, Index index_)
    : body_(std::move(body_)), index_(std::move(index_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<sum_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<sum_expr>())
        {
            if (body_.match(concret_value->body()))
            {
                if (index_.match(concret_value->index_decl()))
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
        body_.reset();
        index_.reset();
    }
private:
    Body body_;
    Index index_;
};


template <typename Body, typename Index>
sum_pattern<Body, Index> sum(Body body, Index index)
{
    return sum_pattern<Body, Index>(body, index);
}

}
}
}

#endif