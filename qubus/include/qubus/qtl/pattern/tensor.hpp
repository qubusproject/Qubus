#ifndef QUBUS_QTL_PATTERN_TENSOR_HPP
#define QUBUS_QTL_PATTERN_TENSOR_HPP

#include <qubus/IR/type.hpp>
#include <qubus/IR/variable_ref_expr.hpp>
#include <qubus/pattern/any.hpp>
#include <qubus/pattern/type.hpp>
#include <qubus/pattern/variable.hpp>

#include <functional>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace pattern
{
template <typename Declaration, typename TensorType>
class tensor_pattern
{
public:
    explicit tensor_pattern(Declaration declaration_, TensorType tensor_type_)
    : declaration_(declaration_), tensor_type_(tensor_type_)
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const qubus::pattern::variable<const variable_ref_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<variable_ref_expr>())
        {
            if (tensor_type_.match(concret_value->declaration().var_type()))
            {
                if (declaration_.match(concret_value->declaration()))
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
        declaration_.reset();
        tensor_type_.reset();
    }

private:
    Declaration declaration_;
    TensorType tensor_type_;
};

template <typename Declaration>
auto tensor(Declaration declaration)
{
    using qubus::pattern::_;

    return tensor_pattern<Declaration, decltype(array_t(_, _))>(declaration, array_t(_, _));
}

inline auto tensor()
{
    return tensor(qubus::pattern::_);
}

template <typename Declaration>
auto sparse_tensor(Declaration declaration)
{
    using qubus::pattern::_;

    return tensor_pattern<Declaration, decltype(sparse_tensor_t(_))>(declaration,
                                                                     sparse_tensor_t(_));
}

inline auto sparse_tensor()
{
    return sparse_tensor(qubus::pattern::_);
}
}
}
}

#endif