#ifndef QBB_KUBUS_EQUIVALENCE_HPP
#define QBB_KUBUS_EQUIVALENCE_HPP

#include <qbb/kubus/IR/function_declaration.hpp>

namespace qbb
{
namespace kubus
{

bool test_equivalence(const function_declaration& lhs, const function_declaration& rhs);

}
}   

#endif