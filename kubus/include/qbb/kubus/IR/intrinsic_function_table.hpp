#ifndef QBB_KUBUS_INTRINSIC_FUNCTION_TABLE_HPP
#define QBB_KUBUS_INTRINSIC_FUNCTION_TABLE_HPP

#include <qbb/kubus/IR/type.hpp>

#include <string>
#include <vector>

namespace qbb
{
namespace qubus
{

type lookup_intrinsic_result_type(const std::string& name, const std::vector<type>& arg_types);
    
}
}

#endif