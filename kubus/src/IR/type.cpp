#include <qbb/kubus/IR/type.hpp> 

#include <qbb/util/multi_method.hpp>

#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{
    
qbb::util::sparse_multi_method<bool(const qbb::util::virtual_<type>&, const qbb::util::virtual_<type>&)> type_eq = {};
    
bool type_eq_double(const types::double_&, const types::double_&)
{
    return true;
}

bool type_eq_float(const types::float_&, const types::float_&)
{
    return true;
}

bool type_eq_integer(const types::integer&, const types::integer&)
{
    return true;
}

bool type_eq_tensor(const types::tensor& lhs, const types::tensor& rhs)
{
    return lhs.value_type() == rhs.value_type();
}

bool type_eq_sparse_tensor(const types::sparse_tensor& lhs, const types::sparse_tensor& rhs)
{
    return lhs.value_type() == rhs.value_type();
}

bool type_eq_sparse_tensor_default(const type&, const type&)
{
    return false;
}

void init_type_eq()
{
    type_eq.add_specialization(type_eq_double);
    type_eq.add_specialization(type_eq_float);
    type_eq.add_specialization(type_eq_integer);
    type_eq.add_specialization(type_eq_tensor);
    type_eq.add_specialization(type_eq_sparse_tensor);
    
    type_eq.set_fallback(type_eq_sparse_tensor_default);
}

std::once_flag type_eq_init_flag = {};

}

bool operator==(const type& lhs, const type& rhs)
{
    std::call_once(type_eq_init_flag, init_type_eq);
    
    return type_eq(lhs, rhs);
}

qbb::util::implementation_table type::implementation_table_ = {};
    
}
}