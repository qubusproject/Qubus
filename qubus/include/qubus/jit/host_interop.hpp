#ifndef QUBUS_JIT_HOST_INTEROP_HPP
#define QUBUS_JIT_HOST_INTEROP_HPP

#include <qubus/IR/constructor.hpp>
#include <qubus/IR/function.hpp>
#include <qubus/jit/compiler.hpp>
#include <qubus/jit/jit_engine.hpp>

namespace qubus::jit
{

using host_variable_ref = void*;

class host_callable_constructor
{
public:
    void operator()(void* raw_memory, std::vector<host_variable_ref> arguments) const;
};

class host_callable_function
{
public:
    void operator()(std::vector<host_variable_ref> arguments, std::vector<host_variable_ref> results) const;
};

host_callable_constructor wrap_constructor(const constructor& constr, compiler& comp, jit_engine& engine);

host_callable_function wrap_function(const function& fn, compiler& comp, jit_engine& engine);

}

#endif
