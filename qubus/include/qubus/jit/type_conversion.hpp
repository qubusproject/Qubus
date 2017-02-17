#ifndef QUBUS_JIT_TYPE_CONVERSION_HPP
#define QUBUS_JIT_TYPE_CONVERSION_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/type.hpp>

#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/reference.hpp>

namespace qubus
{
namespace jit
{

class compiler;

reference emit_type_conversion(const type& target_type, const expression& arg,
                               compiler& comp);
}
}

#endif
