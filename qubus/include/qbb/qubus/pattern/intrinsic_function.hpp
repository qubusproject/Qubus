#ifndef QBB_QUBUS_PATTERN_INTRINSIC_FUNCTION_HPP
#define QBB_QUBUS_PATTERN_INTRINSIC_FUNCTION_HPP

#include <qbb/qubus/IR/intrinsic_function_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/sequence.hpp>

#include <utility>

namespace qbb
{
namespace qubus
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

    void reset() const
    {
        name_.reset();
        args_.reset();
    }
private:
    Name name_;
    Args args_;
};


template <typename Name, typename Args>
intrinsic_function_pattern<Name, Args> intrinsic_function(Name name, Args args)
{
    return intrinsic_function_pattern<Name, Args>(name, args);
}

template <typename Name, typename... Args>
auto intrinsic_function_n(Name name, Args... args)
{
    return intrinsic_function(name, sequence(args...));
}

}
}
}

#endif