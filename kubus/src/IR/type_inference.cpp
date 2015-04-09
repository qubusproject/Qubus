#include <qbb/kubus/IR/type_inference.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/util/assert.hpp>

#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

qbb::util::multi_method<type(const qbb::util::virtual_<type>&, const qbb::util::virtual_<type>&)>
common_type_ = {};

type common_type_double_double(const types::double_&, const types::double_&)
{
    return types::double_{};
}

type common_type_float_float(const types::float_&, const types::float_&)
{
    return types::float_{};
}

type common_type_integer_integer(const types::integer&, const types::integer&)
{
    return types::integer{};
}

type common_type_index_index(const types::index&, const types::index&)
{
    return types::index{};
}

type common_type_double_integer(const types::double_&, const types::integer&)
{
    return types::double_{};
}

type common_type_integer_double(const types::integer&, const types::double_&)
{
    return types::double_{};
}

type common_type_float_integer(const types::float_&, const types::integer&)
{
    return types::float_{};
}

type common_type_integer_float(const types::integer&, const types::float_&)
{
    return types::float_{};
}

type common_type_double_float(const types::double_&, const types::float_&)
{
    return types::double_{};
}

type common_type_float_double(const types::float_&, const types::double_&)
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

type common_type_tensor_tensor(const types::tensor& lhs, const types::tensor& rhs)
{
    return types::tensor(common_type_(lhs.value_type(), rhs.value_type()));
}

void init_common_type()
{
    common_type_.add_specialization(common_type_double_double);
    common_type_.add_specialization(common_type_float_float);
    common_type_.add_specialization(common_type_integer_integer);
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
    common_type_.add_specialization(common_type_tensor_tensor);
}

std::once_flag common_type_init_flag = {};

type common_type(const type& t1, const type& t2)
{
    std::call_once(common_type_init_flag, init_common_type);

    return common_type_(t1, t2);
}


qbb::util::multi_method<type(const qbb::util::virtual_<type>&)>
value_type_ = {};

type value_type_tensor(const types::tensor& tensor_type)
{
    return tensor_type.value_type();
}

type value_type_sparse_tensor(const types::sparse_tensor& tensor_type)
{
    return tensor_type.value_type();
}

void init_value_type()
{
    value_type_.add_specialization(value_type_tensor);
    value_type_.add_specialization(value_type_sparse_tensor);
}

std::once_flag value_type_init_flag = {};

type value_type(const type& tensor_type)
{
    std::call_once(value_type_init_flag, init_value_type);

    return value_type_(tensor_type);
}


qbb::util::multi_method<type(const qbb::util::virtual_<expression>&)> infer_type = {};

type infer_type_binary_op_expr(const binary_operator_expr& expr)
{
    type left_type = typeof_(expr.left());
    type right_type = typeof_(expr.right());

    type result_type = common_type(left_type, right_type);

    return result_type;
}

type infer_type_unary_op_expr(const unary_operator_expr& expr)
{
    type arg_type = typeof_(expr.arg());

    return arg_type;
}

type infer_type_sum_expr(const sum_expr& expr)
{
    type body_type = typeof_(expr.body());

    return body_type;
}

type infer_type_for_all_expr(const for_all_expr&)
{
    return types::unknown{};
}

type infer_type_for_expr(const for_expr&)
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

type infer_type_double_literal_expr(const double_literal_expr&)
{
    return types::double_{};
}

type infer_type_float_literal_expr(const float_literal_expr&)
{
    return types::float_{};
}

type infer_type_integer_literal_expr(const integer_literal_expr&)
{
    return types::integer{};
}

type infer_type_intrinsic_function_expr(const intrinsic_function_expr& expr)
{
    std::vector<type> arg_types;
    
    for(const auto& arg : expr.args())
    {
        arg_types.push_back(typeof_(arg));
    }
    
    return lookup_intrinsic_result_type(expr.name(), arg_types);
}

type infer_type_compound_expr(const compound_expr&)
{
    return types::unknown{};
}

type infer_type_variable_ref_expr(const variable_ref_expr& expr)
{
    return expr.declaration().var_type();
}

type infer_type_scoped_view_expr(const scoped_view_expr&)
{
    return types::unknown{};
}

void init_infer_type()
{
    infer_type.add_specialization(infer_type_binary_op_expr);
    infer_type.add_specialization(infer_type_unary_op_expr);
    infer_type.add_specialization(infer_type_sum_expr);
    infer_type.add_specialization(infer_type_for_all_expr);
    infer_type.add_specialization(infer_type_for_expr);
    infer_type.add_specialization(infer_type_subscription_expr);
    infer_type.add_specialization(infer_type_type_conversion_expr);
    infer_type.add_specialization(infer_type_double_literal_expr);
    infer_type.add_specialization(infer_type_float_literal_expr);
    infer_type.add_specialization(infer_type_integer_literal_expr);
    infer_type.add_specialization(infer_type_intrinsic_function_expr);
    infer_type.add_specialization(infer_type_compound_expr);
    infer_type.add_specialization(infer_type_variable_ref_expr);
    infer_type.add_specialization(infer_type_scoped_view_expr);
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
}