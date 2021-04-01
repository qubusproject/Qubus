#ifndef QUBUS_IR_TYPES_HPP
#define QUBUS_IR_TYPES_HPP

#include <qubus/IR/constructor.hpp>
#include <qubus/IR/member.hpp>

#include <boost/range/adaptor/indirected.hpp>

#include <qubus/util/optional_ref.hpp>

#include <string>
#include <string_view>

namespace qubus
{

class object_type
{
public:
    enum class object_kind
    {
        concrete,
        abstract,
        builtin
    };

    explicit object_type(object_kind kind_, std::string name_, std::vector<property> properties_,
                         std::vector<std::shared_ptr<constructor>> constructors_)
    : m_kind(kind_),
      m_name(std::move(name_)),
      m_properties(std::move(properties_)),
      m_constructors(std::move(constructors_))
    {
    }

    object_type(const object_type&) = delete;
    object_type& operator=(const object_type&) = delete;

    object_type(object_type&&) noexcept = default;
    object_type& operator=(object_type&&) noexcept = default;

    [[nodiscard]] object_kind kind() const
    {
        return m_kind;
    }

    [[nodiscard]] std::string_view name() const
    {
        return m_name;
    }

    [[nodiscard]] const std::vector<property>& properties() const
    {
        return m_properties;
    }

    [[nodiscard]] const std::vector<type_property>& type_properties() const
    {
        return m_type_properties;
    }

    [[nodiscard]] util::optional_ref<const type_property> get_type_property(std::string_view id) const;

    [[nodiscard]] auto constructors() const
    {
        return m_constructors | boost::adaptors::indirected;
    }

    [[nodiscard]] const type& operator[](std::string_view id) const;

    [[nodiscard]] std::size_t property_index(std::string_view id) const;

    [[nodiscard]] std::size_t property_count() const
    {
        return m_properties.size();
    }

private:
    object_kind m_kind;
    std::string m_name;
    std::vector<property> m_properties;
    std::vector<type_property> m_type_properties;
    std::vector<std::shared_ptr<constructor>> m_constructors;
};

bool validate(const object_type& t);

class symbolic_type
{
public:
    [[nodiscard]] std::string_view name() const
    {
        return "type";
    }
};

class void_type
{
public:
    [[nodiscard]] std::string_view name() const
    {
        return "void";
    }
};

bool is_integer(const type& type);
bool is_integer(const object_type& type);

//type sparse_tensor(type value_type);
} // namespace qubus

#endif
