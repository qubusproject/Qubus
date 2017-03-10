#ifndef QUBUS_MULTI_INDEX_CONVERSION_HPP
#define QUBUS_MULTI_INDEX_CONVERSION_HPP

#include <qubus/IR/function_declaration.hpp>

namespace qubus
{
namespace qtl
{
std::unique_ptr<expression> expand_multi_indices(const expression& expr);
function_declaration expand_multi_indices(function_declaration decl);
}
}

#endif
