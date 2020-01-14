#include <qubus/IR/type_inference.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/IR/types.hpp>
#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/multi_method.hpp>
#include <qubus/util/unused.hpp>

#include <mutex>

namespace qubus
{

namespace
{

util::multi_method<type(const util::virtual_<expression>&, const assembly&)> infer_type = {};

type infer_type_binary_op_expr(const binary_operator_expr& expr, const assembly& active_assembly)
{
    switch (expr.tag())
    {
    case binary_op_tag::equal_to:
    case binary_op_tag::less_equal:
    case binary_op_tag::less:
    case binary_op_tag::greater_equal:
    case binary_op_tag::greater:
        return active_assembly.get_type_bool();
    default:
    {
        type left_type = typeof_(expr.left());
        type right_type = typeof_(expr.right());

        QUBUS_ASSERT(left_type == right_type,
                     "The type of the left-hand side and the right-hand side must match.");

        return left_type;
    }
    }
}

type infer_type_unary_op_expr(const unary_operator_expr& expr, const assembly& active_assembly)
{
    type arg_type = typeof_(expr.arg());

    return arg_type;
}

type infer_type_for_expr(const for_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_void();
}

type infer_type_if_expr(const if_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_void();
}

type infer_type_subscription_expr(const subscription_expr& expr, const assembly& active_assembly)
{
    auto tensor_type = typeof_(expr.indexed_expr());
    auto val_type = value_type(tensor_type);

    if (!val_type)
    {
        throw 0;
    }

    auto rank = util::to_uindex(expr.indices().size());

    for (const auto& index : expr.indices())
    {
        auto index_type = typeof_(index);

        if (index_type == active_assembly.get_type_int64())
        {
            --rank;
        }
    }

    bool is_a_slice = rank > 0;

    if (is_a_slice)
    {
        return active_assembly.get_type_array_slice(*val_type, rank);
    }

    return *val_type;
}

type infer_type_type_conversion_expr(const type_conversion_expr& expr, const assembly& active_assembly)
{
    return expr.target_type();
}

type infer_type_double_literal_expr(const double_literal_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_float64();
}

type infer_type_float_literal_expr(const float_literal_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_float32();
}

type infer_type_integer_literal_expr(const integer_literal_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_int64();
}

type infer_type_bool_literal_expr(const bool_literal_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_bool();
}

type infer_type_intrinsic_function_expr(const intrinsic_function_expr& expr, const assembly& active_assembly)
{
    std::vector<type> arg_types;

    for (const auto& arg : expr.args())
    {
        arg_types.push_back(typeof_(arg));
    }

    return lookup_intrinsic_result_type(expr.name(), arg_types);
}

type infer_type_compound_expr(const compound_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_void();
}

type infer_type_variable_ref_expr(const variable_ref_expr& expr, const assembly& active_assembly)
{
    return expr.declaration()->var_type();
}

type infer_type_local_variable_def_expr(const local_variable_def_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_void();
}

type infer_type_construct_expr(const construct_expr& expr, const assembly& active_assembly)
{
    return expr.result_type();
}

type infer_type_member_access_expr(const member_access_expr& expr, const assembly& active_assembly)
{
    const auto& obj_type = typeof_(expr.object()).as<object_type>();

    return obj_type[expr.member_name()];
}

type infer_type_integer_range_expr(const integer_range_expr& /*unused*/, const assembly& active_assembly)
{
    return active_assembly.get_type_integer_range(active_assembly.get_type_int64());
}

void init_infer_type()
{
    infer_type.add_specialization(infer_type_binary_op_expr);
    infer_type.add_specialization(infer_type_unary_op_expr);
    infer_type.add_specialization(infer_type_for_expr);
    infer_type.add_specialization(infer_type_if_expr);
    infer_type.add_specialization(infer_type_subscription_expr);
    infer_type.add_specialization(infer_type_type_conversion_expr);
    infer_type.add_specialization(infer_type_double_literal_expr);
    infer_type.add_specialization(infer_type_float_literal_expr);
    infer_type.add_specialization(infer_type_integer_literal_expr);
    infer_type.add_specialization(infer_type_bool_literal_expr);
    infer_type.add_specialization(infer_type_intrinsic_function_expr);
    infer_type.add_specialization(infer_type_compound_expr);
    infer_type.add_specialization(infer_type_variable_ref_expr);
    infer_type.add_specialization(infer_type_local_variable_def_expr);
    infer_type.add_specialization(infer_type_construct_expr);
    infer_type.add_specialization(infer_type_member_access_expr);
    infer_type.add_specialization(infer_type_integer_range_expr);
}

std::once_flag infer_type_init_flag = {};

type typeof_(const expression& expr, const assembly& active_assembly)
{
    if (auto type_annotation = expr.annotations().lookup("type"))
    {
        return type_annotation.as<type>();
    }
    else
    {
        std::call_once(infer_type_init_flag, init_infer_type);

        type infered_type = infer_type(expr, active_assembly);

        expr.annotations().add("type", annotation(infered_type));

        return infered_type;
    }
}
} // namespace

type_inference_result::type_inference_result(assembly_inference_result& assembly_inference_,
                                             pass_resource_manager& resource_manager_)
: m_assembly_inference_(&assembly_inference_), m_resource_manager(&resource_manager_)
{
}

type type_inference_result::get_type(const expression& expr) const
{
    return typeof_(expr, m_assembly_inference_->infered_assembly());
}

type_inference_result type_inference_pass::run(const function& func,
                                               function_analysis_manager& manager,
                                               pass_resource_manager& resource_manager) const
{
    return type_inference_result(manager.get_analysis<assembly_inference_pass>(func),
                                 resource_manager);
}

std::vector<analysis_id> type_inference_pass::required_analyses() const
{
    return {get_analysis_id<assembly_inference_pass>()};
}

} // namespace qubus