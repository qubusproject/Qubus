#include <qbb/qubus/jit/optimization_pipeline.hpp>

#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Analysis/ScalarEvolutionAliasAnalysis.h>
#include <llvm/Analysis/TypeBasedAliasAnalysis.h>
#include <llvm/Analysis/ScopedNoAliasAA.h>
#include <llvm/Transforms/Scalar.h>
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 9
#   include <llvm/Transforms/Scalar/GVN.h>
#endif
#include <llvm/Transforms/IPO.h>
#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 9
#   include <llvm/Transforms/IPO/FunctionAttrs.h>
#endif
#include <llvm/Transforms/IPO/ForceFunctionAttrs.h>
#include <llvm/Transforms/IPO/InferFunctionAttrs.h>
#include <llvm/Transforms/Vectorize.h>

#include <llvm/Config/llvm-config.h>

namespace qbb
{
namespace qubus
{
namespace jit
{

void setup_function_optimization_pipeline(llvm::legacy::FunctionPassManager& manager, bool optimize)
{
    using namespace llvm;

    if (!optimize)
        return;

    manager.add(createBasicAAWrapperPass());
    manager.add(createSCEVAAWrapperPass());
    manager.add(createTypeBasedAAWrapperPass());
    manager.add(createScopedNoAliasAAWrapperPass());

    manager.add(createCFGSimplificationPass());
    manager.add(createSROAPass());
    manager.add(createEarlyCSEPass());
    manager.add(createLowerExpectIntrinsicPass());
}

void setup_optimization_pipeline(llvm::legacy::PassManager& manager, bool optimize, bool vectorize)
{
    using namespace llvm;

    manager.add(createForceFunctionAttrsLegacyPass());

    if (!optimize)
        return;

    manager.add(createBasicAAWrapperPass());
    manager.add(createSCEVAAWrapperPass());
    manager.add(createTypeBasedAAWrapperPass());
    manager.add(createScopedNoAliasAAWrapperPass());

    manager.add(createInferFunctionAttrsLegacyPass());

    manager.add(createIPSCCPPass());          // IP SCCP
    manager.add(createGlobalOptimizerPass()); // Optimize out global vars

    manager.add(createPromoteMemoryToRegisterPass());

    manager.add(createDeadArgEliminationPass()); // Dead argument elimination

    manager.add(createInstructionCombiningPass()); // Clean up after IPCP & DAE

    manager.add(createCFGSimplificationPass()); // Clean up after IPCP & DAE

    manager.add(createFunctionInliningPass());

#if LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR <= 8
    manager.add(createPostOrderFunctionAttrsPass());
#elif LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR == 9
    manager.add(createPostOrderFunctionAttrsLegacyPass());
#endif

    manager.add(createArgumentPromotionPass()); // Scalarize uninlined fn args

    manager.add(createSROAPass());

    manager.add(createEarlyCSEPass());                   // Catch trivial redundancies
    manager.add(createJumpThreadingPass());              // Thread jumps.
    manager.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
    manager.add(createCFGSimplificationPass());          // Merge & remove BBs
    manager.add(createInstructionCombiningPass());       // Combine silly seq's

    manager.add(createTailCallEliminationPass()); // Eliminate tail calls

    manager.add(createCFGSimplificationPass()); // Merge & remove BBs
    manager.add(createReassociatePass());       // Reassociate expressions
    // Rotate Loop - disable header duplication at -Oz
    manager.add(createLoopRotatePass(-1));
    manager.add(createLICMPass()); // Hoist loop invariants
    manager.add(createLoopUnswitchPass(false));
    manager.add(createInstructionCombiningPass());
    manager.add(createIndVarSimplifyPass()); // Canonicalize indvars
    // manager.add(createLoopIdiomPass());             // Recognize idioms like memset.
    manager.add(createLoopDeletionPass()); // Delete dead loops

    // manager.add(createLoopInterchangePass());

    manager.add(createMergedLoadStoreMotionPass());
    manager.add(createGVNPass(false));

    // manager.add(createMemCpyOptPass());
    manager.add(createSCCPPass()); // Constant prop with SCCP

    manager.add(createInstructionCombiningPass());

    manager.add(createJumpThreadingPass()); // Thread jumps
    manager.add(createCorrelatedValuePropagationPass());
    manager.add(createDeadStoreEliminationPass()); // Delete dead stores
    manager.add(createLICMPass());

    manager.add(createLoadCombinePass());

    manager.add(createAggressiveDCEPass());        // Delete dead instructions
    manager.add(createCFGSimplificationPass());    // Merge & remove BBs
    manager.add(createInstructionCombiningPass()); // Clean up after everything.

    manager.add(createBarrierNoopPass());

    // vectorization

    if (vectorize)
    {
        manager.add(createLoopRotatePass(-1));

        //manager.add(createLoopDistributePass());

        manager.add(createLoopVectorizePass(true, true));

        manager.add(createLoopLoadEliminationPass());

        manager.add(createInstructionCombiningPass());

        manager.add(createEarlyCSEPass());
        manager.add(createCorrelatedValuePropagationPass());
        manager.add(createInstructionCombiningPass());
        manager.add(createLICMPass());
        manager.add(createLoopUnswitchPass(false));
        manager.add(createCFGSimplificationPass());
        manager.add(createInstructionCombiningPass());

        manager.add(createSLPVectorizerPass()); // Vectorize parallel scalar chains.
        manager.add(createEarlyCSEPass());
    }

    // end vectorization

    manager.add(createCFGSimplificationPass());
    manager.add(createInstructionCombiningPass());

    manager.add(createAlignmentFromAssumptionsPass());

    manager.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

    manager.add(createGlobalDCEPass());     // Remove dead fns and globals.
    manager.add(createConstantMergePass()); // Merge dup global constants

    manager.add(createMergeFunctionsPass());
}

}
}
}
