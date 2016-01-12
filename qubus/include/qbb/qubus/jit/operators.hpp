#ifndef QBB_QUBUS_JIT_OPERATORS_HPP
#define QBB_QUBUS_JIT_OPERATORS_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/binary_operator_expr.hpp>
#include <qbb/qubus/IR/unary_operator_expr.hpp>
#include <qbb/qubus/IR/expression.hpp>

#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compilation_context.hpp>
#include <qbb/qubus/jit/reference.hpp>

namespace qbb
{
namespace qubus
{
namespace jit
{

class compiler;

reference emit_binary_operator(binary_op_tag tag, const expression& left, const expression& right, compiler& comp);

reference emit_unary_operator(unary_op_tag tag, const expression& arg, compiler& comp);

}
}
}

#endif
