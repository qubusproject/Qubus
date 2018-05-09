#ifndef QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP
#define QUBUS_KRONECKER_DELTA_FOLDING_PASS_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>

#include <memory>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> fold_kronecker_deltas(const expression& expr);

}
}

#endif