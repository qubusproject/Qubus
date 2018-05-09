#ifndef QUBUS_JIT_ARRAY_ACCESS_HPP
#define QUBUS_JIT_ARRAY_ACCESS_HPP

#include <hpx/config.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/jit/compilation_context.hpp>
#include <qubus/jit/exception.hpp>
#include <qubus/jit/llvm_environment.hpp>
#include <qubus/jit/reference.hpp>

#include <llvm/IR/Value.h>

#include <vector>

namespace qubus
{
namespace jit
{

class compiler;

llvm::Value* load_rank(reference array, llvm_environment& env, compilation_context& ctx);
llvm::Value* load_array_data_ptr(reference array, llvm_environment& env, compilation_context& ctx);
llvm::Value* load_array_shape_ptr(reference array, llvm_environment& env, compilation_context& ctx);

reference extent(const expression& array_like, const expression& dim, compiler& comp);

reference emit_subscription(const expression& array_like,
                            const std::vector<std::reference_wrapper<const expression>>& indices,
                            compiler& comp);
} // namespace jit
} // namespace qubus

#endif
