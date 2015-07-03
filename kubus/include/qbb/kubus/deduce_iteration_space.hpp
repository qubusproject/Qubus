#ifndef QBB_KUBUS_DEDUCE_ITERATION_SPACE_HPP
#define QBB_KUBUS_DEDUCE_ITERATION_SPACE_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/variable_declaration.hpp>

#include <array>

namespace qbb
{
namespace qubus
{
    
std::array<expression, 2> deduce_iteration_space(const variable_declaration& index, const expression& expr);
    
}
}

#endif