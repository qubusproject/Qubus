#include <qbb/kubus/backends/llvm_environment.hpp>

#include <qbb/kubus/pattern/type.hpp>
#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/IR.hpp>

#include <qbb/util/make_unique.hpp>
#include <qbb/util/integers.hpp>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Linker/Linker.h>

#include <boost/optional.hpp>

#include <tuple>

namespace qbb
{
namespace kubus
{

namespace
{
std::string mangle_type(const type& t)
{
    pattern::variable<type> subtype;

    auto m = pattern::make_matcher<type, std::string>()
                 .case_(pattern::integer_t,
                        [&]
                        {
                            return "integer";
                        })
                 .case_(pattern::double_t,
                        [&]
                        {
                            return "double";
                        })
                 .case_(pattern::float_t,
                        [&]
                        {
                            return "float";
                        })
                 .case_(complex_t(subtype),
                        [&]
                        {
                            return "complex_" + mangle_type(subtype.get());
                        })
                 .case_(tensor_t(subtype), [&]
                        {
                            return "tensor_" + mangle_type(subtype.get());
                        });

    return pattern::match(t, m);
}
}

llvm_environment::llvm_environment()
: builder_(llvm::getGlobalContext()), md_builder_(llvm::getGlobalContext()),
  the_module_(util::make_unique<llvm::Module>("kubus module", llvm::getGlobalContext())),
  tbaa_root_(md_builder_.createTBAARoot("kubus TBAA root"))
{
    llvm::FastMathFlags fast_math_flags;
    fast_math_flags.setUnsafeAlgebra();

    builder_.SetFastMathFlags(fast_math_flags);

    local_tbaa_node_ = md_builder_.createTBAANode("local", tbaa_root_);
    param_tbaa_node_ = md_builder_.createTBAANode("param", tbaa_root_);

    std::vector<llvm::Type*> assume_params = {llvm::Type::getInt1Ty(llvm::getGlobalContext())};
    
    llvm::FunctionType* assume_FT =
        llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()),
                                assume_params, false);

    auto assume = module().getOrInsertFunction("llvm.assume", assume_FT);

    auto int_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());

    std::vector<llvm::Type*> params = {llvm::Type::getDoublePtrTy(llvm::getGlobalContext()), int_type};
    
    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getDoublePtrTy(llvm::getGlobalContext()),
        params, false);

    assume_align_ =
        llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "assume_align", &module());

    assume_align_->addFnAttr(llvm::Attribute::AlwaysInline);

    assume_align_->setDoesNotAlias(1);

    set_current_function(assume_align_);

    llvm::BasicBlock* BB =
        llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", assume_align_);
    builder().SetInsertPoint(BB);

    auto ptrint = builder().CreatePtrToInt(&assume_align_->getArgumentList().front(), int_type);
    auto lhs = builder().CreateAnd(ptrint, llvm::ConstantInt::get(int_type, 31));
    auto cond = builder().CreateICmpEQ(lhs, llvm::ConstantInt::get(int_type, 0));

    builder().CreateCall(assume, cond);

    builder().CreateRet(&assume_align_->getArgumentList().front());
}

llvm::IRBuilder<>& llvm_environment::builder()
{
    return builder_;
}

llvm::Module& llvm_environment::module()
{
    return *the_module_;
}

const llvm::Module& llvm_environment::module() const
{
    return *the_module_;
}

llvm::Type* llvm_environment::map_kubus_type(const type& t) const
{
    llvm::Type*& llvm_type = type_map_[t];

    if (llvm_type)
    {
        return llvm_type;
    }
    else
    {
        pattern::variable<type> subtype;
        pattern::variable<type> total_type;

        auto m =
            pattern::make_matcher<type, llvm::Type*>()
                .case_(pattern::integer_t,
                       [&]
                       {
                           return llvm::IntegerType::get(llvm::getGlobalContext(),
                                                         8 * sizeof(std::size_t));
                       })
                .case_(pattern::double_t,
                       [&]
                       {
                           return llvm::Type::getDoubleTy(llvm::getGlobalContext());
                       })
                .case_(pattern::float_t,
                       [&]
                       {
                           return llvm::Type::getFloatTy(llvm::getGlobalContext());
                       })
                .case_(bind_to(complex_t(subtype), total_type),
                       [&]
                       {
                           llvm::Type* real_type = map_kubus_type(types::double_());

                           llvm::Type* real_pair = llvm::ArrayType::get(real_type, 2);

                           return llvm::StructType::create({real_pair},
                                                           mangle_type(total_type.get()));
                       })
                .case_(bind_to(tensor_t(subtype), total_type), [&]
                       {
                           llvm::Type* size_type = map_kubus_type(types::integer());
                           std::vector<llvm::Type*> types{
                               llvm::PointerType::get(map_kubus_type(subtype.get()), 0),
                               llvm::PointerType::get(size_type, 0)};
                           return llvm::StructType::create(types, mangle_type(total_type.get()));
                       });

        llvm_type = pattern::match(t, m);

        return llvm_type;
    }
}

llvm::MDNode* llvm_environment::get_tbaa_node(const access_path& path) const
{
    auto name = path.str();

    auto& node = tbaa_table_[name];

    if (!node)
    {
        llvm::MDNode* tbaa_metadata = md_builder_.createTBAANode(name, tbaa_root_);

        node = tbaa_metadata;
    }

    return node;
}

llvm::MDNode* llvm_environment::local_tbaa_node() const
{
    return local_tbaa_node_;
}

llvm::MDNode* llvm_environment::param_tbaa_node() const
{
    return param_tbaa_node_;
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
