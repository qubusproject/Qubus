#ifndef QUBUS_TYPE_HPP
#define QUBUS_TYPE_HPP

#include <hpx/config.hpp>

#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <optional>

namespace qubus
{

class module;

class object_type;
class symbolic_type;
class void_type;

class type
{
public:
    using id_type = const void*;

    explicit type(object_type type);
    explicit type(symbolic_type type);
    explicit type(void_type type);

    [[nodiscard]] id_type id() const
    {
        return std::visit([] (const auto& th) -> id_type { return th.get(); }, m_value);
    }

    [[nodiscard]] std::string_view name() const;

    template <typename Visitor>
    friend auto visit(const type& t, Visitor&& visitor)
    {
        return std::visit([&visitor] (const auto& th) { return visitor(*th); }, t.m_value);
    }

    template <typename ConcreteType>
    const ConcreteType& as() const
    {
        return *std::get<type_holder_type<ConcreteType>>(m_value);
    }

    template <typename ConcreteType>
    const ConcreteType* try_as() const
    {
        if (auto value_ref = std::get_if<type_holder_type<ConcreteType>>(&m_value))
        {
            return value_ref->get();
        }

        return nullptr;
    }
private:
    template <typename Type>
    using type_holder_type = std::shared_ptr<const Type>;

    template <typename... TypeClasses>
    using data_holder_type =
        std::variant<type_holder_type<TypeClasses>...>;

    data_holder_type<object_type, symbolic_type, void_type> m_value;
};

bool operator==(const type& lhs, const type& rhs)
{
    return lhs.id() == rhs.id();
}

bool operator!=(const type& lhs, const type& rhs)
{
    return !(lhs == rhs);
}

// Type helpers

std::optional<type> value_type(const type& t);

} // namespace qubus

#endif
