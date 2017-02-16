#ifndef QUBUS_OPTIMIZATION_PIPELINE_HPP
#define QUBUS_OPTIMIZATION_PIPELINE_HPP

#include <llvm/IR/LegacyPassManager.h>

namespace qubus
{
namespace jit
{
void setup_function_optimization_pipeline(llvm::legacy::FunctionPassManager& manager, bool optimize);
void setup_optimization_pipeline(llvm::legacy::PassManager& manager, bool optimize, bool vectorize);
}
}

#endif
