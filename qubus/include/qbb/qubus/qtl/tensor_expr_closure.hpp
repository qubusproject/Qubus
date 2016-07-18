#ifndef QBB_QUBUS_TENSOR_EXPR_CLOSURE_HPP
#define QBB_QUBUS_TENSOR_EXPR_CLOSURE_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <boost/optional.hpp>

#include <utility>
#include <vector>

namespace qbb
{
namespace qubus
{
namespace qtl
{

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

struct tensor_expr_closure
{
    tensor_expr_closure(std::vector<index_info> free_indices,
                        std::vector<variable_declaration> params, std::unique_ptr<expression> rhs)
    : free_indices(std::move(free_indices)), params(std::move(params)), rhs(std::move(rhs))
    {
    }

    std::vector<index_info> free_indices;
    std::vector<variable_declaration> params;
    std::unique_ptr<expression> rhs;
};
}
}
}

#endif