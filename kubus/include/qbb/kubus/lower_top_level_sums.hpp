#ifndef QBB_KUBUS_LOWER_TOP_LEVEL_SUMS_HPP
#define QBB_KUBUS_LOWER_TOP_LEVEL_SUMS_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/function_declaration.hpp>

namespace qbb
{
namespace kubus
{
 
expression lower_top_level_sums(const expression& expr);
function_declaration lower_top_level_sums(function_declaration decl);
    
}
}

#endif