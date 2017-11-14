#ifndef QUBUS_JIT_ARRAY_ACCESS_HPP
#define QUBUS_JIT_ARRAY_ACCESS_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/reference.hpp>

#include <llvm/IR/Value.h>

#include <vector>

namespace qubus
{
namespace jit
{

class compiler;

llvm::Value* load_array_data_ptr(reference array, llvm_environment& env, compilation_context& ctx);
llvm::Value* load_array_shape_ptr(reference array, llvm_environment& env, compilation_context& ctx);

reference emit_tensor_access(const expression& tensor,
                             const std::vector<std::reference_wrapper<const expression>>& indices,
                             compiler& comp);

reference emit_array_slice_access(const reference& slice, const std::vector<llvm::Value*>& indices,
                                  compiler& comp);

reference emit_array_slice(const reference& array, const std::vector<llvm::Value*>& offset,
                           const std::vector<llvm::Value*>& shape,
                           const std::vector<llvm::Value*>& strides, compiler& comp);
}
}

#endif
