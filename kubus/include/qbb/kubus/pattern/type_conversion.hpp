#ifndef QBB_KUBUS_PATTERN_TYPE_CONVERSION_HPP
#define QBB_KUBUS_PATTERN_TYPE_CONVERSION_HPP

#include <qbb/kubus/IR/type_conversion_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename Arg>
class type_conversion_pattern
{
public:
    type_conversion_pattern(Arg arg_) : arg_(std::move(arg_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<type_conversion_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<type_conversion_expr>())
        {
            if (arg_.match(concret_value->arg()))
            {
                if (var)
                {
                    var->set(*concret_value);
                }

                return true;
            }
        }

        return false;
    }

private:
    Arg arg_;
};

template <typename Arg>
type_conversion_pattern<Arg> type_conversion(Arg arg)
{
    return type_conversion_pattern<Arg>(arg);
}
}
}
}

#endif