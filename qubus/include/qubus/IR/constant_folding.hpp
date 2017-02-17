#ifndef QUBUS_CONSTANT_FOLDING_HPP
#define QUBUS_CONSTANT_FOLDING_HPP

#include <qubus/IR/expression.hpp>

#include <memory>

namespace qubus
{

std::unique_ptr<expression> fold_constant_expressions(const expression& expr);

}

#endif
