#ifndef QBB_QUBUS_JIT_TYPE_CONVERSION_HPP
#define QBB_QUBUS_JIT_TYPE_CONVERSION_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/type.hpp>

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

reference emit_type_conversion(const type& target_type, const expression& arg,
                               compiler& comp);
}
}
}

#endif
