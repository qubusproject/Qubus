#ifndef QUBUS_IR_FUNCTION_HPP
#define QUBUS_IR_FUNCTION_HPP

#include <hpx/config.hpp>

#include <qubus/IR/annotations.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/type.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/unused.hpp>

#include <boost/optional.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

namespace qubus
{

class module;

class function
{
public:
    explicit function(module& containing_module_, std::string name_,
                      std::vector<variable_declaration> params_, variable_declaration result_,
                      std::unique_ptr<expression> body_);

    function(const function&) = delete;
    function(function&&) = default;

    function& operator=(const function&) = delete;
    function& operator=(function&&) = default;

    const std::string& name() const;
    std::string full_name() const;

    util::handle id() const;

    const std::vector<variable_declaration>& params() const;

    const variable_declaration& result() const;

    std::size_t arity() const;

    const expression& body() const;

    module& containing_module() const;

    void substitute_body(std::unique_ptr<expression> body);

    annotation_map& annotations() const;

    annotation_map& annotations();

    variable_declaration declare_new_local_variable(type datatype);
    variable_declaration declare_new_local_variable(type datatype, const std::string& hint);

private:
    module* containing_module_;
    std::string name_;
    std::vector<variable_declaration> params_;
    variable_declaration result_;
    std::unique_ptr<expression> body_;

    std::unordered_map<std::string, variable_declaration> local_variable_table_;
    mutable annotation_map annotations_;
};

bool operator==(const function& lhs, const function& rhs);
bool operator!=(const function& lhs, const function& rhs);
}

#endif