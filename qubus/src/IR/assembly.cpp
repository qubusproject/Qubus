#include <qubus/IR/assembly.hpp>

#include <qubus/util/assert.hpp>

namespace qubus
{

std::optional<type> assembly::lookup_type(std::string_view name) const
{
    auto search_result = m_type_table.find(std::string(name.data(), name.size()));

    if (search_result != m_type_table.end())
    {
        return search_result->second;
    }

    return std::nullopt;
}

std::optional<type>
assembly::instantiate_type_template(std::string_view name, const std::vector<compile_time_value>& args) const
{
    auto search_result = m_type_templates.find(std::string(name.data(), name.size()));

    if (search_result != m_type_templates.end())
    {
        return search_result->second.instantiate(args);
    }

    return std::nullopt;
}

type assembly::get_type_bool() const
{
    std::optional<type> type_bool = lookup_type("Bool");

    QUBUS_ASSERT(type_bool.has_value(), "Type 'Bool' must be reachable from every assembly.");

    return *type_bool;
}

type assembly::get_type_int64() const
{
    std::optional<type> type_int64 = lookup_type("Int64");

    QUBUS_ASSERT(type_int64.has_value(), "Type 'Int64' must be reachable from every assembly.");

    return *type_int64;
}

type assembly::get_type_int32() const
{
    std::optional<type> type_int32 = lookup_type("Int32");

    QUBUS_ASSERT(type_int32.has_value(), "Type 'Int32' must be reachable from every assembly.");

    return *type_int32;
}

type assembly::get_type_int16() const
{
    std::optional<type> type_int16 = lookup_type("Int16");

    QUBUS_ASSERT(type_int16.has_value(), "Type 'Int16' must be reachable from every assembly.");

    return *type_int16;
}

type assembly::get_type_int8() const
{
    std::optional<type> type_int8 = lookup_type("Int8");

    QUBUS_ASSERT(type_int8.has_value(), "Type 'Int8' must be reachable from every assembly.");

    return *type_int8;
}

type assembly::get_type_float64() const
{
    std::optional<type> type_float64 = lookup_type("Float64");

    QUBUS_ASSERT(type_float64.has_value(), "Type 'Float64' must be reachable from every assembly.");

    return *type_float64;
}

type assembly::get_type_float32() const
{
    std::optional<type> type_float32 = lookup_type("Float32");

    QUBUS_ASSERT(type_float32.has_value(), "Type 'Float32' must be reachable from every assembly.");

    return *type_float32;
}

type assembly::get_type_void() const
{
    std::optional<type> type_void = lookup_type("Void");

    QUBUS_ASSERT(type_void.has_value(), "Type 'Void' must be reachable from every assembly.");

    return *type_void;
}

type assembly::get_type_integer_range(type value_type) const
{
    std::optional<type> type_int_range =
        instantiate_type_template("IntRange", {compile_time_value(std::move(value_type))});

    QUBUS_ASSERT(type_int_range.has_value(),
                 "Type template 'IntRange' must be reachable from every assembly.");

    return *type_int_range;
}

type assembly::get_type_array(type value_type, util::index_t rank) const
{
    std::optional<type> type_array = instantiate_type_template(
        "Array", {compile_time_value(std::move(value_type)), compile_time_value(rank)});

    QUBUS_ASSERT(type_array.has_value(),
                 "Type template 'Array' must be reachable from every assembly.");

    return *type_array;
}

type assembly::get_type_array_slice(type value_type, util::index_t rank) const
{
    std::optional<type> type_array_slice = instantiate_type_template(
        "ArraySlice", {compile_time_value(std::move(value_type)), compile_time_value(rank)});

    QUBUS_ASSERT(type_array_slice.has_value(),
                 "Type template 'ArraySlice' must be reachable from every assembly.");

    return *type_array_slice;
}

} // namespace qubus
