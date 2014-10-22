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

template <typename Body, typename Indices>
class sum_pattern
{
public:
    sum_pattern(Body body_, Indices indices_)
    : body_(std::move(body_)), indices_(std::move(indices_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<sum_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<sum_expr>())
        {
            if (body_.match(concret_value->body()))
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

private:
    Body body_;
    Indices indices_;
};


template <typename Body, typename Indices>
sum_pattern<Body, Indices> sum(Body body, Indices indices)
{
    return sum_pattern<Body, Indices>(body, indices);
}

template <typename Body, typename... Indices>
auto sum_n(Body body, Indices... indices)
{
    return sum(body, sequence(indices...));
}

}
}
}

#endif