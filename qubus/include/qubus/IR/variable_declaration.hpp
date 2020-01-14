#ifndef QUBUS_VARIABLE_DECLARATION_HPP
#define QUBUS_VARIABLE_DECLARATION_HPP

#include <hpx/config.hpp>

#include <qubus/IR/annotations.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/IR/template_parameter.hpp>

#include <qubus/util/unused.hpp>

#include <string_view>

namespace qubus
{

class variable_declaration final : public template_parameter
{
public:
    explicit variable_declaration(std::string name_, type var_type_);

    variable_declaration(const variable_declaration&) = delete;
    variable_declaration& operator=(const variable_declaration&) = delete;

    variable_declaration(variable_declaration&&) = delete;
    variable_declaration& operator=(variable_declaration&&) = delete;

    std::string_view name() const override;
    const type& var_type() const;

    annotation_map& annotations() const;
    annotation_map& annotations();

private:
    std::string name_;
    type var_type_;

    mutable annotation_map annotations_;
};

} // namespace qubus

#endif