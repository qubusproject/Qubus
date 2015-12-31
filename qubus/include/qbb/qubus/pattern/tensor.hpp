#ifndef QBB_QUBUS_PATTERN_TENSOR_HPP
#define QBB_QUBUS_PATTERN_TENSOR_HPP

#include <qbb/qubus/IR/variable_ref_expr.hpp>
#include <qbb/qubus/pattern/variable.hpp>
#include <qbb/qubus/pattern/any.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/qubus/pattern/type.hpp>

#include <utility>

namespace qbb
{
namespace qubus
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
    bool match(const BaseType& value, const variable<variable_ref_expr>* var = nullptr) const
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
    return tensor_pattern<Declaration, decltype(tensor_t(_))>(declaration, tensor_t(_));
}

inline auto tensor()
{
    return tensor(_);
}

template <typename Declaration>
auto sparse_tensor(Declaration declaration)
{
    return tensor_pattern<Declaration, decltype(sparse_tensor_t(_))>(declaration, sparse_tensor_t(_));
}

inline auto sparse_tensor()
{
    return sparse_tensor(_);
}
}
}
}

#endif