#include <qubus/qtl/task_generator.hpp>

#include <qubus/qtl/IR/all.hpp>
#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/util/assert.hpp>

#include <algorithm>
#include <vector>

namespace qubus
{
namespace qtl
{

namespace
{

std::vector<variable_declaration> extract_params(const expression& expr)
{
    using pattern::_;

    std::vector<variable_declaration> params;

    pattern::variable<variable_declaration> decl;

    auto m = pattern::make_matcher<expression, void>()
       .case_(var(decl), [&]
       {
            if (decl.get().var_type() != types::index{})
            {
                if (std::find(params.begin(), params.end(), decl.get()) == params.end())
                {
                    params.push_back(decl.get());
                }
            }
       });

    pattern::for_each(expr, m);

    return params;
}

}

function_declaration wrap_code_in_task(std::unique_ptr<expression> expr)
{
    const expression* root = expr.get();

    while (auto for_all = root->try_as<for_all_expr>())
    {
        root = &for_all->body();
    }

    auto assignment = root->try_as<binary_operator_expr>();

    if (!assignment)
        throw 0;

    if (assignment->tag() != binary_op_tag::assign)
        throw 0;

    auto immutable_params = extract_params(assignment->right());
    auto mutable_params = extract_params(assignment->left());

    QUBUS_ASSERT(mutable_params.size() == 1, "Only one mutable param is currently allowed.");

    function_declaration func("entry", std::move(immutable_params), std::move(mutable_params[0]), std::move(expr));

    return func;
}

}
}