#ifndef QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP
#define QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> fold_kronecker_deltas(const expression& expr);

function_declaration fold_kronecker_deltas(function_declaration decl);

}
}

#endif