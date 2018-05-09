#ifndef QUBUS_VARIABLE_DECLARATION_HPP
#define QUBUS_VARIABLE_DECLARATION_HPP

#include <hpx/config.hpp>

#include <qubus/IR/type.hpp>
#include <qubus/IR/annotations.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/unused.hpp>

#include <memory>
#include <functional>

namespace qubus
{

class variable_declaration_info
{
public:
    variable_declaration_info() = default;
    variable_declaration_info(std::string name_, type var_type_);

    variable_declaration_info(const variable_declaration_info&) = delete;
    variable_declaration_info& operator=(const variable_declaration_info&) = delete;

    const std::string& name() const;
    const type& var_type() const;

    annotation_map& annotations() const;

    annotation_map& annotations();
private:
    std::string name_;
    type var_type_;

    mutable annotation_map annotations_;
};

class variable_declaration
{
public:
    variable_declaration() = default;
    variable_declaration(std::string name_, type var_type_);

    const std::string& name() const;
    const type& var_type() const;
    util::handle id() const;
    
    annotation_map& annotations() const;
    annotation_map& annotations();
private:
    std::shared_ptr<variable_declaration_info> info_;
};

bool operator==(const variable_declaration& lhs, const variable_declaration& rhs);
bool operator!=(const variable_declaration& lhs, const variable_declaration& rhs);

}

namespace std
{

template<>
struct less<qubus::variable_declaration>
{
    using result_type = bool;
    using first_argument_type = qubus::variable_declaration;
    using second_argument_type = first_argument_type;

    bool operator()(const qubus::variable_declaration& lhs, const qubus::variable_declaration& rhs) const
    {
        return lhs.id() < rhs.id();
    }
};

}

#endif