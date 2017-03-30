#include <qubus/qtl/task_generator.hpp>

#include <qubus/variable_access_analysis.hpp>

#include <qubus/util/assert.hpp>

#include <algorithm>
#include <vector>

namespace qubus
{
namespace qtl
{

namespace
{
template <typename Range, typename T>
bool contains(const Range& range, const T& value)
{
    return std::find(range.begin(), range.end(), value) != range.end();
}
}

function_declaration wrap_code_in_task(std::unique_ptr<expression> expr)
{
    pass_resource_manager resource_man;
    analysis_manager analysis_man(resource_man);

    variable_access_analysis analysis;

    auto result = analysis.run(*expr, analysis_man, resource_man);

    auto access_set = result.query_accesses_for_location(*expr);

    std::vector<variable_declaration> mutable_params;

    for (const auto& access : access_set.get_write_accesses())
    {
        if (!contains(mutable_params, access.variable()))
        {
            mutable_params.push_back(access.variable());
        }
    }

    std::vector<variable_declaration> immutable_params;

    for (const auto& access : access_set.get_read_accesses())
    {
        if (!contains(mutable_params, access.variable()) &&
            !contains(immutable_params, access.variable()))
        {
            immutable_params.push_back(access.variable());
        }
    }

    QUBUS_ASSERT(mutable_params.size() == 1, "Only one mutable param is currently allowed.");

    function_declaration func("entry", std::move(immutable_params), std::move(mutable_params[0]),
                              std::move(expr));

    return func;
}
}
}