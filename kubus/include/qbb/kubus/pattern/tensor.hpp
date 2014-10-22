#ifndef QBB_KUBUS_PATTERN_TENSOR_HPP
#define QBB_KUBUS_PATTERN_TENSOR_HPP

#include <qbb/kubus/IR/tensor_access_expr.hpp>

#include <qbb/kubus/pattern/variable.hpp>
#include <qbb/kubus/pattern/any.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{
namespace pattern
{
template <typename Variable>
class tensor_pattern
{
public:
    tensor_pattern(Variable variable_) : variable_(variable_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<tensor_access_expr>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<tensor_access_expr>())
        {
            if (variable_.match(concret_value->variable()))
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
    Variable variable_;
};

template <typename Variable>
tensor_pattern<Variable> tensor(Variable variable)
{
    return tensor_pattern<Variable>(variable);
}
}
}
}

#endif