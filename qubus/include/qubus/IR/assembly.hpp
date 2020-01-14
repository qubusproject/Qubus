#ifndef QUBUS_IR_ASSEMBLY_HPP
#define QUBUS_IR_ASSEMBLY_HPP

#include <qubus/IR/compile_time_evaluator.hpp>

#include <qubus/IR/function.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/IR/templates.hpp>

#include <boost/range/adaptor/map.hpp>

#include <optional>
#include <string_view>
#include <unordered_map>

namespace qubus
{

class assembly
{
public:


    [[nodiscard]] auto functions() const
    {
        return m_function_table | boost::adaptors::map_values;
    }

    [[nodiscard]] auto types() const
    {
        return m_type_table | boost::adaptors::map_values;
    }

    [[nodiscard]] std::optional<type> lookup_type(std::string_view name) const;

    [[nodiscard]] std::optional<type>
    instantiate_type_template(std::string_view name, const std::vector<compile_time_value>& args);

    [[nodiscard]] type get_type_bool() const;

    [[nodiscard]] type get_type_int64() const;
    [[nodiscard]] type get_type_int32() const;
    [[nodiscard]] type get_type_int16() const;
    [[nodiscard]] type get_type_int8() const;

    [[nodiscard]] type get_type_float64() const;
    [[nodiscard]] type get_type_float32() const;

    [[nodiscard]] type get_type_void() const;

    [[nodiscard]] type get_type_integer_range(type value_type);

    [[nodiscard]] type get_type_array(type value_type, util::index_t rank);
    [[nodiscard]] type get_type_array_slice(type value_type, util::index_t rank);

private:
    std::unordered_map<std::string, function> m_function_table;
    std::unordered_map<std::string, type> m_type_table;

    std::unordered_map<std::string, type_template> m_type_templates;
    std::unordered_map<std::string, function_template> m_function_templates;
};

} // namespace qubus

#endif
