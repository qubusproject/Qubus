#include <qubus/IR/unique_variable_generator.hpp>

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/core.hpp>
#include <qubus/pattern/variable_ref.hpp>

#include <qubus/stdexcept.hpp>

#include <boost/format.hpp>

#include <utility>
#include <limits>

namespace qubus
{

unique_variable_generator::unique_variable_generator(const function& func)
: unique_variable_generator(func.body())
{
    for (const auto& param : func.params())
    {
        reserved_names_.insert(param.name());
    }

    reserved_names_.insert(func.result().name());
}

unique_variable_generator::unique_variable_generator(const expression& expr)
{
    pattern::variable<variable_declaration> decl;

    auto m = pattern::make_matcher<expression, void>()
    .case_(pattern::var(decl), [&, this]
    {
        reserved_names_.insert(decl.get().name());
    });

    pattern::for_each(expr, m);
}

variable_declaration unique_variable_generator::create_new_variable(type var_type)
{
    return create_new_variable(std::move(var_type), "var");
}

variable_declaration unique_variable_generator::create_new_variable(type var_type, std::string_view hint)
{
    auto name = generate_unique_name(hint);

    return variable_declaration(std::move(name), std::move(var_type));
}

std::string unique_variable_generator::generate_unique_name(std::string_view hint)
{
    for (long int i = 0; i <= std::numeric_limits<long int>::max(); ++i)
    {
        std::string proposed_name = str(boost::format("%1%%2%") % hint % i);

        auto [pos, inserted] = reserved_names_.insert(std::move(proposed_name));

        if (inserted)
            return *pos;
    }

    throw runtime_error("Unable to generate a new unique name.");
}

}
