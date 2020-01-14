#include <qubus/jit/array_access.hpp>

#include <qubus/jit/compiler.hpp>
#include <qubus/jit/entry_block_alloca.hpp>
#include <qubus/jit/load_store.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/IR/type_inference.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace qubus
{
namespace jit
{
namespace
{

type get_value_type(const type& array_type)
{
    using pattern::_;

    pattern::variable<type> value_type;

    auto m = pattern::make_matcher<type, type>()
                 .case_(array_t(value_type, _), [&] { return value_type.get(); })
                 .case_(array_slice_t(value_type, _), [&] { return value_type.get(); });

    // TODO: Error handling
    return pattern::match(array_type, m);
}

util::index_t get_rank(const type& array_type)
{
    using pattern::_;

    pattern::variable<util::index_t> rank;

    auto m = pattern::make_matcher<type, util::index_t>()
                 .case_(array_t(_, rank), [&] { return rank.get(); })
                 .case_(array_slice_t(_, rank), [&] { return rank.get(); });

    // TODO: Error handling
    return pattern::match(array_type, m);
}

llvm::Value* emit_array_access(llvm::Value* data, const std::vector<llvm::Value*>& shape,
                               const std::vector<llvm::Value*>& indices, llvm_environment& env)
{
    auto& builder = env.builder();

    llvm::Type* size_type = env.map_qubus_type(types::integer());

    llvm::Value* linearized_index = llvm::ConstantInt::get(size_type, 0);

    // for (std::size_t i = indices.size(); i-- > 0;)
    for (std::size_t i = 0; i < indices.size(); i++)
    {
        auto extent = shape[i];

        linearized_index =
            builder.CreateAdd(builder.CreateMul(extent, linearized_index, "idx_mul", true, true),
                              indices[i], "idx_add", true, true);
    }

    auto result = builder.CreateInBoundsGEP(data, linearized_index);

    return result;
}

reference emit_array_access(const reference& data, const reference& shape,
                            const std::vector<llvm::Value*>& indices, llvm_environment& env,
                            compilation_context& ctx)
{
    auto& builder = env.builder();

    std::vector<llvm::Value*> shape_;
    shape_.reserve(indices.size());

    auto shape_value_type = shape.datatype();

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent_ptr = builder.CreateConstInBoundsGEP1_32(env.map_qubus_type(shape_value_type),
                                                             shape.addr(), i);

        auto extent =
            load_from_ref(reference(extent_ptr, shape.origin(), shape_value_type), env, ctx);

        shape_.push_back(extent);
    }

    auto accessed_element = emit_array_access(data.addr(), shape_, indices, env);

    auto data_value_type = data.datatype();

    return reference(accessed_element, data.origin(), data_value_type);
}

std::unique_ptr<expression> reassociate_index_expression(const expression& expr)
{
    using pattern::_;

    util::index_t constant_term = 0;

    std::function<std::unique_ptr<expression>(const expression&)> collect_constant_terms =
        [&](const expression& expr) {
            pattern::variable<const expression&> lhs, rhs;
            pattern::variable<util::index_t> value;

            auto m = pattern::matcher<expression, std::unique_ptr<expression>>()
                         .case_(integer_literal(value) + rhs,
                                [&] {
                                    constant_term += value.get();

                                    return collect_constant_terms(rhs.get());
                                })
                         .case_(lhs + integer_literal(value),
                                [&] {
                                    constant_term += value.get();

                                    return collect_constant_terms(lhs.get());
                                })
                         .case_(lhs + rhs,
                                [&] {
                                    return collect_constant_terms(lhs.get()) +
                                           collect_constant_terms(rhs.get());
                                })
                         .case_(_, [&](const expression& self) { return clone(self); });

            return pattern::match(expr, m);
        };

    auto simplified_expr = collect_constant_terms(expr);

    if (constant_term != 0)
    {
        return std::move(simplified_expr) + qubus::integer_literal(constant_term);
    }
    else
    {
        return simplified_expr;
    }
}

std::vector<llvm::Value*> permute_indices(const std::vector<llvm::Value*>& indices,
                                          const std::vector<util::index_t>& permutation)
{
    std::vector<llvm::Value*> permuted_indices;
    permuted_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        permuted_indices.push_back(indices[permutation[i]]);
    }

    return permuted_indices;
}
} // namespace

llvm::Value* load_rank(reference array, llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    auto int_type = env.map_qubus_type(types::integer{});

    if (auto array_type = array.datatype().try_as<types::array>())
    {
        return llvm::ConstantInt::get(int_type, array_type->rank(), true);
    }
    else
    {
        auto rank_ptr = builder.CreateBitCast(array.addr(), int_type->getPointerTo(0), "rank_ptr");

        auto rank = builder.CreateLoad(rank_ptr, "rank");

        return rank;
    }
}

llvm::Value* load_array_data_ptr(reference array, llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    auto int_type = env.map_qubus_type(types::integer{});

    auto rank = load_rank(array, env, ctx);

    auto offset =
        builder.CreateAdd(rank, llvm::ConstantInt::get(int_type, 1, true), "offset", true, true);

    auto base_ptr = builder.CreateBitCast(array.addr(), int_type->getPointerTo(0), "rank_ptr");

    llvm::Value* data_ptr = builder.CreateInBoundsGEP(int_type, base_ptr, offset, "data_ptr");

    auto val_type = value_type(array.datatype());

    return builder.CreateBitCast(data_ptr, env.map_qubus_type(val_type)->getPointerTo(0));
}

llvm::Value* load_array_shape_ptr(reference array, llvm_environment& env, compilation_context& ctx)
{
    auto& builder = env.builder();

    auto int_type = env.map_qubus_type(types::integer{});

    auto base_ptr = builder.CreateBitCast(array.addr(), int_type->getPointerTo(0), "rank_ptr");

    auto rank = load_rank(array, env, ctx);

    auto offset = llvm::ConstantInt::get(int_type, 1, true);

    llvm::Value* shape_ptr = builder.CreateInBoundsGEP(int_type, base_ptr, offset, "shape_ptr");

    return shape_ptr;
}

reference extent(const expression& array_like, const expression& dim, compiler& comp)
{
    using pattern::_;

    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    auto& builder = env.builder();

    auto array_like_ = comp.compile(array_like);

    auto m = pattern::make_matcher<expression, reference>()
                 .case_(typeof_(array_t(_, _)),
                        [&] {
                            llvm::Value* shape_ptr = load_array_shape_ptr(array_like_, env, ctx);

                            reference dim_ref = comp.compile(dim);

                            auto dim_ = load_from_ref(dim_ref, env, ctx);

                            llvm::Value* extent_ptr =
                                builder.CreateInBoundsGEP(shape_ptr, dim_, "extent_ptr");

                            return reference(extent_ptr, array_like_.origin() / "shape",
                                             types::integer());
                        })
                 .case_(typeof_(array_slice_t(_, _)), [&] {
                     auto slice_ty = env.map_qubus_type(typeof_(array_like));

                     auto zero = builder.getIntN(32, 0);
                     auto two = builder.getIntN(32, 2);
                     auto three = builder.getIntN(32, 3);
                     auto four = builder.getIntN(32, 4);

                     reference dim_ref = comp.compile(dim);

                     auto idx = load_from_ref(dim_ref, env, ctx);

                     std::vector<llvm::Value*> offset_indices = {zero, two, idx};

                     auto offset_ptr = builder.CreateInBoundsGEP(slice_ty, array_like_.addr(),
                                                                 offset_indices, "offset_ptr");
                     auto offset = builder.CreateLoad(offset_ptr);

                     std::vector<llvm::Value*> bounds_indices = {zero, three, idx};

                     auto bound_ptr = builder.CreateInBoundsGEP(slice_ty, array_like_.addr(),
                                                                bounds_indices, "bounds_ptr");
                     auto bound = builder.CreateLoad(bound_ptr);

                     std::vector<llvm::Value*> stride_indices = {zero, four, idx};

                     auto stride_ptr = builder.CreateInBoundsGEP(slice_ty, array_like_.addr(),
                                                                 stride_indices, "strides_ptr");
                     auto stride = builder.CreateLoad(stride_ptr);

                     auto distance =
                         builder.CreateSub(bound, offset, "distance", true, true);

                     auto extent = builder.CreateSDiv(distance, stride, "extent");

                     auto result = create_entry_block_alloca(env.get_current_function(), extent->getType());

                     builder.CreateStore(extent, result);

                     return reference(result, access_path(), types::integer());
                 });

    return pattern::match(array_like, m);
}

namespace
{
reference emit_array_access(const expression& array,
                            const std::vector<std::reference_wrapper<const expression>>& indices,
                            compiler& comp)
{
    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    std::unique_ptr<expression> linearized_index = integer_literal(0);

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        std::vector<std::unique_ptr<expression>> args;
        args.reserve(2);
        args.push_back(clone(array));
        args.push_back(integer_literal(i));

        auto extent = intrinsic_function("extent", std::move(args));

        linearized_index = std::move(extent) * std::move(linearized_index) + clone(indices[i]);
    }

    linearized_index = reassociate_index_expression(*linearized_index);

    auto linearized_index_ = comp.compile(*linearized_index);

    auto& builder = env.builder();

    auto array_ = comp.compile(array);

    auto data = load_array_data_ptr(array_, env, ctx);

    auto data_value_type = get_value_type(array_.datatype());

    auto data_ref = reference(builder.CreateInBoundsGEP(env.map_qubus_type(data_value_type), data,
                                                        load_from_ref(linearized_index_, env, ctx)),
                              array_.origin() / "data", data_value_type);

    return data_ref;
}

reference
emit_array_slice_access(const reference& slice,
                        const std::vector<std::reference_wrapper<const expression>>& indices,
                        compiler& comp)
{
    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    auto& builder = env.builder();

    std::vector<llvm::Value*> indices_;
    indices_.reserve(indices.size());

    for (const auto& index : indices)
    {
        auto index_ref = comp.compile(index);

        auto index_ = load_from_ref(index_ref, env, ctx);

        indices_.push_back(index_);
    }

    auto slice_ty = env.map_qubus_type(slice.datatype());

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(env.map_qubus_type(slice.datatype()),
                                                               slice.addr(), 0, 0, "data_ptr");
    llvm::Value* shape_ptr = builder.CreateConstInBoundsGEP2_32(
        env.map_qubus_type(slice.datatype()), slice.addr(), 0, 1, "shape_ptr");

    auto data_value_type = get_value_type(slice.datatype());
    auto shape_value_type = types::integer();

    auto data = load_from_ref(reference(data_ptr, slice.origin(), data_value_type), env, ctx);
    auto shape = load_from_ref(reference(shape_ptr, slice.origin(), shape_value_type), env, ctx);

    std::vector<llvm::Value*> transformed_indices;
    transformed_indices.reserve(indices_.size());

    auto zero = builder.getIntN(32, 0);
    auto one = builder.getIntN(32, 1);
    auto two = builder.getIntN(32, 2);
    auto three = builder.getIntN(32, 3);
    auto four = builder.getIntN(32, 4);

    auto size_type = env.map_qubus_type(types::integer{});

    for (std::size_t i = 0; i < indices_.size(); ++i)
    {
        auto idx = llvm::ConstantInt::get(size_type, i, true);

        std::vector<llvm::Value*> offset_indices = {zero, two, idx};

        auto offset_ref =
            reference(builder.CreateInBoundsGEP(slice_ty, slice.addr(), offset_indices),
                      slice.origin() / "offset", types::integer{});

        auto offset = load_from_ref(offset_ref, env, ctx);

        std::vector<llvm::Value*> stride_indices = {zero, four, idx};

        auto stride_ref =
            reference(builder.CreateInBoundsGEP(slice_ty, slice.addr(), stride_indices),
                      slice.origin() / "strides", types::integer{});

        auto stride = load_from_ref(stride_ref, env, ctx);

        auto transformed_index = builder.CreateAdd(
            offset, builder.CreateMul(stride, indices_[i], "", true, true), "", true, true);

        transformed_indices.push_back(transformed_index);
    }

    auto accessed_element =
        emit_array_access(reference(data, slice.origin() / "data", data_value_type),
                          reference(shape, slice.origin() / "shape", shape_value_type),
                          transformed_indices, env, ctx);

    return accessed_element;
}

reference
emit_array_like_access(const expression& array_like,
                       const std::vector<std::reference_wrapper<const expression>>& indices,
                       compiler& comp)
{
    using pattern::_;

    auto m = pattern::make_matcher<type, reference>()
                 .case_(pattern::array_t(_, _),
                        [&] { return emit_array_access(array_like, indices, comp); })
                 .case_(pattern::array_slice_t(_, _), [&] {
                     auto array_like_ = comp.compile(array_like);

                     return emit_array_slice_access(array_like_, indices, comp);
                 });

    return pattern::match(typeof_(array_like), m);
}

reference emit_array_slice(const reference& array,
                           const std::vector<std::reference_wrapper<const expression>>& indices,
                           compiler& comp)
{
    using pattern::_;

    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    auto& builder = env.builder();

    auto zero = builder.getIntN(32, 0);
    auto one = builder.getIntN(32, 1);
    auto two = builder.getIntN(32, 2);
    auto three = builder.getIntN(32, 3);
    auto four = builder.getIntN(32, 4);

    auto size_type = env.map_qubus_type(types::integer{});

    auto array_type = array.datatype();

    auto array_ty = env.map_qubus_type(array_type);

    auto value_type = get_value_type(array_type);
    auto rank = get_rank(array_type);

    std::vector<llvm::Value*> offset;
    offset.reserve(rank);

    std::vector<llvm::Value*> bounds;
    bounds.reserve(rank);

    std::vector<llvm::Value*> strides;
    strides.reserve(rank);

    for (util::index_t i = 0; i < rank; ++i)
    {
        pattern::variable<const expression&> a, b, c;
        pattern::variable<util::index_t> value;

        auto m =
            pattern::make_matcher<expression, void>()
                .case_(integer_range(a, b, c),
                       [&] {
                           auto lower_bound = comp.compile(a.get());

                           offset.push_back(load_from_ref(lower_bound, env, ctx));

                           auto bound = comp.compile(b.get());

                           bounds.push_back(load_from_ref(bound, env, ctx));

                           auto stride = comp.compile(c.get());

                           strides.push_back(load_from_ref(stride, env, ctx));
                       })
                .case_(typeof_(pattern::integer_t), [&] {
                    throw compilation_error("Found an integer argument as part of an array slice."
                                            "This is currently not supported.");
                });

        pattern::match(indices[i], m);
    }

    auto m = pattern::make_matcher<type, void>().case_(array_slice_t(_, _), [&] {
        throw compilation_error("Slicing is currently not supported on slices.");
    });

    pattern::try_match(array_type, m);

    auto array_data = load_array_data_ptr(array, env, ctx);
    auto array_shape = load_array_shape_ptr(array, env, ctx);

    auto slice_type = types::array_slice(value_type, rank);

    auto slice_ty = env.map_qubus_type(slice_type);
    auto slice = create_entry_block_alloca(env.get_current_function(), slice_ty);

    auto data_ptr = builder.CreateConstInBoundsGEP2_32(slice_ty, slice, 0, 0, "data_ptr");
    builder.CreateStore(array_data, data_ptr);

    auto original_shape_ptr =
        builder.CreateConstInBoundsGEP2_32(slice_ty, slice, 0, 1, "original_shape_ptr");
    builder.CreateStore(array_shape, original_shape_ptr);

    for (util::index_t i = 0; i < rank; ++i)
    {
        auto idx = llvm::ConstantInt::get(size_type, i, true);

        std::vector<llvm::Value*> offset_indices = {zero, two, idx};

        auto offset_ptr = builder.CreateInBoundsGEP(slice_ty, slice, offset_indices, "offset_ptr");
        builder.CreateStore(offset[i], offset_ptr);

        std::vector<llvm::Value*> bounds_indices = {zero, three, idx};

        auto bounds_ptr = builder.CreateInBoundsGEP(slice_ty, slice, bounds_indices, "bounds_ptr");
        builder.CreateStore(bounds[i], bounds_ptr);

        std::vector<llvm::Value*> stride_indices = {zero, four, idx};

        auto strides_ptr =
            builder.CreateInBoundsGEP(slice_ty, slice, stride_indices, "strides_ptr");
        builder.CreateStore(strides[i], strides_ptr);
    }

    return reference(slice, array.origin(), std::move(slice_type));
}
} // namespace

reference emit_subscription(const expression& array_like,
                            const std::vector<std::reference_wrapper<const expression>>& indices,
                            compiler& comp)
{
    auto& env = comp.env();
    auto& ctx = comp.compiler_ctx();

    using pattern::_;

    auto m =
        pattern::make_matcher<std::vector<std::reference_wrapper<const expression>>, reference>()
            .case_(any_of(typeof_(pattern::integer_range_t)),
                   [&] {
                       auto ref = comp.compile(array_like);

                       return emit_array_slice(ref, indices, comp);
                   })
            .case_(all_of(typeof_(pattern::integer_t)),
                   [&] { return emit_array_like_access(array_like, indices, comp); });

    return pattern::match(indices, m);
}

} // namespace jit
} // namespace qubus