#ifndef QBB_QUBUS_EXTRACT_HPP
#define QBB_QUBUS_EXTRACT_HPP

#include <qbb/qubus/IR/expression.hpp>

#include <string>

namespace qbb
{
namespace qubus
{

expression extract_expr_as_function(expression expr, const std::string& extracted_func_name);

}
}

#endif