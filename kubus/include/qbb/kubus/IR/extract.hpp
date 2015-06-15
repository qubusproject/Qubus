#ifndef QBB_KUBUS_EXTRACT_HPP
#define QBB_KUBUS_EXTRACT_HPP

#include <qbb/kubus/IR/expression.hpp>

#include <string>

namespace qbb
{
namespace kubus
{

expression extract_expr_as_function(expression expr, const std::string& extracted_func_name);

}
}

#endif
