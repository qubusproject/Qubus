#ifndef QBB_KUBUS_PATTERN_INTRINSIC_FUNCTION_HPP
#define QBB_KUBUS_PATTERN_INTRINSIC_FUNCTION_HPP

#include <qbb/kubus/IR/intrinsic_function_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{

template <typename Name, typename Args>
class intrinsic_function_pattern
{
public:
    intrinsic_function_pattern(Name name_, Args args_)
    : name_(std::move(name_)), args_(std::move(args_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<intrinsic_function_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<intrinsic_function_expr>())
        {
            if (name_.match(concret_value->name()))
            {
                if (args_.match(concret_value->args()))
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
    Name name_;
    Args args_;
};


template <typename Body, typename Indices>
intrinsic_function_pattern<Body, Indices> intrinsic_function(Body body, Indices indices)
{
    return intrinsic_function_pattern<Body, Indices>(body, indices);
}

template <typename Body, typename... Indices>
auto intrinsic_function_n(Body body, Indices... indices)
{
    return intrinsic_function(body, sequence(indices...));
}

}
}
}

#endif