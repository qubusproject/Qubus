#ifndef QUBUS_UNIQUE_VARIABLE_GENERATOR_HPP
#define QUBUS_UNIQUE_VARIABLE_GENERATOR_HPP

#include <qubus/IR/variable_declaration.hpp>
#include <qubus/IR/type.hpp>

#include <unordered_set>
#include <string_view>
#include <string>

namespace qubus
{

class function;
class expression;

class unique_variable_generator
{
public:
    unique_variable_generator() = default;
    explicit unique_variable_generator(const function& func);
    explicit unique_variable_generator(const expression& expr);

    variable_declaration create_new_variable(type var_type);
    variable_declaration create_new_variable(type var_type, std::string_view hint);

private:
    std::string generate_unique_name(std::string_view hint);

    std::unordered_set<std::string> reserved_names_;
};

}

#endif
