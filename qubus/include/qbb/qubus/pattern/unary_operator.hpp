#ifndef QUBUS_PATTERN_UNARY_OPERATOR_HPP
#define QUBUS_PATTERN_UNARY_OPERATOR_HPP

#include <qbb/qubus/IR/unary_operator_expr.hpp>

#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/pattern/value.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{
template <typename Tag, typename Arg>
class unary_operator_pattern
{
public:
    unary_operator_pattern(Tag tag_, Arg arg_)
    :tag_(std::move(tag_)), arg_(std::move(arg_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const unary_operator_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<unary_operator_expr>())
        {
            if (tag_.match(concret_value->tag()))
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
        }

        return false;
    }

    void reset() const
    {
        tag_.reset();
        arg_.reset();
    }
private:
    Tag tag_;
    Arg arg_;
};

template <typename Tag, typename Arg>
unary_operator_pattern<Tag, Arg> unary_operator(Tag tag, Arg arg)
{
    return unary_operator_pattern<Tag, Arg>(tag, arg);
}

template <typename Arg>
unary_operator_pattern<any, Arg> unary_operator(Arg arg)
{
    return unary_operator_pattern<any, Arg>(_, arg);
}

template<typename Arg>
auto nop(Arg arg)
{
    return unary_operator(value(unary_op_tag::nop), arg);
}

template<typename Arg>
auto operator+(Arg arg)
{
    return unary_operator(value(unary_op_tag::plus), arg);
}

template<typename Arg>
auto operator-(Arg arg)
{
    return unary_operator(value(unary_op_tag::negate), arg);
}

template<typename Arg>
auto logical_not(Arg arg)
{
    return unary_operator(value(unary_op_tag::logical_not), arg);
}

}
}

#endif