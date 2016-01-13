#include <qbb/qubus/jit/array_access.hpp>

#include <qbb/qubus/jit/load_store.hpp>
#include <qbb/qubus/jit/compiler.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/IR/type_inference.hpp>

#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace qbb
{
namespace qubus
{
namespace jit
{
namespace
{
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

    return builder.CreateInBoundsGEP(data, linearized_index);
}

reference emit_array_access(const reference& data, const reference& shape,
                            const std::vector<llvm::Value*>& indices, llvm_environment& env,
                            compilation_context& ctx)
{
    auto& builder = env.builder();

    std::vector<llvm::Value*> shape_;
    shape_.reserve(indices.size());

    auto shape_type = shape.datatype().as<types::array>();
    auto shape_value_type = shape_type.value_type();

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent_ptr = builder.CreateConstInBoundsGEP1_32(env.map_qubus_type(shape.datatype()),
                                                             shape.addr(), i);

        auto extent =
            load_from_ref(reference(extent_ptr, shape.origin(), shape_value_type), env, ctx);

        shape_.push_back(extent);
    }

    auto accessed_element = emit_array_access(data.addr(), shape_, indices, env);

    auto data_type = data.datatype().as<types::array>();
    auto data_value_type = data_type.value_type();

    return reference(accessed_element, data.origin(), data_value_type);
}

expression reassociate_index_expression(expression expr)
{
    using pattern::_;

    util::index_t constant_term = 0;

    std::function<expression(expression)> collect_constant_terms = [&](expression expr)
    {
        pattern::variable<expression> lhs, rhs;
        pattern::variable<util::index_t> value;

        auto m = pattern::matcher<expression, expression>()
                     .case_(binary_operator(pattern::value(binary_op_tag::plus),
                                            integer_literal(value), rhs),
                            [&]
                            {
                                constant_term += value.get();

                                return collect_constant_terms(rhs.get());
                            })
                     .case_(binary_operator(pattern::value(binary_op_tag::plus), lhs,
                                            integer_literal(value)),
                            [&]
                            {
                                constant_term += value.get();

                                return collect_constant_terms(lhs.get());
                            })
                     .case_(binary_operator(pattern::value(binary_op_tag::plus), lhs, rhs),
                            [&]
                            {
                                return binary_operator_expr(binary_op_tag::plus,
                                                            collect_constant_terms(lhs.get()),
                                                            collect_constant_terms(rhs.get()));
                            })
                     .case_(_, [&](const expression& self)
                            {
                                return self;
                            });

        return pattern::match(expr, m);
    };

    auto simplified_expr = collect_constant_terms(expr);

    if (constant_term != 0)
    {
        return binary_operator_expr(binary_op_tag::plus, simplified_expr,
                                    integer_literal_expr(constant_term));
    }
    else
    {
        return simplified_expr;
    }
}

type value_type(const type& array_type)
{
    pattern::variable<type> value_type;

    auto m = pattern::make_matcher<type, type>()
                 .case_(array_t(value_type),
                        [&]
                        {
                            return value_type.get();
                        })
                 .case_(tensor_t(value_type),
                        [&]
                        {
                            return value_type.get();
                        })
                 .case_(array_slice_t(value_type), [&]
                        {
                            return value_type.get();
                        });

    // TODO: Error handling
    return pattern::match(array_type, m);
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
}

reference emit_tensor_access(const variable_declaration& tensor,
                             const std::vector<expression>& indices, compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    expression linearized_index = integer_literal_expr(0);

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent =
            intrinsic_function_expr("extent", {variable_ref_expr(tensor), integer_literal_expr(i)});

        linearized_index = binary_operator_expr(
            binary_op_tag::plus,
            binary_operator_expr(binary_op_tag::multiplies, extent, linearized_index), indices[i]);
    }

    linearized_index = reassociate_index_expression(std::move(linearized_index));

    auto linearized_index_ = comp.compile(linearized_index);

    auto tensor_ = ctx.symbol_table().at(tensor.id());

    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(
        env.map_qubus_type(tensor_.datatype()), tensor_.addr(), 0, 0, "data_ptr");

    auto data_value_type = value_type(tensor_.datatype());

    auto data = load_from_ref(reference(data_ptr, tensor_.origin(), types::array(data_value_type)),
                              env, ctx);

    auto data_ref = reference(builder.CreateInBoundsGEP(env.map_qubus_type(data_value_type), data,
                                                        load_from_ref(linearized_index_, env, ctx)),
                              tensor_.origin() / "data", data_value_type);

    // TODO: Reenable LAAA
    /*if (auto code_region = ctx.current_code_region())
    {
        auto alias_info = code_region->register_access(tensor, indices, data_ref);

        data_ref.add_alias_info(alias_info);
    }*/

    return data_ref;
}

reference emit_tensor_access(const expression& tensor, const std::vector<expression>& indices,
                             compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    expression linearized_index = integer_literal_expr(0);

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto extent = intrinsic_function_expr("extent", {tensor, integer_literal_expr(i)});

        linearized_index = binary_operator_expr(
            binary_op_tag::plus,
            binary_operator_expr(binary_op_tag::multiplies, extent, linearized_index), indices[i]);
    }

    linearized_index = reassociate_index_expression(std::move(linearized_index));

    auto linearized_index_ = comp.compile(linearized_index);

    auto tensor_ = comp.compile(tensor);

    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(env.map_qubus_type(typeof_(tensor)),
                                                               tensor_.addr(), 0, 0, "data_ptr");

    auto data_value_type = value_type(tensor_.datatype());

    auto data = load_from_ref(reference(data_ptr, tensor_.origin(), data_value_type), env, ctx);

    auto data_ref = reference(builder.CreateInBoundsGEP(env.map_qubus_type(data_value_type), data,
                                                        load_from_ref(linearized_index_, env, ctx)),
                              tensor_.origin() / "data", data_value_type);

    return data_ref;
}

// TODO: Implement the index expression reassociation optimization for slices.
reference emit_array_slice_access(const reference& slice, const std::vector<llvm::Value*>& indices,
                                  compiler& comp)
{
    auto& env = comp.get_module().env();
    auto& ctx = comp.get_module().ctx();

    auto& builder = env.builder();

    llvm::Value* data_ptr = builder.CreateConstInBoundsGEP2_32(env.map_qubus_type(slice.datatype()),
                                                               slice.addr(), 0, 0, "data_ptr");
    llvm::Value* shape_ptr = builder.CreateConstInBoundsGEP2_32(
        env.map_qubus_type(slice.datatype()), slice.addr(), 0, 1, "shape_ptr");
    llvm::Value* origin_ptr = builder.CreateConstInBoundsGEP2_32(
        env.map_qubus_type(slice.datatype()), slice.addr(), 0, 2, "origin_ptr");

    auto data_value_type = value_type(slice.datatype());
    auto shape_value_type = types::integer();
    auto origin_value_type = types::integer();

    auto shape = load_from_ref(reference(shape_ptr, slice.origin(), data_value_type), env, ctx);
    auto data = load_from_ref(reference(data_ptr, slice.origin(), shape_value_type), env, ctx);
    auto origin = load_from_ref(reference(origin_ptr, slice.origin(), origin_value_type), env, ctx);

    std::vector<llvm::Value*> transformed_indices;
    transformed_indices.reserve(indices.size());

    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        auto origin_component_ptr =
            builder.CreateConstInBoundsGEP1_32(env.map_qubus_type(origin_value_type), origin, i);

        auto origin_component = load_from_ref(
            reference(origin_component_ptr, slice.origin() / "origin", origin_value_type), env,
            ctx);

        auto transformed_index = builder.CreateAdd(origin_component, indices[i], "", true, true);

        transformed_indices.push_back(transformed_index);
    }

    auto accessed_element =
        emit_array_access(reference(data, slice.origin() / "data", data_value_type),
                          reference(shape, slice.origin() / "shape", shape_value_type),
                          transformed_indices, env, ctx);

    return accessed_element;
}
}
}
}