#include <qubus/IR/function.hpp>

#include <qubus/IR/module.hpp>
#include <qubus/IR/type_inference.hpp>

#include <limits>
#include <utility>

namespace qubus
{

function::function(module& containing_module_, std::string name_,
                   std::vector<std::shared_ptr<const variable_declaration>> params_,
                   std::shared_ptr<const variable_declaration> result_,
                   std::unique_ptr<expression> body_)
: containing_module_(&containing_module_),
  name_(std::move(name_)),
  params_(std::move(params_)),
  result_(std::move(result_)),
  body_(std::move(body_))
{
}

const std::string& function::name() const
{
    return name_;
}

std::string function::full_name() const
{
    return containing_module().id().string() + "." + name();
}

util::handle function::id() const
{
    return util::handle_from_ptr(this);
}

const std::vector<std::shared_ptr<const variable_declaration>>& function::params() const
{
    return params_;
}

std::shared_ptr<const variable_declaration> function::result() const
{
    return result_;
}

std::size_t function::arity() const
{
    return params_.size();
}

const expression& function::body() const
{
    return *body_;
}

module& function::containing_module() const
{
    return *containing_module_;
}

void function::substitute_body(std::unique_ptr<expression> body)
{
    body_ = std::move(body);
}

annotation_map& function::annotations() const
{
    return annotations_;
}

annotation_map& function::annotations()
{
    return annotations_;
}

std::shared_ptr<const variable_declaration> function::declare_new_local_variable(type datatype)
{
    return declare_new_local_variable(std::move(datatype), "var");
}

std::shared_ptr<const variable_declaration>
function::declare_new_local_variable(type datatype, const std::string& hint)
{
    long int index = 0;

    constexpr auto largest_index = std::numeric_limits<long int>::max();

    for (;;)
    {
        auto var_name = hint + std::to_string(index);

        auto search_result = local_variable_table_.find(var_name);

        if (search_result == local_variable_table_.end())
        {
            auto var = std::make_shared<variable_declaration>(var_name, std::move(datatype));

            local_variable_table_.emplace(std::move(var_name), var);

            return var;
        }

        if (index == largest_index)
            throw 0;

        ++index;
    }
}

bool operator==(const function& lhs, const function& rhs)
{
    return lhs.id() == rhs.id();
}

bool operator!=(const function& lhs, const function& rhs)
{
    return !(lhs == rhs);
}
}