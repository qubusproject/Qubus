#ifndef QBB_KUBUS_LOWER_ABSTRACT_INDICES_HPP
#define QBB_KUBUS_LOWER_ABSTRACT_INDICES_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/function_declaration.hpp>

namespace qbb
{
namespace kubus
{

expression lower_abstract_indices(const expression& expr);
function_declaration lower_abstract_indices(function_declaration decl);

}   
}


#endif