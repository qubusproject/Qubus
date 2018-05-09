#ifndef QUBUS_LOWER_TOP_LEVEL_SUMS_HPP
#define QUBUS_LOWER_TOP_LEVEL_SUMS_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_top_level_sums(const expression &expr);

}
}

#endif