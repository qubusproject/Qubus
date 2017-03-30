#ifndef QUBUS_QTL_LEGALIZATION_PASS_HPP
#define QUBUS_QTL_LEGALIZATION_PASS_HPP

#include <qubus/IR/expression.hpp>

#include <memory>

namespace qubus
{
namespace qtl
{

std::unique_ptr<expression> legalize_expression(const expression& expr);

}
}

#endif
