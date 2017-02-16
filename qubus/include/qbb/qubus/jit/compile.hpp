#ifndef QBB_QUBUS_JIT_COMPILE_HPP
#define QBB_QUBUS_JIT_COMPILE_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>

#include <qbb/qubus/jit/reference.hpp>
#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compilation_context.hpp>

namespace qubus
{
namespace jit
{

class compiler;

reference compile(const expression& expr, compiler& comp);

}
}

#endif
