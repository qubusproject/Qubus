#ifndef QUBUS_MAKE_IMPLICIT_CONVERSIONS_EXPLICIT_HPP
#define QUBUS_MAKE_IMPLICIT_CONVERSIONS_EXPLICIT_HPP

#include <qubus/IR/function.hpp>
#include <qubus/IR/module.hpp>
namespace qubus
{

std::unique_ptr<module> make_implicit_conversions_explicit(const module& mod);
    
}

#endif