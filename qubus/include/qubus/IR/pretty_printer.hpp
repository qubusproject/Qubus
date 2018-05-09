#ifndef QUBUS_PRETTY_PRINTER_HPP
#define QUBUS_PRETTY_PRINTER_HPP

#include <hpx/config.hpp>

#include <qubus/exception.hpp>

#include <carrot/carrot.hpp>

#include <stdexcept>

namespace qubus
{

class type;
class expression;
class function;
class module;

class printing_error : virtual public exception, virtual public std::logic_error
{
public:
    explicit printing_error(const char* what_);
    explicit printing_error(const std::string& what_);
};

//carrot::block pretty_print(const type& t);
carrot::block pretty_print(const expression& expr);
carrot::block pretty_print(const function& decl);
carrot::block pretty_print(const module& mod);
}

#endif