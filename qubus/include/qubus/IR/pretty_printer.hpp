#ifndef QUBUS_PRETTY_PRINTER_HPP
#define QUBUS_PRETTY_PRINTER_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/function_declaration.hpp>
#include <qubus/IR/type.hpp>

namespace qubus
{

void pretty_print(const expression& expr, bool print_types = false);
void pretty_print(const function_declaration& decl, bool print_types = false);
void pretty_print_type(const type& t);
}

#endif