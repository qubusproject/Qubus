#ifndef QBB_QUBUS_LOWER_TOP_LEVEL_SUMS_HPP
#define QBB_QUBUS_LOWER_TOP_LEVEL_SUMS_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>

inline namespace qbb
{
namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_top_level_sums(const expression &expr);

function_declaration lower_top_level_sums(function_declaration decl);

}
}
}

#endif