#ifndef QUBUS_MULTI_INDEX_CONVERSION_HPP
#define QUBUS_MULTI_INDEX_CONVERSION_HPP

#include <qubus/IR/function.hpp>

namespace qubus
{
namespace qtl
{
std::unique_ptr<expression> expand_multi_indices(const expression& expr);
}
}

#endif
