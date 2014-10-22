#include <qbb/kubus/IR/expression.hpp> 

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>

#include <mutex>
#include <functional>

namespace qbb
{
namespace kubus
{

qbb::util::implementation_table expression::implementation_table_ = {};

namespace
{

qbb::util::multi_method<bool(const qbb::util::virtual_<expression>&, const qbb::util::virtual_<expression>&)> equal = {};

template<typename T>
bool always_true(const T&,const T&)
{
    return true;
}

void init_equal()
{
    equal.add_specialization(always_true<binary_operator_expr>);
    equal.add_specialization(always_true<unary_operator_expr>);
    equal.add_specialization(always_true<float_literal_expr>);
    equal.add_specialization(always_true<double_literal_expr>);
    equal.add_specialization(always_true<integer_literal_expr>);
    equal.add_specialization(always_true<compound_expr>);
    equal.add_specialization(always_true<delta_expr>);
    equal.add_specialization(always_true<for_all_expr>);
    equal.add_specialization(always_true<for_expr>);
    equal.add_specialization(always_true<index_expr>);
    equal.add_specialization(always_true<intrinsic_function_expr>);
    equal.add_specialization(always_true<subscription_expr>);
    equal.add_specialization(always_true<sum_expr>);
    equal.add_specialization(always_true<tensor_access_expr>);
    equal.add_specialization(always_true<type_conversion_expr>);;
    
    equal.set_fallback([](const expression&, const expression&) { return false; });
}

std::once_flag equal_init_flag = {};

}

bool operator==(const expression& lhs, const expression& rhs)
{
    std::call_once(equal_init_flag, init_equal);
    
    return equal(lhs, rhs);
}

bool operator!=(const expression& lhs, const expression& rhs)
{
    return !(lhs == rhs);
}

}
}