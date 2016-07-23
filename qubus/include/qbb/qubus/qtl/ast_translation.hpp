#ifndef QUBUS_QTL_AST_TRANSLATION_HPP
#define QUBUS_QTL_AST_TRANSLATION_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/qtl/ast.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <hpx/include/naming.hpp>

#include <boost/hana/for_each.hpp>

#include <qbb/util/unreachable.hpp>

#include <qbb/util/optional_ref.hpp>
#include <qbb/util/unused.hpp>

#include <unordered_map>
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

struct ast_context
{
    class symbol_table_t
    {
    public:
        const variable_declaration& lookup_or_create_symbol(hpx::naming::gid_type id, type var_type)
        {
            auto search_result = table_.find(id);

            if (search_result != table_.end())
            {
                return search_result->second;
            }
            else
            {
                variable_declaration var(std::move(var_type));

                auto result = table_.emplace(std::move(id), std::move(var));

                return result.first->second;
            }
        }

        void add_symbol(hpx::naming::gid_type id, variable_declaration var)
        {
            auto result = table_.emplace(std::move(id), std::move(var));

            if (!result.second)
                throw 0;
        }

        util::optional_ref<const variable_declaration> lookup(const hpx::naming::gid_type& id) const
        {
            auto search_result = table_.find(id);

            if (search_result != table_.end())
            {
                return search_result->second;
            }
            else
            {
                return {};
            }
        }

    private:
        std::unordered_map<hpx::naming::gid_type, variable_declaration> table_;
    };

    symbol_table_t symbol_table;
    std::vector<std::pair<variable_declaration, object>> object_table;
};

template <typename T>
auto translate_ast(literal<T> node, ast_context& QBB_UNUSED(ctx))
{
    return lit(node.value());
}

template <binary_operator_tag Tag, typename LHS, typename RHS>
auto translate_ast(binary_operator<Tag, LHS, RHS> node, ast_context& ctx)
{
    auto lhs = translate_ast(node.lhs(), ctx);
    auto rhs = translate_ast(node.rhs(), ctx);

    switch (node.tag())
    {
    case binary_operator_tag::plus:
        return qbb::qubus::operator+(std::move(lhs), std::move(rhs));
    case binary_operator_tag::minus:
        return qbb::qubus::operator-(std::move(lhs), std::move(rhs));
    case binary_operator_tag::multiplies:
        return qbb::qubus::operator*(std::move(lhs), std::move(rhs));
    case binary_operator_tag::divides:
        return qbb::qubus::operator/(std::move(lhs), std::move(rhs));
    }

    QBB_UNREACHABLE();
}

template <typename Body>
auto translate_ast(contraction<index, Body> node, ast_context& ctx)
{
    variable_declaration idx_var(types::index{});

    ctx.symbol_table.add_symbol(node.contraction_index().id(), idx_var);

    auto body = translate_ast(node.body(), ctx);

    return qbb::qubus::sum(std::move(idx_var), std::move(body));
}

template <long int Rank, typename Body>
auto translate_ast(contraction<multi_index<Rank>, Body> node, ast_context& ctx)
{
    variable_declaration multi_idx_var(types::multi_index{});

    ctx.symbol_table.add_symbol(node.contraction_index().id(), multi_idx_var);

    std::vector<variable_declaration> indices;

    for (long int i = 0; i < node.contraction_index().rank(); ++i)
    {
        variable_declaration idx_var(types::index{});

        indices.push_back(idx_var);

        ctx.symbol_table.add_symbol(node.contraction_index()[i].id(), idx_var);
    }

    auto body = translate_ast(node.body(), ctx);

    return qbb::qubus::sum(std::move(indices), std::move(multi_idx_var), std::move(body));
}

template <typename Tensor, typename... Indices>
auto translate_ast(subscripted_tensor<Tensor, Indices...> node, ast_context& ctx)
{
    auto tensor = translate_ast(node.tensor(), ctx);

    std::vector<std::unique_ptr<expression>> indices;

    boost::hana::for_each(node.indices(), [&indices, &ctx](auto index) {
        indices.push_back(translate_ast(index, ctx));
    });

    return subscription(std::move(tensor), std::move(indices));
}

inline auto translate_ast(const index& idx, ast_context& ctx)
{
    if (auto idx_var = ctx.symbol_table.lookup(idx.id()))
    {
        return var(*idx_var);
    }
    else
    {
        throw 0;
    }
}

template <long int Rank>
auto translate_ast(const multi_index<Rank>& idx, ast_context& ctx)
{
    if (auto idx_var = ctx.symbol_table.lookup(idx.id()))
    {
        return var(*idx_var);
    }
    else
    {
        throw 0;
    }
}

template <typename T>
auto translate_ast(const variable<T>& tensor, ast_context& ctx)
{
    if (auto tensor_var = ctx.symbol_table.lookup(tensor.id()))
    {
        return var(*tensor_var);
    }
    else
    {
        throw 0;
    }
}

template <typename Tensor>
auto translate_ast(const Tensor& tensor, ast_context& ctx)
{
    if (auto tensor_var = ctx.symbol_table.lookup(tensor.get_object().id().get_gid()))
    {
        return var(*tensor_var);
    }
    else
    {
        variable_declaration new_tensor_var(tensor.get_object().object_type());

        ctx.symbol_table.add_symbol(tensor.get_object().id().get_gid(), new_tensor_var);

        ctx.object_table.emplace_back(new_tensor_var, tensor.get_object());

        return var(new_tensor_var);
    }
}

template <typename FirstIndex, typename SecondIndex>
auto translate_ast(kronecker_delta<FirstIndex, SecondIndex> node, ast_context& ctx)
{
    auto first_index = translate_ast(node.first_index(), ctx);
    auto second_index = translate_ast(node.first_index(), ctx);

    return qbb::qubus::kronecker_delta(node.extent(), std::move(first_index), std::move(second_index));
}
}
}
}
}

#endif
