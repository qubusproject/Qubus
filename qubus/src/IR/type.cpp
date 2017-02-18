#include <qubus/IR/type.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/type.hpp>

#include <qubus/util/hash.hpp>
#include <qubus/util/multi_method.hpp>

#include <algorithm>
#include <mutex>

namespace qubus
{

namespace
{

util::sparse_multi_method<bool(const util::virtual_<type>&,
                                    const util::virtual_<type>&)>
    type_eq = {};

bool type_eq_double(const types::double_& /*unused*/, const types::double_& /*unused*/)
{
    return true;
}

bool type_eq_float(const types::float_& /*unused*/, const types::float_& /*unused*/)
{
    return true;
}

bool type_eq_integer(const types::integer& /*unused*/, const types::integer& /*unused*/)
{
    return true;
}

bool type_eq_bool(const types::bool_& /*unused*/, const types::bool_& /*unused*/)
{
    return true;
}

bool type_eq_index(const types::index& /*unused*/, const types::index& /*unused*/)
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

bool type_eq_default(const type& /*unused*/, const type& /*unused*/)
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

util::implementation_table type::implementation_table_ = {};

namespace types
{
const type& struct_::operator[](const std::string& id) const
{
    auto iter = std::find_if(members_.begin(), members_.end(),
                             [&id](const member& value) { return value.id == id; });

    if (iter != members_.end())
        return iter->datatype;

    throw 0;
}

std::size_t struct_::member_index(const std::string& id) const
{
    auto iter = std::find_if(members_.begin(), members_.end(),
                             [&id](const member& value) { return value.id == id; });

    if (iter != members_.end())
        return iter - members_.begin();

    throw 0;
}

type sparse_tensor(type value_type)
{
    auto sell_tensor_type = types::struct_(
        "sell_tensor", {types::struct_::member(types::array(std::move(value_type), 1), "val"),
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

namespace std
{
std::size_t hash<qubus::type>::operator()(const qubus::type& value) const noexcept
{
    using namespace qubus;
    using pattern::_;

    pattern::variable<type> t;
    pattern::variable<util::index_t> rank;

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

                       util::hash_combine(seed, std::type_index(typeid(types::complex)));
                       util::hash_combine(seed, t.get());

                       return seed;
                   })
            .case_(pattern::array_t(t, rank),
                   [&] {
                       std::size_t seed = 0;

                       util::hash_combine(seed, std::type_index(typeid(types::array)));
                       util::hash_combine(seed, t.get());
                       util::hash_combine(seed, rank.get());

                       return seed;
                   })
            .case_(pattern::array_slice_t(t, rank),
                   [&] {
                       std::size_t seed = 0;

                       util::hash_combine(seed, std::type_index(typeid(types::array_slice)));
                       util::hash_combine(seed, t.get());
                       util::hash_combine(seed, rank.get());

                       return seed;
                   })
            .case_(pattern::struct_t(_, _), [&](const type& self) {
                const auto& stype = self.as<types::struct_>();

                std::size_t seed = 0;

                util::hash_combine(seed, std::type_index(typeid(types::struct_)));
                util::hash_combine(seed, stype.id());

                for (const auto& member : stype)
                {
                    util::hash_combine(seed, member);
                }

                return seed;
            });

    return pattern::match(value, m);
}

std::size_t hash<qubus::types::struct_::member>::
operator()(const qubus::types::struct_::member& value) const noexcept
{
    std::size_t seed = 0;

    qubus::util::hash_combine(seed, value.datatype);
    qubus::util::hash_combine(seed, value.id);

    return seed;
}
}