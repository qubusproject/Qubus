#ifndef QUBUS_JIT_OPERATORS_HPP
#define QUBUS_JIT_OPERATORS_HPP

#include <hpx/config.hpp>

#include <qubus/IR/binary_operator_expr.hpp>
#include <qubus/IR/unary_operator_expr.hpp>
#include <qubus/IR/expression.hpp>

#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/reference.hpp>

namespace qubus
{
namespace jit
{

class compiler;

reference emit_binary_operator(binary_op_tag tag, const expression& left, const expression& right, compiler& comp);

reference emit_unary_operator(unary_op_tag tag, const expression& arg, compiler& comp);

}
}

#endif
