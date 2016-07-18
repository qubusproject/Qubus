#ifndef QBB_QUBUS_JIT_ARRAY_ACCESS_HPP
#define QBB_QUBUS_JIT_ARRAY_ACCESS_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/IR/expression.hpp>
#include <qbb/qubus/IR/variable_declaration.hpp>

#include <qbb/qubus/jit/llvm_environment.hpp>
#include <qbb/qubus/jit/compilation_context.hpp>
#include <qbb/qubus/jit/reference.hpp>

#include <llvm/IR/Value.h>

#include <vector>

namespace qbb
{
namespace qubus
{
namespace jit
{

class compiler;

reference emit_tensor_access(const variable_declaration& tensor,
                             const std::vector<std::reference_wrapper<const expression>>& indices, compiler& comp);

reference emit_tensor_access(const expression& tensor, const std::vector<std::reference_wrapper<const expression>>& indices,
                             compiler& comp);

reference emit_array_slice_access(const reference& slice, const std::vector<llvm::Value*>& indices,
                                  compiler& comp);
}
}
}

#endif
