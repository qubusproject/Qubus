#include <qubus/IR/types.hpp>

namespace qubus
{
util::optional_ref<const type_property> object_type::get_type_property(std::string_view id) const
{
    auto search_result =
        std::find_if(m_type_properties.begin(), m_type_properties.end(),
                     [&id](const type_property& prop) { return prop.id() == id; });

    if (search_result != m_type_properties.end())
    {
        return *search_result;
    }

    return std::nullopt;
}

const type& object_type::operator[](std::string_view id) const
{
    auto iter = std::find_if(m_properties.begin(), m_properties.end(),
                             [&id](const property& value) { return value.id() == id; });

    if (iter != m_properties.end())
        return iter->datatype();

    throw 0;
}

std::size_t object_type::property_index(std::string_view id) const
{
    auto iter = std::find_if(m_properties.begin(), m_properties.end(),
                             [&id](const property& value) { return value.id() == id; });

    if (iter != m_properties.end())
        return iter - m_properties.begin();

    throw 0;
}

bool validate(const object_type& t)
{
    if (t.kind() == object_type::object_kind::abstract || t.kind() == object_type::object_kind::builtin)
    {
        for (const property& prop : t.properties())
        {
            if (prop.getter() != nullptr)
            {
                return false;
            }

            if (prop.setter() != nullptr)
            {
                return false;
            }
        }
    }

    return true;
}

/*type sparse_tensor(type value_type)
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
}*/

bool is_integer(const type& tested_type)
{
    const object_type* obj_type = tested_type.try_as<object_type>();

    if (obj_type == nullptr)
    {
        return false;
    }

    return is_integer(*obj_type);
}

bool is_integer(const object_type& tested_type)
{
    // Currently only built-in types can be integers.
    if (tested_type.kind() != object_type::object_kind::builtin)
    {
        return false;
    }

    if (tested_type.name() == "int64")
    {
        return true;
    }
}
}