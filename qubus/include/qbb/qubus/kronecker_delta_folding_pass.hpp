#ifndef QBB_QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP
#define QBB_QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

namespace qbb
{
namespace qubus
{
   
expression fold_kronecker_deltas(expression expr);

function_declaration fold_kronecker_deltas(function_declaration decl);

}     
}

#endif