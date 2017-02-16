#ifndef QUBUS_EXTRACT_HPP
#define QUBUS_EXTRACT_HPP

#include <qbb/qubus/IR/expression.hpp>

#include <string>
#include <memory>

namespace qubus
{

std::unique_ptr<expression> extract_expr_as_function(const expression& expr, const std::string& extracted_func_name);

}

#endif
