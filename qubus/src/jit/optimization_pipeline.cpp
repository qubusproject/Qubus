#include <qubus/jit/optimization_pipeline.hpp>

#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Analysis/ScalarEvolutionAliasAnalysis.h>
#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9) || LLVM_VERSION_MAJOR >= 4
#include <llvm/Analysis/CFLAndersAliasAnalysis.h>
#include <llvm/Analysis/CFLSteensAliasAnalysis.h>
#include <llvm/Analysis/GlobalsModRef.h>
#endif
#include <llvm/Analysis/ScopedNoAliasAA.h>
#include <llvm/Analysis/TypeBasedAliasAnalysis.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/Transforms/Scalar.h>
#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9) || LLVM_VERSION_MAJOR >= 4
#include <llvm/Transforms/Scalar/GVN.h>
#endif
#include <llvm/Transforms/IPO.h>
#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9) || LLVM_VERSION_MAJOR >= 4
#include <llvm/Transforms/IPO/FunctionAttrs.h>
#include <llvm/Transforms/Instrumentation.h>
#endif
#include <llvm/Transforms/IPO/ForceFunctionAttrs.h>
#include <llvm/Transforms/IPO/InferFunctionAttrs.h>
#include <llvm/Transforms/Vectorize.h>

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

#if (LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR >= 9) || LLVM_VERSION_MAJOR >= 4

namespace
{
void add_initial_alias_analysis_passes(llvm::legacy::PassManager& manager,
                                       bool use_experimental_passes)
{
    using namespace llvm;

    if (use_experimental_passes)
    {
        // Only one of these passes can be currently activated.
        //manager.add(createCFLSteensAAWrapperPass());
        manager.add(createCFLAndersAAWrapperPass());
    }

    manager.add(createTypeBasedAAWrapperPass());
    manager.add(createScopedNoAliasAAWrapperPass());
}

void add_function_simplification_passes(llvm::legacy::PassManager& manager,
                                        bool use_experimental_passes)
{
    using namespace llvm;

    // Start of function pass.
    // Break up aggregate allocas, using SSAUpdater.
    manager.add(createSROAPass());
    manager.add(createEarlyCSEPass()); // Catch trivial redundancies
    // Speculative execution if the target has divergent branches; otherwise nop.
    manager.add(createSpeculativeExecutionIfHasBranchDivergencePass());
    manager.add(createJumpThreadingPass());              // Thread jumps.
    manager.add(createCorrelatedValuePropagationPass()); // Propagate conditionals
    manager.add(createCFGSimplificationPass());          // Merge & remove BBs
    // Combine silly seq's
    manager.add(createInstructionCombiningPass(true));

    manager.add(createTailCallEliminationPass()); // Eliminate tail calls
    manager.add(createCFGSimplificationPass());   // Merge & remove BBs
    manager.add(createReassociatePass());         // Reassociate expressions
    // Rotate Loop - disable header duplication at -Oz
    manager.add(createLoopRotatePass(-1));
    manager.add(createLICMPass()); // Hoist loop invariants
    manager.add(createLoopUnswitchPass(false));
    manager.add(createCFGSimplificationPass());
    manager.add(createInstructionCombiningPass(true));
    manager.add(createIndVarSimplifyPass()); // Canonicalize indvars
    manager.add(createLoopIdiomPass());      // Recognize idioms like memset.
    manager.add(createLoopDeletionPass());   // Delete dead loops

    if (use_experimental_passes)
    {
        manager.add(createLoopInterchangePass()); // Interchange loops
        manager.add(createCFGSimplificationPass());
    }

    manager.add(createSimpleLoopUnrollPass()); // Unroll small loops

    manager.add(createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds
    manager.add(createGVNPass(false));              // Remove redundancies

    manager.add(createMemCpyOptPass()); // Remove memcpy / form memset
    manager.add(createSCCPPass());      // Constant prop with SCCP

    // Delete dead bit computations (instcombine runs after to fold away the dead
    // computations, and then ADCE will run later to exploit any new DCE
    // opportunities that creates).
    manager.add(createBitTrackingDCEPass()); // Delete dead bit computations

    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    manager.add(createInstructionCombiningPass(true));
    manager.add(createJumpThreadingPass()); // Thread jumps
    manager.add(createCorrelatedValuePropagationPass());
    manager.add(createDeadStoreEliminationPass()); // Delete dead stores
    manager.add(createLICMPass());

    if (use_experimental_passes)
        manager.add(createLoopRerollPass());

    if (use_experimental_passes)
        manager.add(createLoadCombinePass());

    manager.add(createAggressiveDCEPass());     // Delete dead instructions
    manager.add(createCFGSimplificationPass()); // Merge & remove BBs
    // Clean up after everything.
    manager.add(createInstructionCombiningPass(true));
}
}

void setup_optimization_pipeline(llvm::legacy::PassManager& manager, bool optimize, bool vectorize)
{
    using namespace llvm;

    bool use_experimental_passes = true;

    // Allow forcing function attributes as a debugging and tuning aid.
    manager.add(createForceFunctionAttrsLegacyPass());

    add_initial_alias_analysis_passes(manager, use_experimental_passes);

    // Infer attributes about declarations if possible.
    manager.add(createInferFunctionAttrsLegacyPass());

    manager.add(createIPSCCPPass());          // IP SCCP
    manager.add(createGlobalOptimizerPass()); // Optimize out global vars
    // Promote any localized global vars.
    manager.add(createPromoteMemoryToRegisterPass());

    manager.add(createDeadArgEliminationPass()); // Dead argument elimination

    manager.add(createInstructionCombiningPass(true)); // Clean up after IPCP & DAE
    manager.add(createCFGSimplificationPass());        // Clean up after IPCP & DAE

    // Indirect call promotion that promotes intra-module targets only.
    manager.add(createPGOIndirectCallPromotionLegacyPass());

    manager.add(createGlobalsAAWrapperPass());

    manager.add(createPruneEHPass());

    manager.add(createFunctionInliningPass());

    manager.add(createPostOrderFunctionAttrsLegacyPass());

    manager.add(createArgumentPromotionPass()); // Scalarize uninlined fn args

    add_function_simplification_passes(manager, use_experimental_passes);

    // FIXME: This is a HACK! The inliner pass above implicitly creates a CGSCC
    // pass manager that we are specifically trying to avoid. To prevent this
    // we must insert a no-op module pass to reset the pass manager.
    manager.add(createBarrierNoopPass());

    manager.add(createEliminateAvailableExternallyPass());

    manager.add(createReversePostOrderFunctionAttrsPass());

    // Scheduling LoopVersioningLICM when inlining is over, because after that
    // we may see more accurate aliasing. Reason to run this late is that too
    // early versioning may prevent further inlining due to increase of code
    // size. By placing it just after inlining other optimizations which runs
    // later might get benefit of no-alias assumption in clone loop.
    if (use_experimental_passes)
    {
        manager.add(createLoopVersioningLICMPass()); // Do LoopVersioningLICM
        manager.add(createLICMPass());               // Hoist loop invariants
    }

    // We add a fresh GlobalsModRef run at this point. This is particularly
    // useful as the above will have inlined, DCE'ed, and function-attr
    // propagated everything. We should at this point have a reasonably minimal
    // and richly annotated call graph. By computing aliasing and mod/ref
    // information for all local globals here, the late loop passes and notably
    // the vectorizer will be able to use them to help recognize vectorizable
    // memory operations.
    //
    // Note that this relies on a bug in the pass manager which preserves
    // a module analysis into a function pass pipeline (and throughout it) so
    // long as the first function pass doesn't invalidate the module analysis.
    // Thus both Float2Int and LoopRotate have to preserve AliasAnalysis for
    // this to work. Fortunately, it is trivial to preserve AliasAnalysis
    // (doing nothing preserves it as it is required to be conservatively
    // correct in the face of IR changes).
    manager.add(createGlobalsAAWrapperPass());

    // Re-rotate loops in all our loop nests. These may have fallout out of
    // rotated form due to GVN or other transformations, and the vectorizer relies
    // on the rotated form. Disable header duplication at -Oz.
    manager.add(createLoopRotatePass(-1));

    // Distribute loops to allow partial vectorization.  I.e. isolate dependences
    // into separate loop that would otherwise inhibit vectorization.  This is
    // currently only performed for loops marked with the metadata
    // llvm.loop.distribute=true or when -enable-loop-distribute is specified.
#if LLVM_VERSION_MAJOR < 4
    manager.add(createLoopDistributePass(/*ProcessAllLoopsByDefault=*/false));
#else
    manager.add(createLoopDistributePass());
#endif

    manager.add(createLoopVectorizePass());

    // Eliminate loads by forwarding stores from the previous iteration to loads
    // of the current iteration.
    manager.add(createLoopLoadEliminationPass());

    // FIXME: Because of #pragma vectorize enable, the passes below are always
    // inserted in the pipeline, even when the vectorizer doesn't run (ex. when
    // on -O1 and no #pragma is found). Would be good to have these two passes
    // as function calls, so that we can only pass them when the vectorizer
    // changed the code.
    manager.add(createInstructionCombiningPass(true));

    // At higher optimization levels, try to clean up any runtime overlap and
    // alignment checks inserted by the vectorizer. We want to track correllated
    // runtime checks for two inner loops in the same outer loop, fold any
    // common computations, hoist loop-invariant aspects out of any outer loop,
    // and unswitch the runtime checks if possible. Once hoisted, we may have
    // dead (or speculatable) control flows or more combining opportunities.
    manager.add(createEarlyCSEPass());
    manager.add(createCorrelatedValuePropagationPass());
    manager.add(createInstructionCombiningPass(true));
    manager.add(createLICMPass());
    manager.add(createLoopUnswitchPass(false));
    manager.add(createCFGSimplificationPass());
    manager.add(createInstructionCombiningPass(true));

    manager.add(createSLPVectorizerPass()); // Vectorize parallel scalar chains.
    manager.add(createEarlyCSEPass());

    manager.add(createCFGSimplificationPass());
    manager.add(createInstructionCombiningPass(true));

    manager.add(createLoopUnrollPass()); // Unroll small loops

    // LoopUnroll may generate some redundency to cleanup.
    manager.add(createInstructionCombiningPass(true));

    // Runtime unrolling will introduce runtime check in loop prologue. If the
    // unrolled loop is a inner loop, then the prologue will be inside the
    // outer loop. LICM pass can help to promote the runtime check out if the
    // checked value is loop invariant.
    manager.add(createLICMPass());

    // Get rid of LCSSA nodes.
    manager.add(createInstructionSimplifierPass());

    // After vectorization and unrolling, assume intrinsics may tell us more
    // about pointer alignments.
    manager.add(createAlignmentFromAssumptionsPass());

    // FIXME: We shouldn't bother with this anymore.
    manager.add(createStripDeadPrototypesPass()); // Get rid of dead prototypes

    // GlobalOpt already deletes dead functions and globals, at -O2 try a
    // late pass of GlobalDCE.  It is capable of deleting dead cycles.
    manager.add(createGlobalDCEPass());     // Remove dead fns and globals.
    manager.add(createConstantMergePass()); // Merge dup global constants
}

#else
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

    manager.add(createPostOrderFunctionAttrsPass());

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

        // manager.add(createLoopDistributePass());

        manager.add(createLoopVectorizePass());

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
#endif
}
}
