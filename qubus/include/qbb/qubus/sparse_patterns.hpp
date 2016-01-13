#ifndef QBB_QUBUS_SPARSE_PATTERNS_HPP
#define QBB_QUBUS_SPARSE_PATTERNS_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

namespace qbb
{
namespace qubus
{

expression optimize_sparse_patterns(expression expr);
function_declaration optimize_sparse_patterns(function_declaration decl);

}
}

#endif