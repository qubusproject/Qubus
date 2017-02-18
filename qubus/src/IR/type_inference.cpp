#include <qubus/IR/type_inference.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/qtl/IR/all.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/multi_method.hpp>
#include <qubus/util/unused.hpp>

#include <mutex>

namespace qubus
{

namespace
{

util::multi_method<type(const util::virtual_<type>&, const util::virtual_<type>&)>
    common_type_ = {};

type common_type_double_double(const types::double_& /*unused*/, const types::double_& /*unused*/)
{
    return types::double_{};
}

type common_type_float_float(const types::float_& /*unused*/, const types::float_& /*unused*/)
{
    return types::float_{};
}

type common_type_integer_integer(const types::integer& /*unused*/, const types::integer& /*unused*/)
{
    return types::integer{};
}

type common_type_bool_bool(const types::bool_& /*unused*/, const types::bool_& /*unused*/)
{
    return types::bool_{};
}

type common_type_index_index(const types::index& /*unused*/, const types::index& /*unused*/)
{
    return types::index{};
}

type common_type_double_integer(const types::double_& /*unused*/, const types::integer& /*unused*/)
{
    return types::double_{};
}

type common_type_integer_double(const types::integer& /*unused*/, const types::double_& /*unused*/)
{
    return types::double_{};
}

type common_type_float_integer(const types::float_& /*unused*/, const types::integer& /*unused*/)
{
    return types::float_{};
}

type common_type_integer_float(const types::integer& /*unused*/, const types::float_& /*unused*/)
{
    return types::float_{};
}

type common_type_double_float(const types::double_& /*unused*/, const types::float_& /*unused*/)
{
    return types::double_{};
}

type common_type_float_double(const types::float_& /*unused*/, const types::double_& /*unused*/)
{
    return types::double_{};
}

type common_type_complex_double(const types::complex& lhs, const types::double_& rhs)
{
    return types::complex(common_type_(lhs.real_type(), rhs));
}

type common_type_double_complex(const types::double_& lhs, const types::complex& rhs)
{
    return types::complex(common_type_(lhs, rhs.real_type()));
}

type common_type_complex_float(const types::complex& lhs, const types::float_& rhs)
{
    return types::complex(common_type_(lhs.real_type(), rhs));
}

type common_type_float_complex(const types::float_& lhs, const types::complex& rhs)
{
    return types::complex(common_type_(lhs, rhs.real_type()));
}

type common_type_complex_integer(const types::complex& lhs, const types::integer& rhs)
{
    return types::complex(common_type_(lhs.real_type(), rhs));
}

type common_type_integer_complex(const types::integer& lhs, const types::complex& rhs)
{
    return types::complex(common_type_(lhs, rhs.real_type()));
}

type common_type_complex_complex(const types::complex& lhs, const types::complex& rhs)
{
    return types::complex(common_type_(lhs.real_type(), rhs.real_type()));
}

void init_common_type()
{
    common_type_.add_specialization(common_type_double_double);
    common_type_.add_specialization(common_type_float_float);
    common_type_.add_specialization(common_type_integer_integer);
    common_type_.add_specialization(common_type_bool_bool);
    common_type_.add_specialization(common_type_index_index);
    common_type_.add_specialization(common_type_double_integer);
    common_type_.add_specialization(common_type_integer_double);
    common_type_.add_specialization(common_type_float_integer);
    common_type_.add_specialization(common_type_integer_float);
    common_type_.add_specialization(common_type_double_float);
    common_type_.add_specialization(common_type_float_double);
    common_type_.add_specialization(common_type_complex_double);
    common_type_.add_specialization(common_type_double_complex);
    common_type_.add_specialization(common_type_complex_float);
    common_type_.add_specialization(common_type_float_complex);
    common_type_.add_specialization(common_type_complex_integer);
    common_type_.add_specialization(common_type_integer_complex);
    common_type_.add_specialization(common_type_complex_complex);
}

std::once_flag common_type_init_flag = {};

type common_type(const type& t1, const type& t2)
{
    std::call_once(common_type_init_flag, init_common_type);

    return common_type_(t1, t2);
}

type value_type(const type& tensor_type)
{
    using pattern::_;

    pattern::variable<type> value_type;

    auto m = pattern::make_matcher<type, type>()
                 .case_(sparse_tensor_t(value_type), [&] { return value_type.get(); })
                 .case_(array_t(value_type, _), [&] { return value_type.get(); })
                 .case_(array_slice_t(value_type, _), [&] { return value_type.get(); });

    return pattern::match(tensor_type, m);
}

util::multi_method<type(const util::virtual_<expression>&)> infer_type = {};

type infer_type_binary_op_expr(const binary_operator_expr& expr)
{
    switch (expr.tag())
    {
    case binary_op_tag::equal_to:
    case binary_op_tag::less_equal:
    case binary_op_tag::less:
    case binary_op_tag::greater_equal:
    case binary_op_tag::greater:
        return types::bool_();
    default:
    {
        type left_type = typeof_(expr.left());
        type right_type = typeof_(expr.right());

        return common_type(left_type, right_type);
    }
    }
}

type infer_type_unary_op_expr(const unary_operator_expr& expr)
{
    type arg_type = typeof_(expr.arg());

    return arg_type;
}

type infer_type_sum_expr(const qtl::sum_expr& expr)
{
    type body_type = typeof_(expr.body());

    return body_type;
}

type infer_type_for_all_expr(const qtl::for_all_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_for_expr(const for_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_if_expr(const if_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_subscription_expr(const subscription_expr& expr)
{
    type tensor_type = typeof_(expr.indexed_expr());

    bool has_abstract_index = false;

    for (const auto& index : expr.indices())
    {
        auto index_type = typeof_(index);

        if (index_type == types::index())
        {
            has_abstract_index = true;
            break;
        }
    }

    if (has_abstract_index)
    {
        return tensor_type;
    }
    else
    {
        return value_type(tensor_type);
    }
}

type infer_type_type_conversion_expr(const type_conversion_expr& expr)
{
    return expr.target_type();
}

type infer_type_double_literal_expr(const double_literal_expr& /*unused*/)
{
    return types::double_{};
}

type infer_type_float_literal_expr(const float_literal_expr& /*unused*/)
{
    return types::float_{};
}

type infer_type_integer_literal_expr(const integer_literal_expr& /*unused*/)
{
    return types::integer{};
}

type infer_type_intrinsic_function_expr(const intrinsic_function_expr& expr)
{
    std::vector<type> arg_types;

    for (const auto& arg : expr.args())
    {
        arg_types.push_back(typeof_(arg));
    }

    return lookup_intrinsic_result_type(expr.name(), arg_types);
}

type infer_type_compound_expr(const compound_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_variable_ref_expr(const variable_ref_expr& expr)
{
    return expr.declaration().var_type();
}

type infer_type_spawn_expr(const spawn_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_local_variable_def_expr(const local_variable_def_expr& /*unused*/)
{
    return types::unknown{};
}

type infer_type_construct_expr(const construct_expr& expr)
{
    return expr.result_type();
}

type infer_type_kronecker_delta_expr(const qtl::kronecker_delta_expr& QUBUS_UNUSED(expr))
{
    return types::integer();
}

type infer_type_member_access_expr(const member_access_expr& expr)
{
    auto obj_type = typeof_(expr.object()).as<types::struct_>();

    return obj_type[expr.member_name()];
}

type infer_type_foreign_call_expr(const foreign_call_expr& QUBUS_UNUSED(expr))
{
    return types::unknown{};
}

type infer_type_array_slice_expr(const array_slice_expr& expr)
{
    pattern::variable<type> value_type;
    pattern::variable<util::index_t> rank;

    auto m = pattern::make_matcher<type, type>().case_(
        array_t(value_type, rank) || array_slice_t(value_type, rank),
        [&] { return types::array_slice(value_type.get(), rank.get()); });

    return pattern::match(typeof_(expr.array()), m);
}

void init_infer_type()
{
    infer_type.add_specialization(infer_type_binary_op_expr);
    infer_type.add_specialization(infer_type_unary_op_expr);
    infer_type.add_specialization(infer_type_sum_expr);
    infer_type.add_specialization(infer_type_for_all_expr);
    infer_type.add_specialization(infer_type_for_expr);
    infer_type.add_specialization(infer_type_if_expr);
    infer_type.add_specialization(infer_type_subscription_expr);
    infer_type.add_specialization(infer_type_type_conversion_expr);
    infer_type.add_specialization(infer_type_double_literal_expr);
    infer_type.add_specialization(infer_type_float_literal_expr);
    infer_type.add_specialization(infer_type_integer_literal_expr);
    infer_type.add_specialization(infer_type_intrinsic_function_expr);
    infer_type.add_specialization(infer_type_compound_expr);
    infer_type.add_specialization(infer_type_variable_ref_expr);
    infer_type.add_specialization(infer_type_spawn_expr);
    infer_type.add_specialization(infer_type_local_variable_def_expr);
    infer_type.add_specialization(infer_type_construct_expr);
    infer_type.add_specialization(infer_type_kronecker_delta_expr);
    infer_type.add_specialization(infer_type_member_access_expr);
    infer_type.add_specialization(infer_type_foreign_call_expr);
    infer_type.add_specialization(infer_type_array_slice_expr);
}

std::once_flag infer_type_init_flag = {};
}

type typeof_(const expression& expr)
{
    if (auto type_annotation = expr.annotations().lookup("type"))
    {
        return type_annotation.as<type>();
    }
    else
    {
        std::call_once(infer_type_init_flag, init_infer_type);

        type infered_type = infer_type(expr);

        expr.annotations().add("type", annotation(infered_type));

        return infered_type;
    }
}
}