#include <qbb/kubus/IR/type_inference.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>

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

void init_common_type()
{
    common_type_.add_specialization(common_type_double_double);
    common_type_.add_specialization(common_type_float_float);
    common_type_.add_specialization(common_type_integer_integer);
    common_type_.add_specialization(common_type_double_integer);
    common_type_.add_specialization(common_type_integer_double);
    common_type_.add_specialization(common_type_float_integer);
    common_type_.add_specialization(common_type_integer_float);
    common_type_.add_specialization(common_type_double_float);
    common_type_.add_specialization(common_type_float_double);
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

type infer_type_for_expr(const for_expr&)
{
    return types::unknown{};
}

type infer_type_subscription_expr(const subscription_expr& expr)
{
    type tensor_type = typeof_(expr.indexed_expr());
    
    type val_type = value_type(tensor_type);
    
    return val_type;
}

type infer_type_tensor_variable_expr(const tensor_variable_expr& expr)
{
    return expr.tensor_type();
}

type infer_type_type_conversion_expr(const type_conversion_expr& expr)
{
    return expr.target_type();
}

type infer_type_delta_expr(const delta_expr&)
{
    return types::integer{};
}

type infer_type_index_expr(const index_expr&)
{
    return types::integer{};
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
    //TODO: implement a more general approach
    if(expr.args().size() == 1)
    {
        return typeof_(expr.args()[0]);
    }
    else
    {
        return types::unknown{};
    }
}

void init_infer_type()
{
    infer_type.add_specialization(infer_type_binary_op_expr);
    infer_type.add_specialization(infer_type_unary_op_expr);
    infer_type.add_specialization(infer_type_sum_expr);
    infer_type.add_specialization(infer_type_for_expr);
    infer_type.add_specialization(infer_type_subscription_expr);
    infer_type.add_specialization(infer_type_tensor_variable_expr);
    infer_type.add_specialization(infer_type_type_conversion_expr);
    infer_type.add_specialization(infer_type_delta_expr);
    infer_type.add_specialization(infer_type_index_expr);
    infer_type.add_specialization(infer_type_double_literal_expr);
    infer_type.add_specialization(infer_type_float_literal_expr);
    infer_type.add_specialization(infer_type_integer_literal_expr);
    infer_type.add_specialization(infer_type_intrinsic_function_expr);
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