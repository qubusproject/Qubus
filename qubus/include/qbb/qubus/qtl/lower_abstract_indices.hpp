#ifndef QBB_QUBUS_LOWER_ABSTRACT_INDICES_HPP
#define QBB_QUBUS_LOWER_ABSTRACT_INDICES_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

namespace qbb
{
namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_abstract_indices(const expression& expr);
function_declaration lower_abstract_indices(function_declaration decl);

}
}   
}


#endif