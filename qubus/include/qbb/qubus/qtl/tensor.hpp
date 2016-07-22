#ifndef QBB_QUBUS_TENSOR_VAR_HPP
#define QBB_QUBUS_TENSOR_VAR_HPP

#include <qbb/qubus/local_runtime.hpp>

#include <qbb/qubus/IR/pretty_printer.hpp>
#include <qbb/qubus/IR/qir.hpp>

#include <qbb/util/handle.hpp>
#include <qbb/util/integers.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <qbb/qubus/qtl/kronecker_delta_folding_pass.hpp>
#include <qbb/qubus/qtl/lower_abstract_indices.hpp>
#include <qbb/qubus/qtl/lower_top_level_sums.hpp>
#include <qbb/qubus/qtl/multi_index_handling.hpp>
#include <qbb/qubus/qtl/sparse_patterns.hpp>

#include <qbb/qubus/qtl/ast.hpp>
#include <qbb/qubus/qtl/ast_translation.hpp>
#include <qbb/qubus/qtl/index.hpp>

#include <qbb/qubus/get_view.hpp>

#include <qbb/qubus/associated_qubus_type.hpp>

#include <qbb/util/function_traits.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/curry.hpp>
#include <boost/hana/functional/overload.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unpack.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace qbb
{
namespace qubus
{
namespace qtl
{
namespace ast
{

template<typename T, long int Rank>
class tensor;

namespace detail
{

template<typename Literal>
auto wrap_literals(Literal value) ->
typename std::enable_if<std::is_arithmetic<Literal>::value, literal<Literal>>::type
{
    return literal<Literal>(std::move(value));
}

template<typename T>
auto wrap_literals(T value) -> typename std::enable_if<!std::is_arithmetic<T>::value, T>::type
{
    return value;
}

template<typename Kernel>
struct get_kernel_arg_type_t
{
    template<typename Index>
    constexpr auto operator()(Index index) const
    {
        return boost::hana::type_c<util::arg_type<Kernel, Index::value>>;
    }
};

template<typename Kernel>
constexpr auto get_kernel_arg_type = get_kernel_arg_type_t<Kernel>{};

struct instantiate_t
{
    template<typename Type>
    auto operator()(Type type) const
    {
        using value_type = typename Type::type;

        return value_type{};
    }
};

constexpr auto instantiate = instantiate_t{};

template<typename Kernel>
constexpr auto instantiate_kernel_args()
{
    constexpr std::size_t kernel_arity = util::function_traits<Kernel>::arity;

    constexpr auto index_types = boost::hana::transform(
            boost::hana::to_tuple(boost::hana::range_c<std::size_t, 0, kernel_arity>),
            get_kernel_arg_type<Kernel>);

    return boost::hana::transform(index_types, instantiate);
}

struct index_info
{
    explicit index_info(variable_declaration index) : indices{std::move(index)}
    {
    }

    index_info(std::vector<variable_declaration> indices) : indices(std::move(indices))
    {
    }

    index_info(std::vector<variable_declaration> indices, variable_declaration alias)
            : indices(std::move(indices)), alias(std::move(alias))
    {
    }

    std::vector<variable_declaration> indices;
    boost::optional<variable_declaration> alias;
};

class translate_kernel_arg
{
public:
    translate_kernel_arg(std::vector<index_info> &indices_, ast_context &ctx_)
            : indices_(indices_), ctx_(ctx_)
    {
    }

    void operator()(const index &idx) const
    {
        variable_declaration index_var{types::index()};

        ctx_.get().symbol_table.add_symbol(idx.id(), index_var);

        indices_.get().push_back(index_info(index_var));
    }

    template<long int Rank>
    void operator()(const multi_index<Rank> &idx) const
    {
        variable_declaration multi_index_var{types::multi_index()};

        ctx_.get().symbol_table.add_symbol(idx.id(), multi_index_var);

        std::vector<variable_declaration> indices;

        for (long int i = 0; i < idx.rank(); ++i)
        {
            variable_declaration index_var{types::index()};

            ctx_.get().symbol_table.add_symbol(idx[i].id(), index_var);

            indices.push_back(index_var);
        }

        indices_.get().push_back(index_info(std::move(indices), std::move(multi_index_var)));
    }

private:
    std::reference_wrapper<std::vector<index_info>> indices_;
    std::reference_wrapper<ast_context> ctx_;
};

}

template<typename T, long int Rank>
class tensor_expr
{
public:
    template<typename Kernel>
    tensor_expr(const Kernel &kernel)
    {
        auto kernel_args = detail::instantiate_kernel_args<Kernel>();

        auto ast = detail::wrap_literals(boost::hana::unpack(kernel_args, kernel));

        auto result_type = types::array(associated_qubus_type<T>::get());

        std::vector<detail::index_info> indices;

        ast_context ctx;

        boost::hana::for_each(kernel_args, detail::translate_kernel_arg(indices, ctx));

        auto rhs = translate_ast(ast, ctx);

        std::vector<std::unique_ptr<expression>> subscripts;

        for (const auto &idx : indices)
        {
            if (idx.alias)
            {
                subscripts.push_back(var(*idx.alias));
            }
            else
            {
                QBB_ASSERT(idx.indices.size() == 1,
                           "Unnamed multi-indices are currently not supported.");

                subscripts.push_back(var(idx.indices[0]));
            }
        }

        variable_declaration result(result_type);

        auto lhs = subscription(var(result), std::move(subscripts));

        std::unique_ptr<expression> expr = assign(std::move(lhs), std::move(rhs));

        for (const auto &idx : indices | boost::adaptors::reversed)
        {
            if (idx.alias)
            {
                expr = for_all(idx.indices, *idx.alias, std::move(expr));
            }
            else
            {
                QBB_ASSERT(idx.indices.size() == 1,
                           "Unnamed multi-indices are currently not supported.");

                expr = for_all(idx.indices[0], std::move(expr));
            }
        }

        std::vector<variable_declaration> params;

        for (const auto &variable_object : ctx.object_table)
        {
            params.push_back(variable_object.first);
            args_.push_back(variable_object.second);
        }

        function_declaration entry("entry", std::move(params), std::move(result), std::move(expr));

        entry = expand_multi_indices(entry);

        entry = fold_kronecker_deltas(entry);

        entry = optimize_sparse_patterns(entry);

        entry = lower_top_level_sums(entry);

        entry = lower_abstract_indices(entry);

        stored_computelet_ = make_computelet(std::move(entry));
    }

    tensor_expr(const tensor<T, Rank> &var) : tensor_expr(tensor_to_expr(var))
    {
    }

    computelet stored_computelet() const
    {
        return stored_computelet_;
    }

    const std::vector<object> &args() const
    {
        return args_;
    }

private:
    static auto tensor_to_expr(const tensor<T, Rank> &var)
    {
        return [var](multi_index<Rank> I)
        { return var(I); };
    }

    std::vector<object> args_;
    computelet stored_computelet_;
};

template<typename T, long int Rank>
class tensor
{
public:
    using value_type = T;

    template<typename... SizeTypes>
    explicit tensor(SizeTypes... sizes_)
            : data_(get_runtime().get_object_factory().create_array(associated_qubus_type<T>::get(),
                                                                    {util::to_uindex(sizes_)...}))
    {
    }

    tensor<T, Rank> &operator=(const tensor_expr<T, Rank> &tensor_expr)
    {
        const auto &args = tensor_expr.args();

        execution_context ctx;

        for (const auto &arg : args)
        {
            ctx.push_back_arg(arg);
        }

        ctx.push_back_result(get_object());

        auto c = tensor_expr.stored_computelet();

        get_runtime().execute(c, std::move(ctx)).get();

        return *this;
    }

    template<typename... Indices>
    auto operator()(Indices... indices) const
    {
        static_assert(are_all_indices<Indices...>(), "Expecting indices.");

        return subscripted_tensor<tensor<T, Rank>, Indices...>(*this, indices...);
    }

    hpx::future<void> when_ready()
    {
        return data_.acquire_read_access();
    }

    object get_object() const
    {
        return data_;
    }

private:
    object data_;
};

template<typename View, typename T, long int Rank>
auto get_view(const tensor<T, Rank> &value)
{
    return get_view<View>(value.get_object());
}
}

template<typename T, long int Rank>
using tensor_expr = ast::tensor_expr<T, Rank>;

template<typename T, long int Rank>
using tensor = ast::tensor<T, Rank>;

using ast::get_view;

}
}
}

#endif