#ifndef QBB_KUBUS_HASH_HPP
#define QBB_KUBUS_HASH_HPP

#include <qbb/kubus/IR/function_declaration.hpp>

#include <cstddef>

namespace qbb
{
namespace kubus
{
std::size_t hash(const function_declaration& decl);
}
}

#endif