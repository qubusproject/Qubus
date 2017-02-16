#ifndef QUBUS_DEDUCE_ITERATION_SPACE_HPP
#define QUBUS_DEDUCE_ITERATION_SPACE_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <array>

namespace qubus
{
namespace qtl
{

std::array<std::unique_ptr<expression>, 2> deduce_iteration_space(const variable_declaration& index,
                                                                  const expression& expr);
}
}

#endif