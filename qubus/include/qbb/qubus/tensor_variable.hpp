#ifndef QBB_QUBUS_TENSOR_VAR_HPP
#define QBB_QUBUS_TENSOR_VAR_HPP

#include <qbb/qubus/local_runtime.hpp>

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

    computelet stored_computelet() const
    {
        return proto::value(*this).stored_computelet();
    }

    const std::vector<object_client>& args() const
    {
        return proto::value(*this).args();
    }
    
private:
    static auto tensor_to_expr(const tensor<T, Rank>& var)
    {
        // TODO: Generalize this.

        index i("i");
        index j("j");
        
        return def_tensor(i, j) [ var(i, j) ];
    }
};

template <typename T, long int Rank>
class tensor
    : public tensor_expr_<typename boost::proto::terminal<tensor_info<T, Rank>>::type>
{
public:
    using value_type = T;

    template <typename... SizeTypes>
    explicit tensor(SizeTypes... sizes_)
    : tensor::proto_derived_expr(tensor::proto_base_expr::make(
            tensor_info<T, Rank>(get_runtime().get_object_factory().create_array(
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

        auto c = tensor_expr.stored_computelet();

        get_runtime().execute(c, std::move(ctx));

        return *this;
    }

    hpx::shared_future<void> when_ready() const
    {
        return boost::proto::value(*this).get_last_modification();
    }

    object_client get_object() const
    {
        return boost::proto::value(*this).get_object();
    }
};

template<typename T, long int Rank>
std::ostream& operator<<(std::ostream& os, const tensor_info<T, Rank>&)
{
    return os << "tensor ( Rank = " << Rank << " )";
}
}
}

#endif