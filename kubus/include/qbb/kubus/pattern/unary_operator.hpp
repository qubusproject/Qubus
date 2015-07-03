#ifndef QBB_KUBUS_PATTERN_UNARY_OPERATOR_HPP
#define QBB_KUBUS_PATTERN_UNARY_OPERATOR_HPP

#include <qbb/kubus/IR/unary_operator_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
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
    bool match(const BaseType& value, const variable<unary_operator_expr>* var = nullptr) const
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
unary_operator_pattern<any, Arg> binary_operator(Arg arg)
{
    return unary_operator_pattern<any, Arg>(_, arg);
}
}
}
}

#endif