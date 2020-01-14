#include <qubus/jit/host_interop.hpp>

namespace qubus::jit
{
namespace
{
std::string generate_ffi_wrapper_for_constructor(runtime_function& constructor,
                                                 runtime_library& rt_library,
                                                 compiler& compiler)
{
    const std::string& mangled_name = constructor.name;
    const std::vector<type>& param_types = constructor.parameter_types;

    auto mangled_foreign_name = mangled_name + "_ffi";

    llvm::LLVMContext& ctx = compiler.get_context();
    auto& mod = compiler.get_module().llvm_module();

    llvm::IRBuilder<> builder(ctx);

    auto generic_ptr_type = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);

    std::vector<llvm::Type*> wrapper_param_types;
    wrapper_param_types.reserve(3);

    wrapper_param_types.push_back(generic_ptr_type);
    wrapper_param_types.push_back(generic_ptr_type->getPointerTo(0));
    wrapper_param_types.push_back(generic_ptr_type);

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), wrapper_param_types, false);

    auto wrapper =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, mangled_foreign_name, &mod);

    for (std::size_t i = 0; i < wrapper->arg_size(); ++i)
    {
        wrapper->addAttribute(i + 1, llvm::Attribute::AttrKind::NoAlias);
    }

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ctx, "entry", wrapper);
    builder.SetInsertPoint(BB);

    int offset = 0;

    std::vector<llvm::Value*> args;
    args.reserve(param_types.size() + 2);

    args.push_back(&*wrapper->arg_begin());

    for (const auto& type : param_types)
    {
        llvm::Type* param_type = rt_library.map_qubus_type(type);

        llvm::Value* ptr_to_arg =
            builder.CreateConstInBoundsGEP1_64(&*(wrapper->arg_begin() + 1), offset);

        auto arg = builder.CreateLoad(ptr_to_arg);

        llvm::Value* typed_arg =
            builder.CreatePointerCast(arg, llvm::PointerType::get(param_type, 0));

        args.push_back(typed_arg);

        ++offset;
    }

    args.push_back(&*(wrapper->arg_begin() + 2));

    builder.CreateCall(constructor.implementation, args);

    builder.CreateRetVoid();

    return mangled_foreign_name;
}
}

}