#include <qbb/qubus/IR/type.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/type.hpp>

#include <qbb/util/hash.hpp>
#include <qbb/util/multi_method.hpp>

#include <algorithm>
#include <mutex>

inline namespace qbb
{
namespace qubus
{

namespace
{

qbb::util::sparse_multi_method<bool(const qbb::util::virtual_<type>&,
                                    const qbb::util::virtual_<type>&)>
    type_eq = {};

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

bool type_eq_bool(const types::bool_&, const types::bool_&)
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

bool type_eq_array(const types::array& lhs, const types::array& rhs)
{
    return lhs.value_type() == rhs.value_type() && lhs.rank() == rhs.rank();
}

bool type_eq_array_slice(const types::array_slice& lhs, const types::array_slice& rhs)
{
    return lhs.value_type() == rhs.value_type() && lhs.rank() == rhs.rank();
}

bool type_eq_struct(const types::struct_& lhs, const types::struct_& rhs)
{
    return lhs.id() == rhs.id();
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
    type_eq.add_specialization(type_eq_bool);
    type_eq.add_specialization(type_eq_index);
    type_eq.add_specialization(type_eq_complex);
    type_eq.add_specialization(type_eq_array);
    type_eq.add_specialization(type_eq_array_slice);
    type_eq.add_specialization(type_eq_struct);

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

namespace types
{
const type& struct_::operator[](const std::string& id) const
{
    auto iter = std::find_if(members_.begin(), members_.end(),
                             [&id](const member& value) { return value.id == id; });

    if (iter != members_.end())
    {
        return iter->datatype;
    }
    else
    {
        throw 0;
    }
}

std::size_t struct_::member_index(const std::string& id) const
{
    auto iter = std::find_if(members_.begin(), members_.end(),
                             [&id](const member& value) { return value.id == id; });

    if (iter != members_.end())
    {
        return iter - members_.begin();
    }
    else
    {
        throw 0;
    }
}

type sparse_tensor(type value_type)
{
    auto sell_tensor_type = types::struct_(
        "sell_tensor", {types::struct_::member(types::array(value_type, 1), "val"),
                        types::struct_::member(types::array(types::integer(), 1), "col"),
                        types::struct_::member(types::array(types::integer(), 1), "cs"),
                        types::struct_::member(types::array(types::integer(), 1), "cl")});

    auto sparse_tensor_type = types::struct_(
        "sparse_tensor", {types::struct_::member(sell_tensor_type, "data"),
                          types::struct_::member(types::array(types::integer(), 1), "shape")});

    return sparse_tensor_type;
}
}
}
}

namespace std
{
std::size_t hash<qbb::qubus::type>::operator()(const qbb::qubus::type& value) const noexcept
{
    using namespace qbb::qubus;
    using pattern::_;

    pattern::variable<type> t;
    pattern::variable<qbb::util::index_t> rank;

    auto m =
        pattern::make_matcher<type, std::size_t>()
            .case_(pattern::double_t,
                   [&] { return std::hash<std::type_index>()(typeid(types::double_)); })
            .case_(pattern::float_t,
                   [&] { return std::hash<std::type_index>()(typeid(types::float_)); })
            .case_(pattern::integer_t,
                   [&] { return std::hash<std::type_index>()(typeid(types::integer)); })
            .case_(pattern::index_t,
                   [&] { return std::hash<std::type_index>()(typeid(types::index)); })
            .case_(pattern::complex_t(t),
                   [&] {
                       std::size_t seed = 0;

                       qbb::util::hash_combine(seed, std::type_index(typeid(types::complex)));
                       qbb::util::hash_combine(seed, t.get());

                       return seed;
                   })
            .case_(pattern::array_t(t, rank),
                   [&] {
                       std::size_t seed = 0;

                       qbb::util::hash_combine(seed, std::type_index(typeid(types::array)));
                       qbb::util::hash_combine(seed, t.get());
                       qbb::util::hash_combine(seed, rank.get());

                       return seed;
                   })
            .case_(pattern::array_slice_t(t, rank),
                   [&] {
                       std::size_t seed = 0;

                       qbb::util::hash_combine(seed, std::type_index(typeid(types::array_slice)));
                       qbb::util::hash_combine(seed, t.get());
                       qbb::util::hash_combine(seed, rank.get());

                       return seed;
                   })
            .case_(pattern::struct_t(_, _), [&](const type& self) {
                const auto& stype = self.as<types::struct_>();

                std::size_t seed = 0;

                qbb::util::hash_combine(seed, std::type_index(typeid(types::struct_)));
                qbb::util::hash_combine(seed, stype.id());

                for (const auto& member : stype)
                {
                    qbb::util::hash_combine(seed, member);
                }

                return seed;
            });

    return pattern::match(value, m);
}

std::size_t hash<qbb::qubus::types::struct_::member>::
operator()(const qbb::qubus::types::struct_::member& value) const noexcept
{
    std::size_t seed = 0;

    qbb::util::hash_combine(seed, value.datatype);
    qbb::util::hash_combine(seed, value.id);

    return seed;
}
}