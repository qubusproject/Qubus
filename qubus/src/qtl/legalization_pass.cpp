#include <qubus/qtl/legalization_pass.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>
#include <qubus/qtl/pattern/index.hpp>
#include <qubus/qtl/pattern/tensor.hpp>
#include <qubus/qtl/pattern/captured_multi_index.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/qtl/IR/all.hpp>

#include <boost/optional.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace qubus
{
namespace qtl
{

namespace
{
struct index_info
{
    explicit index_info(variable_declaration index) : index(std::move(index))
    {
    }

    index_info(variable_declaration index, std::vector<variable_declaration> element_indices)
    : index(std::move(index)), element_indices(std::move(element_indices))
    {
    }

    variable_declaration index;
    boost::optional<std::vector<variable_declaration>> element_indices;
};
}

std::unique_ptr<expression> legalize_expression(const expression& expr)
{
    using ::qubus::pattern::_;

    ::qubus::pattern::variable<std::vector<std::reference_wrapper<expression>>> indices;

    auto m = ::qubus::pattern::make_matcher<expression, void>().case_(
        assign(subscription(_, protect(indices)), _), [] {});

    if (!::qubus::pattern::try_match(expr, m))
        throw 0;

    std::vector<index_info> outer_indices;

    for (const auto& expr : indices.get())
    {
        if (auto index = expr.get().try_as<variable_ref_expr>())
        {
            auto index_var = index->declaration();

            auto search_result = std::find_if(
                outer_indices.begin(), outer_indices.end(),
                [&index_var](const index_info& entry) { return entry.index == index_var; });

            if (search_result == outer_indices.end())
            {
                outer_indices.push_back(index_info(std::move(index_var)));
            }
        }
        else if (auto multi_index = expr.get().try_as<multi_index_expr>())
        {
            auto index_var = multi_index->multi_index();

            auto search_result = std::find_if(
                outer_indices.begin(), outer_indices.end(),
                [&index_var](const index_info& entry) { return entry.index == index_var; });

            if (search_result == outer_indices.end())
            {
                auto element_indices = multi_index->element_indices();

                outer_indices.push_back(
                    index_info(std::move(index_var), std::move(element_indices)));
            }
        }
        else
        {
            throw 0;
        }
    }

    auto root = clone(expr);

    for (auto& idx : outer_indices | boost::adaptors::reversed)
    {
        if (idx.element_indices)
        {
            root = for_all(*std::move(idx.element_indices), std::move(idx.index), std::move(root));
        }
        else
        {
            root = for_all(std::move(idx.index), std::move(root));
        }
    }

    ::qubus::pattern::variable<variable_declaration> idx;

    auto m3 = ::qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        pattern::captured_multi_index(idx, _), [&] { return var(idx.get()); });

    root = ::qubus::pattern::substitute(*root, m3);

    return root;
}
}
}
