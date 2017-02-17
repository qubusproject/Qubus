#ifndef QUBUS_JIT_COMPILE_HPP
#define QUBUS_JIT_COMPILE_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>

#include <qubus/jit/reference.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>

namespace qubus
{
namespace jit
{

class compiler;

reference compile(const expression& expr, compiler& comp);

}
}

#endif
