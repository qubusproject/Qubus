#ifndef QBB_QUBUS_PRETTY_PRINTER_HPP
#define QBB_QUBUS_PRETTY_PRINTER_HPP

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/function_declaration.hpp>
#include <qbb/qubus/IR/type.hpp>

namespace qbb
{
namespace qubus
{

void pretty_print(const expression& expr, bool print_types = false);
void pretty_print(const function_declaration& decl, bool print_types = false);
void pretty_print_type(const type& t);
}
}

#endif