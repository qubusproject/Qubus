#ifndef QUBUS_IR_MEMBER_HPP
#define QUBUS_IR_MEMBER_HPP

#include <qubus/IR/expression.hpp>
#include <qubus/IR/type.hpp>

#include <qubus/util/unused.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace qubus
{

class property
{
public:
    explicit property(type datatype, std::string id)
    : m_datatype(std::move(datatype)), m_id(std::move(id))
    {
    }

    property(const property&) = delete;
    property& operator=(const property&) = delete;

    property(property&&) noexcept = default;
    property& operator=(property&&) noexcept = default;

    const type& datatype() const
    {
        return m_datatype;
    }

    std::string_view id() const
    {
        return m_id;
    }

    const expression* getter() const
    {
        return m_getter.get();
    }

    const expression* setter() const
    {
        return m_setter.get();
    }

    bool is_immutable() const
    {
        return setter() == nullptr;
    }

    friend bool operator==(const property& lhs, const property& rhs)
    {
        return lhs.datatype() == rhs.datatype() && lhs.id() == rhs.id();
    }

    friend bool operator!=(const property& lhs, const property& rhs)
    {
        return !(lhs == rhs);
    }

private:
    type m_datatype;
    std::string m_id;

    std::unique_ptr<expression> m_getter = nullptr;
    std::unique_ptr<expression> m_setter = nullptr;
};

class type_property
{
public:
    type_property(const type_property&) = delete;
    type_property& operator=(const type_property&) = delete;

    type_property(type_property&&) noexcept = default;
    type_property& operator=(type_property&&) noexcept = default;

    [[nodiscard]] const type& datatype() const
    {
        return m_datatype;
    }

    [[nodiscard]] std::string_view id() const
    {
        return m_id;
    }

    [[nodiscard]] const expression& value() const
    {
        return *m_value;
    }
private:
    type m_datatype;
    std::string m_id;
    std::unique_ptr<expression> m_value;
};

} // namespace qubus

#endif
