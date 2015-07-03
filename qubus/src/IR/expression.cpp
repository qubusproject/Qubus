#include <qbb/qubus/IR/expression.hpp> 

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/util/multi_method.hpp>

#include <mutex>
#include <functional>

namespace qbb
{
namespace qubus
{

qbb::util::implementation_table expression::implementation_table_ = {};

namespace
{

qbb::util::multi_method<bool(const qbb::util::virtual_<expression>&, const qbb::util::virtual_<expression>&)> equal = {};

void init_equal()
{
    equal.add_specialization(std::equal_to<binary_operator_expr>());
    equal.add_specialization(std::equal_to<unary_operator_expr>());
    equal.add_specialization(std::equal_to<float_literal_expr>());
    equal.add_specialization(std::equal_to<double_literal_expr>());
    equal.add_specialization(std::equal_to<integer_literal_expr>());
    equal.add_specialization(std::equal_to<compound_expr>());
    equal.add_specialization(std::equal_to<for_all_expr>());
    equal.add_specialization(std::equal_to<for_expr>());
    equal.add_specialization(std::equal_to<intrinsic_function_expr>());
    equal.add_specialization(std::equal_to<subscription_expr>());
    equal.add_specialization(std::equal_to<sum_expr>());
    equal.add_specialization(std::equal_to<type_conversion_expr>());
    equal.add_specialization(std::equal_to<variable_ref_expr>());
    
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