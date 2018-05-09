#include <qubus/qtl/object_extraction_pass.hpp>

#include <qubus/IR/unique_variable_generator.hpp>
#include <qubus/qtl/pattern/object.hpp>

#include <qubus/pattern/matcher.hpp>
#include <qubus/pattern/substitute.hpp>
#include <qubus/pattern/variable.hpp>

#include <algorithm>

namespace qubus
{
namespace qtl
{
std::unique_ptr<expression>
extract_objects(const expression& expr,
                std::vector<std::tuple<variable_declaration, object>>& parameter_map)
{
    ::qubus::pattern::variable<object> object;

    unique_variable_generator var_gen;

    auto m = ::qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        pattern::obj(object), [&] {
            auto search_result =
                std::find_if(parameter_map.begin(), parameter_map.end(),
                             [&object](const auto& entry) { return std::get<1>(entry) == object.get(); });

            if (search_result == parameter_map.end())
            {
                auto type = object.get().object_type();

                variable_declaration variable = var_gen.create_new_variable(std::move(type), "obj");

                parameter_map.push_back(std::make_tuple(variable, object.get()));

                return var(std::move(variable));
            }
            else
            {
                return var(std::get<0>(*search_result));
            }
        });

    return ::qubus::pattern::substitute(expr, m);
}
}
}
