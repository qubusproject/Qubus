#include <hpx/config.hpp>

#include <qubus/jit/runtime_library.hpp>

#include <qubus/jit/compiler.hpp>
#include <qubus/jit/jit_engine.hpp>
#include <qubus/jit/optimization_pipeline.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/type.hpp>

#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO/Internalize.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <qubus/util/assert.hpp>

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace qubus::jit
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
                 .case_(pattern::integer_range_t, [&] { return "integer_range"; })
                 .case_(complex_t(subtype), [&] { return "complex_" + mangle_type(subtype.get()); })
                 .case_(array_t(subtype, rank),
                        [&] {
                            return "array_" + mangle_type(subtype.get()) + "_" +
                                   std::to_string(rank.get());
                        })
                 .case_(array_slice_t(subtype, rank),
                        [&] {
                            return "array_slice_" + mangle_type(subtype.get()) + "_" +
                                   std::to_string(rank.get());
                        })
                 .case_(struct_t(_, _), [&](const type& self) {
                     const auto& self_ = self.as<types::struct_>();

                     return self_.id();
                 });

    return pattern::match(t, m);
}

} // namespace

class runtime_library::impl
{
public:
    explicit impl(compiler& compiler_)
    : compiler_(&compiler_)
    {
    }

    llvm::Type* map_qubus_type(const type& t) const
    {
        auto& ctx = compiler().get_context();

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
                           [&] { return llvm::IntegerType::get(ctx, 8 * sizeof(std::size_t)); })
                    .case_(pattern::bool_t, [&] { return llvm::Type::getInt1Ty(ctx); })
                    .case_(pattern::double_t, [&] { return llvm::Type::getDoubleTy(ctx); })
                    .case_(pattern::float_t, [&] { return llvm::Type::getFloatTy(ctx); })
                    .case_(pattern::integer_range_t,
                           [&] {
                               llvm::Type* size_type = map_qubus_type(types::integer());

                               return llvm::StructType::create({size_type, size_type, size_type},
                                                               mangle_type(types::integer_range));
                           })
                    .case_(complex_t(subtype),
                           [&](const type& total_type) {
                               llvm::Type* real_type = map_qubus_type(subtype.get());

                               llvm::Type* real_pair = llvm::ArrayType::get(real_type, 2);

                               return llvm::StructType::create({real_pair},
                                                               mangle_type(total_type));
                           })
                    .case_(array_t(subtype, _),
                           [&](const type& total_type) {
                               return llvm::StructType::create(ctx, mangle_type(total_type));
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

                        auto generic_ptr_type =
                            llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);

                        auto member_table_type =
                            llvm::ArrayType::get(generic_ptr_type, self_.member_count());

                        return llvm::StructType::create({member_table_type}, self_.id());
                    });

            llvm_type = pattern::match(t, m);

            return llvm_type;
        }
    }

    runtime_function get_array_constructor(const type& value_type, util::index_t rank,
        llvm::Module& mod)
    {
        auto& ctx = parent_compiler().get_context();

        std::string mangled_name = "construct_array_";

        mangled_name += mangle_type(value_type);

        mangled_name += '_';

        mangled_name += std::to_string(rank);

        std::vector<type> param_types;

        for (util::index_t i = 0; i < rank; ++i)
        {
            param_types.push_back(types::integer{});
        }

        if (auto constructor = mod.getFunction(mangled_name))
        {
            return runtime_function(std::move(mangled_name), std::move(param_types), constructor);
        }

        llvm::IRBuilder builder(ctx);

        std::vector<llvm::Type*> constructor_args;

        constructor_args.push_back(
            llvm::Type::getInt8PtrTy(ctx)); // Raw memory for the constructed object

        for (util::index_t i = 0; i < rank; ++i)
        {
            constructor_args.push_back(map_qubus_type(param_types[i])->getPointerTo(0));
        }

        constructor_args.push_back(llvm::Type::getInt8PtrTy(ctx));

        auto constructor_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), constructor_args, false);

        auto constructor = llvm::Function::Create(constructor_type, llvm::Function::ExternalLinkage,
                                                  mangled_name, &mod);

        // The data pointer is non-null and at least the metadata accessible.
        constructor->addParamAttr(0, llvm::Attribute::NonNull);
        constructor->addDereferenceableParamAttr(0, sizeof(util::index_t) * (rank + 1));

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", constructor);
        builder.SetInsertPoint(entry);

        auto current_arg = constructor->arg_begin();

        llvm::Value* data = &*current_arg;

        auto metadata = builder.CreatePointerCast(
            data, llvm::Type::getIntNPtrTy(ctx, 8 * sizeof(util::index_t)), "metadata");

        auto rank_ptr = builder.CreateConstInBoundsGEP1_64(metadata, 0, "rank");

        builder.CreateStore(builder.getInt64(rank), rank_ptr);

        ++current_arg;

        auto shape_ptr = builder.CreateConstInBoundsGEP1_64(rank_ptr, 1, "shape");

        for (util::index_t i = 0; i < rank; ++i, ++current_arg)
        {
            auto arg = &*current_arg;

            auto extent = builder.CreateLoad(arg);

            auto extent_ptr = builder.CreateInBoundsGEP(shape_ptr, builder.getInt64(i), "extent");

            builder.CreateStore(extent, extent_ptr);
        }

        builder.CreateRetVoid();

        return runtime_function(std::move(mangled_name), std::move(param_types), constructor);
    }

    runtime_function get_array_sizeof(const type& value_type, util::index_t rank,
                                      llvm::Module& mod)
    {
        auto& ctx = parent_compiler().get_context();

        auto integer_sizeof = parent_compiler().get_sizeof(types::integer{});
        auto value_type_sizeof = parent_compiler().get_sizeof(value_type);

        std::string mangled_name = "sizeof_array_";

        mangled_name += mangle_type(value_type);

        mangled_name += '_';

        mangled_name += std::to_string(rank);

        std::vector<type> param_types;

        for (util::index_t i = 0; i < rank; ++i)
        {
            param_types.push_back(types::integer{});
        }

        if (auto sizeof_fn = mod.getFunction(mangled_name))
        {
            return runtime_function(std::move(mangled_name), std::move(param_types), sizeof_fn);
        }

        llvm::IRBuilder builder(ctx);

        std::vector<llvm::Type*> args;

        for (util::index_t i = 0; i < rank; ++i)
        {
            args.push_back(map_qubus_type(param_types[i])->getPointerTo(0));
        }

        args.push_back(llvm::Type::getInt8PtrTy(ctx));

        auto sizeof_type =
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), args, false);

        auto sizeof_fn = llvm::Function::Create(sizeof_type, llvm::Function::ExternalLinkage,
                                                mangled_name, &mod);

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", sizeof_fn);
        builder.SetInsertPoint(entry);

        auto integer_size = builder.CreateCall(integer_sizeof.addr(), {});
        auto value_type_size = builder.CreateCall(value_type_sizeof.addr(), {});

        auto metadata_size = builder.CreateMul(integer_size, builder.getInt64(rank + 1), "metadata_size", true, true);

        auto current_arg = sizeof_fn->arg_begin();

        llvm::Value* data_size = value_type_size;

        for (util::index_t i = 0; i < rank; ++i, ++current_arg)
        {
            auto arg = &*current_arg;

            auto extent = builder.CreateLoad(arg);

            data_size = builder.CreateMul(data_size, extent, "data_size", true, true);
        }

        auto size = builder.CreateAdd(metadata_size, data_size, "size", true, true);

        builder.CreateRet(size);

        return runtime_function(std::move(mangled_name), std::move(param_types), sizeof_fn);
    }

private:
    compiler& parent_compiler() const
    {
        QUBUS_ASSERT(compiler_ != nullptr, "Invalid object");

        return *compiler_;
    }

    compiler* compiler_;
    mutable std::unordered_map<type, llvm::Type*> type_map_;
};

runtime_library::runtime_library(compiler& compiler_) : impl_(std::make_unique<impl>(compiler_))
{
}

runtime_library::~runtime_library() = default;

llvm::Type* runtime_library::map_qubus_type(const type& t) const
{
    return impl_->map_qubus_type(t);
}

runtime_function runtime_library::get_array_constructor(const type& value_type, util::index_t rank,
                                                        llvm::Module& mod)
{
    return impl_->get_array_constructor(value_type, rank, mod);
}

} // namespace qubus::jit