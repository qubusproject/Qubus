#ifndef QBB_QUBUS_EXTRACT_HPP
#define QBB_QUBUS_EXTRACT_HPP

#include <qbb/qubus/IR/expression.hpp>

#include <string>
#include <memory>

inline namespace qbb
{
namespace qubus
{

std::unique_ptr<expression> extract_expr_as_function(const expression& expr, const std::string& extracted_func_name);

}
}

#endif
