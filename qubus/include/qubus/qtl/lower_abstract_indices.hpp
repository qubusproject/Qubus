#ifndef QUBUS_LOWER_ABSTRACT_INDICES_HPP
#define QUBUS_LOWER_ABSTRACT_INDICES_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function_declaration.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_abstract_indices(const expression& expr);
function_declaration lower_abstract_indices(function_declaration decl);

}
}


#endif