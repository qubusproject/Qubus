#include <qbb/kubus/IR/type.hpp>

#include <qbb/kubus/pattern/core.hpp>
#include <qbb/kubus/pattern/type.hpp>

#include <qbb/util/multi_method.hpp>
#include <qbb/util/hash.hpp>

#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

qbb::util::sparse_multi_method<bool(const qbb::util::virtual_<type>&,
                                    const qbb::util::virtual_<type>&)> type_eq = {};

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

bool type_eq_index(const types::index&, const types::index&)
{
    return true;
}

bool type_eq_complex(const types::complex& lhs, const types::complex& rhs)
{
    return lhs.real_type() == rhs.real_type();
}

bool type_eq_tensor(const types::tensor& lhs, const types::tensor& rhs)
{
    return lhs.value_type() == rhs.value_type();
}

bool type_eq_sparse_tensor(const types::sparse_tensor& lhs, const types::sparse_tensor& rhs)
{
    return lhs.value_type() == rhs.value_type();
}

bool type_eq_default(const type&, const type&)
{
    return false;
}

void init_type_eq()
{
    type_eq.add_specialization(type_eq_double);
    type_eq.add_specialization(type_eq_float);
    type_eq.add_specialization(type_eq_integer);
    type_eq.add_specialization(type_eq_index);
    type_eq.add_specialization(type_eq_complex);
    type_eq.add_specialization(type_eq_tensor);
    type_eq.add_specialization(type_eq_sparse_tensor);

    type_eq.set_fallback(type_eq_default);
}

std::once_flag type_eq_init_flag = {};
}

bool operator==(const type& lhs, const type& rhs)
{
    std::call_once(type_eq_init_flag, init_type_eq);

    return type_eq(lhs, rhs);
}

bool operator!=(const type& lhs, const type& rhs)
{
    return !(lhs == rhs);
}

qbb::util::implementation_table type::implementation_table_ = {};
}
}

namespace std
{
std::size_t hash<qbb::kubus::type>::operator()(const qbb::kubus::type& value) const noexcept
{
    using namespace qbb::kubus;

    pattern::variable<type> t;

    auto m = pattern::make_matcher<type, std::size_t>()
                 .case_(pattern::double_t,
                        [&]
                        {
                            return std::hash<std::type_index>()(typeid(types::double_));
                        })
                 .case_(pattern::float_t,
                        [&]
                        {
                            return std::hash<std::type_index>()(typeid(types::float_));
                        })
                 .case_(pattern::integer_t,
                        [&]
                        {
                            return std::hash<std::type_index>()(typeid(types::integer));
                        })
                 .case_(pattern::index_t,
                        [&]
                        {
                            return std::hash<std::type_index>()(typeid(types::index));
                        })
                 .case_(pattern::complex_t(t),
                        [&]
                        {
                            std::size_t seed = 0;

                            qbb::util::hash_combine(seed, std::type_index(typeid(types::complex)));
                            qbb::util::hash_combine(seed, t.get());

                            return seed;
                        })
                 .case_(pattern::tensor_t(t),
                        [&]
                        {
                            std::size_t seed = 0;

                            qbb::util::hash_combine(seed, std::type_index(typeid(types::tensor)));
                            qbb::util::hash_combine(seed, t.get());

                            return seed;
                        })
                 .case_(pattern::sparse_tensor_t(t), [&]
                        {
                            std::size_t seed = 0;

                            qbb::util::hash_combine(seed, std::type_index(typeid(types::sparse_tensor)));
                            qbb::util::hash_combine(seed, t.get());

                            return seed;
                        });

    return pattern::match(value, m);
}
}