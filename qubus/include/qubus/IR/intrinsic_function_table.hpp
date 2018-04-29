#ifndef QUBUS_INTRINSIC_FUNCTION_TABLE_HPP
#define QUBUS_INTRINSIC_FUNCTION_TABLE_HPP

#include <qubus/IR/type.hpp>

#include <string>
#include <vector>

namespace qubus
{

type lookup_intrinsic_result_type(const std::string& name, const std::vector<type>& arg_types);
    
}

#endif