#ifndef QBB_KUBUS_PRETTY_PRINTER_HPP
#define QBB_KUBUS_PRETTY_PRINTER_HPP

#include <qbb/kubus/IR/expression.hpp>
#include <qbb/kubus/IR/function_declaration.hpp>

namespace qbb
{
namespace qubus
{

void pretty_print(const expression& expr, bool print_types = false);
void pretty_print(const function_declaration& decl, bool print_types = false);
    
}
}

#endif