#include <qbb/qubus/jit/llvm_environment.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/make_unique.hpp>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Linker/Linker.h>

#include <boost/optional.hpp>

#include <array>
#include <tuple>

inline namespace qbb
{
namespace qubus
{
namespace jit
{
namespace
{
std::string mangle_type(const type& t)
{
    using pattern::_;

    pattern::variable<type> subtype;
    pattern::variable<util::index_t> rank;

    auto m = pattern::make_matcher<type, std::string>()
                 .case_(pattern::integer_t, [&] { return "integer"; })
                 .case_(pattern::bool_t, [&] { return "bool"; })
                 .case_(pattern::double_t, [&] { return "double"; })
                 .case_(pattern::float_t, [&] { return "float"; })
                 .case_(complex_t(subtype), [&] { return "complex_" + mangle_type(subtype.get()); })
                 .case_(array_t(subtype, rank), [&] { return "array_" + mangle_type(subtype.get()) + "_" + std::to_string(rank.get()); })
                 .case_(array_slice_t(subtype, rank),
                        [&] { return "array_slice_" + mangle_type(subtype.get()) + "_" + std::to_string(rank.get()); })
                 .case_(struct_t(_, _), [&](const type& self) {
                     const auto& self_ = self.as<types::struct_>();

                     return self_.id();
                 });

    return pattern::match(t, m);
}
}

llvm_environment::llvm_environment(llvm::LLVMContext& ctx_)
: ctx_(&ctx_), builder_(ctx()), md_builder_(ctx()),
  the_module_(util::make_unique<llvm::Module>("Qubus module", ctx()))
{
    llvm::FastMathFlags fast_math_flags;
    fast_math_flags.setUnsafeAlgebra();

    builder_.setFastMathFlags(fast_math_flags);

    global_alias_domain_ = md_builder_.createAliasScopeDomain("qubus.alias_domain");
    get_alias_scope(access_path());

    init_assume_align();
    init_alloc_scratch_mem();
    init_dealloc_scratch_mem();
}

void llvm_environment::init_assume_align()
{
    std::vector<llvm::Type*> assume_params = {llvm::Type::getInt1Ty(ctx())};

    llvm::FunctionType* assume_FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(ctx()), assume_params, false);

    auto assume = module().getOrInsertFunction("llvm.assume", assume_FT);

    auto int_type = llvm::Type::getInt64Ty(ctx());

    std::vector<llvm::Type*> params = {llvm::Type::getInt8PtrTy(ctx()), int_type};

    llvm::FunctionType* FT =
        llvm::FunctionType::get(llvm::Type::getDoublePtrTy(ctx()), params, false);

    assume_align_ =
        llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "assume_align", &module());

    assume_align_->addFnAttr(llvm::Attribute::AlwaysInline);

    assume_align_->setDoesNotAlias(1);

    set_current_function(assume_align_);

    llvm::BasicBlock* BB = llvm::BasicBlock::Create(ctx(), "entry", assume_align_);
    builder().SetInsertPoint(BB);

    auto ptrint = builder().CreatePtrToInt(&assume_align_->getArgumentList().front(), int_type);
    auto lhs = builder().CreateAnd(ptrint, llvm::ConstantInt::get(int_type, 31));
    auto cond = builder().CreateICmpEQ(lhs, llvm::ConstantInt::get(int_type, 0));

    builder().CreateCall(assume, cond);

    builder().CreateRet(&assume_align_->getArgumentList().front());
}

void llvm_environment::init_alloc_scratch_mem()
{
    auto generic_ptr = llvm::Type::getInt8PtrTy(ctx(), 0);
    auto size_type = map_qubus_type(types::integer());

    std::vector<llvm::Type*> param_types = {generic_ptr, size_type};

    llvm::FunctionType* FT = llvm::FunctionType::get(generic_ptr, param_types, false);

    alloc_scratch_mem_ = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                "qbb_qubus_cpurt_alloc_scatch_mem", &module());

    alloc_scratch_mem_->setDoesNotCapture(1);
    alloc_scratch_mem_->setDoesNotAlias(1);
    alloc_scratch_mem_->setDoesNotThrow();
}

void llvm_environment::init_dealloc_scratch_mem()
{
    auto generic_ptr = llvm::Type::getInt8PtrTy(ctx(), 0);
    auto void_type = llvm::Type::getVoidTy(ctx());
    auto size_type = map_qubus_type(types::integer());

    std::vector<llvm::Type*> param_types = {generic_ptr, size_type};

    llvm::FunctionType* FT = llvm::FunctionType::get(void_type, param_types, false);

    dealloc_scratch_mem_ = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                                  "qbb_qubus_cpurt_dealloc_scratch_mem", &module());

    dealloc_scratch_mem_->setDoesNotCapture(1);
    dealloc_scratch_mem_->setDoesNotAlias(1);
    dealloc_scratch_mem_->setDoesNotThrow();
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

llvm::Module& llvm_environment::module()
{
    return *the_module_;
}

const llvm::Module& llvm_environment::module() const
{
    return *the_module_;
}

llvm::Type* llvm_environment::map_qubus_type(const type& t) const
{
    using pattern::_;

    llvm::Type*& llvm_type = type_map_[t];

    if (llvm_type)
    {
        return llvm_type;
    }
    else
    {
        pattern::variable<type> subtype;
        pattern::variable<util::index_t> rank;

        auto m =
            pattern::make_matcher<type, llvm::Type*>()
                .case_(pattern::integer_t,
                       [&] { return llvm::IntegerType::get(ctx(), 8 * sizeof(std::size_t)); })
                .case_(pattern::bool_t, [&] { return llvm::Type::getInt1Ty(ctx()); })
                .case_(pattern::double_t, [&] { return llvm::Type::getDoubleTy(ctx()); })
                .case_(pattern::float_t, [&] { return llvm::Type::getFloatTy(ctx()); })
                .case_(complex_t(subtype),
                       [&](const type& total_type) {
                           llvm::Type* real_type = map_qubus_type(subtype.get());

                           llvm::Type* real_pair = llvm::ArrayType::get(real_type, 2);

                           return llvm::StructType::create({real_pair}, mangle_type(total_type));
                       })
                .case_(array_t(subtype, _),
                       [&](const type& total_type) {
                           llvm::Type* size_type = map_qubus_type(types::integer());
                           std::vector<llvm::Type*> types{
                               llvm::PointerType::get(map_qubus_type(subtype.get()), 0),
                               llvm::PointerType::get(size_type, 0)};
                           return llvm::StructType::create(types, mangle_type(total_type));
                       })
                .case_(array_slice_t(subtype, rank),
                       [&](const type& total_type) {
                           llvm::Type* size_type = map_qubus_type(types::integer());
                           std::vector<llvm::Type*> types{
                               llvm::PointerType::get(map_qubus_type(subtype.get()), 0),
                               llvm::PointerType::get(size_type, 0),
                               llvm::ArrayType::get(size_type, rank.get()),
                               llvm::ArrayType::get(size_type, rank.get()),
                               llvm::ArrayType::get(size_type, rank.get())};
                           return llvm::StructType::create(types, mangle_type(total_type));
                       })
                .case_(struct_t(_, _), [&](const type& self) {
                    const auto& self_ = self.as<types::struct_>();

                    auto generic_ptr_type = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx()), 0);

                    auto member_table_type =
                        llvm::ArrayType::get(generic_ptr_type, self_.member_count());

                    return llvm::StructType::create({member_table_type}, self_.id());
                });

        llvm_type = pattern::match(t, m);

        return llvm_type;
    }
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

llvm::Function* llvm_environment::get_assume_align() const
{
    return assume_align_;
}

llvm::Function* llvm_environment::get_alloc_scratch_mem() const
{
    return alloc_scratch_mem_;
}

llvm::Function* llvm_environment::get_dealloc_scratch_mem() const
{
    return dealloc_scratch_mem_;
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

/*llvm::Value* llvm_environment::lookup_intrinsic_function(const std::string& name,
                                       const std::vector<ir::type>& arg_types)const
{
    return instrinsic_lookup_table_.lookup(name,arg_types);
}*/

std::unique_ptr<llvm::Module> llvm_environment::detach_module()
{
    return std::move(the_module_);
}
}
}
}
