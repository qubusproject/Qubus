#ifndef QBB_QUBUS_TENSOR_VAR_HPP
#define QBB_QUBUS_TENSOR_VAR_HPP

#include <qbb/qubus/runtime.hpp>
#include <qbb/qubus/local_tensor.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/qubus/IR/type.hpp>
#include <qbb/util/integers.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <qbb/qubus/indexed_tensor_expr_context.hpp>
#include <qbb/qubus/grammar.hpp>
#include <qbb/qubus/IR_emitter.hpp>

#include <qbb/qubus/associated_qubus_type.hpp>

#include <memory>
#include <vector>
#include <utility>

namespace qbb
{
namespace qubus
{

template <typename T, long int Rank>
class tensor;
    
template <typename T, long int Rank>
class tensor_expr : public tensor_expr_<typename boost::proto::terminal<tensor_expr_info>::type>
{
public:
    template <typename Expr>
    tensor_expr(const Expr& expr)
    : tensor_expr::proto_derived_expr(tensor_expr::proto_base_expr::make(
          tensor_expr_info(types::tensor(associated_qubus_type<T>::get()), emit_ast(expr))))
    {
    }
    
    tensor_expr(const tensor<T, Rank>& var)
    : tensor_expr(tensor_to_expr(var))
    {
    }

    const plan& compiled_plan() const
    {
        return proto::value(*this).compiled_plan();
    }

    const std::vector<std::shared_ptr<object>>& args() const
    {
        return proto::value(*this).args();
    }
    
private:
    static auto tensor_to_expr(const tensor<T, Rank>& var)
    {
        index i("i");
        index j("j");
        
        return def_tensor(i, j) [ var(i, j) ];
    }
};

template <typename T, long int Rank>
class tensor
    : public tensor_expr_<typename boost::proto::terminal<std::shared_ptr<local_tensor>>::type>
{
public:
    using value_type = T;

    template <typename... SizeTypes>
    explicit tensor(SizeTypes... sizes_)
    : tensor::proto_derived_expr(tensor::proto_base_expr::make(
          std::shared_ptr<local_tensor>(get_runtime().get_object_factory().create_tensor(
              associated_qubus_type<T>::get(), {util::to_uindex(sizes_)...}))))
    {
    }

    tensor<T, Rank>& operator=(const tensor_expr<T, Rank>& tensor_expr)
    {
        const auto& args = tensor_expr.args();

        execution_context ctx;
        
        for (const auto& arg : args)
        {
            ctx.push_back_arg(arg);
        }
        
        ctx.push_back_arg(get_object());

        auto p = tensor_expr.compiled_plan();

        get_runtime().execute(p, std::move(ctx));

        return *this;
    }

    hpx::shared_future<void> when_ready() const
    {
        return get_runtime().when_ready(*boost::proto::value(*this));
    }
    
    std::shared_ptr<local_tensor> get_object()
    {
        return boost::proto::value(*this);
    }
};

inline std::ostream& operator<<(std::ostream& os, const local_tensor&)
{
    return os << "tensor ( Rank = " << 2 << " )";
}
}
}

#endif