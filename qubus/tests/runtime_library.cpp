#include <qubus/jit/runtime_library.hpp>
#include <qubus/jit/host_interop.hpp>
#include <qubus/jit/jit_engine.hpp>
#include <qubus/jit/compiler.hpp>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JITEventListener.h>

#include <llvm/Support/TargetSelect.h>

#include <gtest/gtest.h>

TEST(runtime_library, array_constructor)
{
    llvm::LLVMContext ctx;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    qubus::jit::compiler comp;

    llvm::EngineBuilder builder;

    auto TM = std::unique_ptr<llvm::TargetMachine>(builder.selectTarget());

    qubus::jit_engine engine(std::move(TM));

    auto constructor_def = comp.env().rt_library().get_array_constructor(qubus::types::double_{}, 2);

    auto constructor = qubus::jit::wrap_constructor(constructor_def, comp, engine);

    std::array<qubus::util::index_t, 2> shape = {10, 42};

    std::vector<void*> args = {&shape[0], &shape[1]};

    std::vector<char> data(100);

    constructor(data.data(), args);

    qubus::util::index_t rank;

    std::memcpy(&rank, data.data(), sizeof(qubus::util::index_t));

    std::array<qubus::util::index_t, 2> shape_test;

    std::memcpy(shape_test.data(), data.data() + sizeof(qubus::util::index_t), 2 * sizeof(qubus::util::index_t));

    ASSERT_EQ(rank, 2);

    ASSERT_EQ(shape_test[0], 10);

    ASSERT_EQ(shape_test[1], 42);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}



