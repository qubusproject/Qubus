#include <qubus/jit/llvm_environment.hpp>

#include <qubus/jit/compiler.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>
#include <qubus/pattern/type.hpp>

#include <qubus/util/integers.hpp>
#include <qubus/util/make_unique.hpp>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Linker/Linker.h>

#include <boost/optional.hpp>

#include <array>
#include <tuple>

#include <iostream>

namespace qubus
{
namespace jit
{
extern "C" void qubus_log_int(util::index_t value)
{
    std::cout << "Index = " << value << std::endl;
}

extern "C" void qubus_log_double(double value)
{
    std::cout << "Double = " << value << std::endl;
}

extern "C" void qubus_log_ptr(void* value)
{
    std::cout << "Ptr = " << value << std::endl;
}

llvm_environment::llvm_environment(compiler& compiler_)
: ctx_(&compiler_.get_context()),
  builder_(ctx()),
  md_builder_(ctx()),
  rt_library_(compiler_)
{
    llvm::FastMathFlags fast_math_flags;

    fast_math_flags.setAllowReassoc();
    fast_math_flags.setNoNaNs();
    fast_math_flags.setNoInfs();
    fast_math_flags.setNoSignedZeros();
    fast_math_flags.setAllowReciprocal();
    fast_math_flags.setAllowContract(true);

    builder_.setFastMathFlags(fast_math_flags);

    global_alias_domain_ = md_builder_.createAliasScopeDomain("qubus.alias_domain");
    get_alias_scope(access_path());
}

llvm::LLVMContext& llvm_environment::ctx() const
{
    return *ctx_;
}

llvm::IRBuilder<>& llvm_environment::builder()
{
    return builder_;
}

llvm::MDBuilder& llvm_environment::md_builder()
{
    return md_builder_;
}

runtime_library& llvm_environment::rt_library()
{
    return rt_library_;
}

llvm::Type* llvm_environment::map_qubus_type(const type& t) const
{
    return rt_library_.map_qubus_type(t);
}

llvm::MDNode* llvm_environment::get_alias_scope(const access_path& path) const
{
    auto name = path.str();

    auto& alias_scope = alias_scope_table_[name];

    if (!alias_scope)
    {
        llvm::MDNode* new_alias_scope = md_builder_.createAliasScope(name, global_alias_domain_);

        alias_scope = new_alias_scope;
    }

    std::vector<llvm::Metadata*> alias_scopes = {alias_scope};

    return llvm::MDNode::get(ctx(), alias_scopes);
}

llvm::MDNode* llvm_environment::get_noalias_set(const access_path& path) const
{
    std::vector<llvm::Metadata*> alias_scopes;

    for (const auto& entry : alias_scope_table_)
    {
        if (entry.first != path.str())
        {
            alias_scopes.push_back(entry.second);
        }
    }

    return llvm::MDNode::get(ctx(), alias_scopes);
}

llvm::Function* llvm_environment::get_current_function() const
{
    return current_function_;
}

void llvm_environment::set_current_function(llvm::Function* func)
{
    current_function_ = func;
}

llvm::Function* llvm_environment::get_assume_align(llvm::Module& mod)
{
    if (llvm::Function* assume_align = mod.getFunction("assume_align"))
    {
        return assume_align;
    }

    std::vector<llvm::Type*> assume_params = {llvm::Type::getInt1Ty(ctx())};

    llvm::FunctionType* assume_FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()), assume_params, false);

    auto assume = mod.getOrInsertFunction("llvm.assume", assume_FT);

    auto int_type = llvm::Type::getInt64Ty(ctx());

    std::vector<llvm::Type*> params = {llvm::Type::getInt8PtrTy(ctx()), int_type};

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getInt8PtrTy(ctx()), params, false);

    auto assume_align =
        llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "assume_align", &mod);

    assume_align->addFnAttr(llvm::Attribute::AlwaysInline);

    assume_align->addAttribute(1, llvm::Attribute::AttrKind::NoAlias);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ctx(), "entry", assume_align);
    builder().SetInsertPoint(BB);

    auto ptrint = builder().CreatePtrToInt(&*assume_align->arg_begin(), int_type);
    auto lhs = builder().CreateAnd(ptrint, llvm::ConstantInt::get(int_type, 31));
    auto cond = builder().CreateICmpEQ(lhs, llvm::ConstantInt::get(int_type, 0));

    builder().CreateCall(assume, cond);

    builder().CreateRet(&*assume_align->arg_begin());

    return assume_align;
}

llvm::Function* llvm_environment::get_alloc_scratch_mem(llvm::Module& mod)
{
    if (llvm::Function* alloc_scratch_mem = mod.getFunction("QUBUS_cpurt_alloc_scatch_mem"))
    {
        return alloc_scratch_mem;
    }

    auto generic_ptr = llvm::Type::getInt8PtrTy(ctx(), 0);
    auto size_type = map_qubus_type(types::integer());

    std::vector<llvm::Type*> param_types = {generic_ptr, size_type};

    llvm::FunctionType* FT = llvm::FunctionType::get(generic_ptr, param_types, false);

    auto alloc_scratch_mem = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                "QUBUS_cpurt_alloc_scatch_mem", &mod);

    alloc_scratch_mem->addAttribute(1, llvm::Attribute::AttrKind::NoCapture);
    alloc_scratch_mem->addAttribute(1, llvm::Attribute::AttrKind::NoAlias);
    alloc_scratch_mem->setDoesNotThrow();

    return alloc_scratch_mem;
}

llvm::Function* llvm_environment::get_dealloc_scratch_mem(llvm::Module& mod)
{
    if (llvm::Function* dealloc_scratch_mem = mod.getFunction("QUBUS_cpurt_dealloc_scratch_mem"))
    {
        return dealloc_scratch_mem;
    }

    auto generic_ptr = llvm::Type::getInt8PtrTy(ctx(), 0);
    auto void_type = llvm::Type::getVoidTy(ctx());
    auto size_type = map_qubus_type(types::integer());

    std::vector<llvm::Type*> param_types = {generic_ptr, size_type};

    llvm::FunctionType* FT = llvm::FunctionType::get(void_type, param_types, false);

    auto dealloc_scratch_mem = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                  "QUBUS_cpurt_dealloc_scratch_mem", &mod);

    dealloc_scratch_mem->addAttribute(1, llvm::Attribute::AttrKind::NoCapture);
    dealloc_scratch_mem->addAttribute(1, llvm::Attribute::AttrKind::NoAlias);
    dealloc_scratch_mem->setDoesNotThrow();

    return dealloc_scratch_mem;
}

bool llvm_environment::bind_symbol(const std::string& symbol, llvm::Value* value)
{
    return symbol_table_.insert(std::make_pair(symbol, value)).second;
}

bool llvm_environment::unbind_symbol(const std::string& symbol)
{
    return symbol_table_.erase(symbol) > 0;
}

llvm::Value* llvm_environment::lookup_symbol(const std::string& symbol) const
{
    auto symbol_position = symbol_table_.find(symbol);

    if (symbol_position == end(symbol_table_))
        throw 0;

    return symbol_position->second;
}

} // namespace jit
} // namespace qubus
