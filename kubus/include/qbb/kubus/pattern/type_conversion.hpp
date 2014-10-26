#ifndef QBB_KUBUS_PATTERN_TYPE_CONVERSION_HPP
#define QBB_KUBUS_PATTERN_TYPE_CONVERSION_HPP

#include <qbb/kubus/IR/type_conversion_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename TargetType, typename Arg>
class type_conversion_pattern
{
public:
    type_conversion_pattern(TargetType target_type_, Arg arg_) : target_type_(std::move(target_type_)), arg_(std::move(arg_))
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
    TargetType target_type_;
    Arg arg_;
};

template <typename TargetType, typename Arg>
type_conversion_pattern<TargetType, Arg> type_conversion(TargetType target_type, Arg arg)
{
    return type_conversion_pattern<TargetType, Arg>(target_type, arg);
}

template <typename Arg>
auto type_conversion(Arg arg)
{
    return type_conversion(_, arg);
}

}
}
}

#endif