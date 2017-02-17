#ifndef QUBUS_SPARSE_PATTERNS_HPP
#define QUBUS_SPARSE_PATTERNS_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function_declaration.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> optimize_sparse_patterns(const expression& expr);
function_declaration optimize_sparse_patterns(function_declaration decl);

}
}

#endif
