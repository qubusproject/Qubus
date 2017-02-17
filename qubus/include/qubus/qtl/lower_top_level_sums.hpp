#ifndef QUBUS_LOWER_TOP_LEVEL_SUMS_HPP
#define QUBUS_LOWER_TOP_LEVEL_SUMS_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function_declaration.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_top_level_sums(const expression &expr);

function_declaration lower_top_level_sums(function_declaration decl);

}
}

#endif