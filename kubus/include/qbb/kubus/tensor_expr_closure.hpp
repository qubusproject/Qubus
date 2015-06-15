#ifndef QBB_KUBUS_TENSOR_EXPR_CLOSURE_HPP
#define QBB_KUBUS_TENSOR_EXPR_CLOSURE_HPP

#include <qbb/kubus/IR/variable_declaration.hpp>
#include <qbb/kubus/IR/expression.hpp>

#include <vector>
#include <utility>

namespace qbb
{
namespace kubus
{
struct tensor_expr_closure
{
    tensor_expr_closure(std::vector<variable_declaration> free_indices, std::vector<variable_declaration> params, expression rhs)
    : free_indices(std::move(free_indices)), params(std::move(params)), rhs(std::move(rhs))
    {
    }
    
    std::vector<variable_declaration> free_indices;
    std::vector<variable_declaration> params;
    expression rhs;
};
}
}

#endif