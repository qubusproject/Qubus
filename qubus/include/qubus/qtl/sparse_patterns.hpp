#ifndef QUBUS_SPARSE_PATTERNS_HPP
#define QUBUS_SPARSE_PATTERNS_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> optimize_sparse_patterns(const expression& expr);

}
}

#endif
