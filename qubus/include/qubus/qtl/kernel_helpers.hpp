#ifndef QUBUS_QTL_KERNEL_HELPERS_HPP
#define QUBUS_QTL_KERNEL_HELPERS_HPP

#include <qubus/qtl/ast.hpp>
#include <qubus/qtl/ast_translation.hpp>
#include <qubus/qtl/index.hpp>

#include <boost/hana/for_each.hpp>
#include <boost/hana/functional/apply.hpp>
#include <boost/hana/functional/curry.hpp>
#include <boost/hana/functional/overload.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/transform.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/unpack.hpp>

#include <qubus/util/function_traits.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace qubus
{
namespace qtl
{
namespace ast
{

template <typename Kernel>
struct get_kernel_arg_type_t
{
    template <typename Index>
    constexpr auto operator()(Index index) const
    {
        return boost::hana::type_c<util::arg_type<Kernel, Index::value>>;
    }
};

template <typename Kernel>
constexpr auto get_kernel_arg_type = get_kernel_arg_type_t<Kernel>{};

struct instantiate_t
{
    template <typename Type>
    auto operator()(Type type) const
    {
        using value_type = typename Type::type;

        return value_type{};
    }
};

constexpr auto instantiate = instantiate_t{};

template <typename Kernel>
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
    translate_kernel_arg(std::vector<index_info>& indices_,
                         std::vector<variable_declaration>& params_, ast_context& ctx_)
    : indices_(indices_), params_(params_), ctx_(ctx_)
    {
    }

    void operator()(const index& idx) const
    {
        variable_declaration index_var{types::index()};

        ctx_.get().symbol_table.add_symbol(idx.id(), index_var);

        indices_.get().push_back(index_info(index_var));
    }

    template <long int Rank>
    void operator()(const multi_index<Rank>& idx) const
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

    template <typename T>
    void operator()(const variable<T>& tensor) const
    {
        variable_declaration tensor_var(associated_qubus_type<T>::get());

        ctx_.get().symbol_table.add_symbol(tensor.id(), tensor_var);

        params_.get().push_back(std::move(tensor_var));
    }

private:
    std::reference_wrapper<std::vector<index_info>> indices_;
    std::reference_wrapper<std::vector<variable_declaration>> params_;
    std::reference_wrapper<ast_context> ctx_;
};

template <typename Kernel>
auto build_kernel(const Kernel& kernel, std::vector<index_info>& indices,
                  std::vector<variable_declaration>& params, std::vector<object>& args)
{
    auto kernel_args = instantiate_kernel_args<Kernel>();

    auto ast = boost::hana::unpack(kernel_args, kernel);

    ast_context ctx;

    boost::hana::for_each(kernel_args, translate_kernel_arg(indices, params, ctx));

    auto ir = translate_ast(ast, ctx);

    for (const auto& variable_object : ctx.object_table)
    {
        params.push_back(variable_object.first);
        args.push_back(variable_object.second);
    }

    return ir;
}
}
}
}

#endif
