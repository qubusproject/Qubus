#include <qubus/IR/type.hpp>
#include <qubus/IR/types.hpp>

namespace qubus
{

type::type(object_type type) : m_value(std::make_shared<const object_type>(std::move(type)))
{
}

type::type(symbolic_type type) : m_value(std::make_shared<const symbolic_type>(std::move(type)))
{
}

type::type(void_type type) : m_value(std::make_shared<const void_type>(type))
{
}

std::string_view type::name() const
{
    return std::visit([] (const auto& th) -> std::string_view { return th->name(); }, m_value);
}

namespace
{

struct value_type_visitor
{
    std::optional<type> operator()(const object_type& obj_type) const
    {
        if (util::optional_ref<const type_property> value_type = obj_type.get_type_property("value_type"))
        {
            // TODO: Assert that the property actually refers to a type.

            // Evaluate the value type.
            value_type->value();
        }

        // The object has no property 'value_type'.
        return std::nullopt;
    }

    std::optional<type> operator()(const symbolic_type& /*unused*/) const
    {
        return std::nullopt;
    }

    std::optional<type> operator()(const void_type& /*unused*/) const
    {
        return std::nullopt;
    }
};

}

std::optional<type> value_type(const type& t)
{
    return visit(t, value_type_visitor{});
}

} // namespace qubus