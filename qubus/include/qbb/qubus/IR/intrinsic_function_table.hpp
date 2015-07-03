#ifndef QBB_QUBUS_INTRINSIC_FUNCTION_TABLE_HPP
#define QBB_QUBUS_INTRINSIC_FUNCTION_TABLE_HPP

#include <qbb/qubus/IR/type.hpp>

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