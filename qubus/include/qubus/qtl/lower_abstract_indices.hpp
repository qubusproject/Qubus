#ifndef QUBUS_LOWER_ABSTRACT_INDICES_HPP
#define QUBUS_LOWER_ABSTRACT_INDICES_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function.hpp>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> lower_abstract_indices(const expression& expr);

}
}


#endif