#include <qubus/qtl/multi_index_handling.hpp>

#include <qubus/IR/qir.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/qtl/pattern/all.hpp>

#include <boost/optional.hpp>
#include <boost/range/combine.hpp>

#include <qubus/util/assert.hpp>

#include <map>
#include <vector>

namespace qubus
{
namespace qtl
{

namespace
{

std::vector<std::unique_ptr<expression>> expand_multi_index(
    const expression& expr,
    const std::map<util::handle, std::vector<std::unique_ptr<expression>>>& multi_index_map)
{
    using qubus::pattern::_;
    using qubus::pattern::variable;
    using pattern::multi_index;

    variable<variable_declaration> decl;

    auto m =
        qubus::pattern::make_matcher<expression, std::vector<std::unique_ptr<expression>>>()
            .case_(multi_index(decl), [&] { return clone(multi_index_map.at(decl.get().id())); })
            .case_(_, [&](const expression& self) {
                std::vector<std::unique_ptr<expression>> indices;
                indices.push_back(clone(self));

                return indices;
            });

    return qubus::pattern::match(expr, m);
}
}

std::unique_ptr<expression> expand_multi_indices(const expression& expr)
{
    using qubus::pattern::variable;
    using pattern::for_all_multi;
    using pattern::sum_multi;

    std::map<util::handle, std::vector<std::unique_ptr<expression>>> multi_index_map;

    variable<std::vector<std::reference_wrapper<expression>>> indices;
    variable<const access_expr&> tensor;

    variable<std::vector<variable_declaration>> index_decls;
    variable<variable_declaration> alias;

    variable<const expression &> body, a, b;

    auto m =
        qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>()
            .case_(subscription(tensor, indices),
                   [&] {
                       std::vector<std::unique_ptr<expression>> new_indices;

                       for (const auto& index : indices.get())
                       {
                           auto expanded_indices = expand_multi_index(index, multi_index_map);

                           for (auto&& index : std::move(expanded_indices))
                           {
                               new_indices.push_back(std::move(index));
                           }
                       }

                       return subscription(clone(tensor.get()), std::move(new_indices));
                   })
            //                 TODO: Reenable the code below after implementing tuples. They are
            //                 needed to handle the extent.
            //                 .case_(delta(a, b),
            //                        [&]
            //                        {
            //                            auto new_indices_left = expand_multi_index(a.get(),
            //                            multi_index_map);
            //                            auto new_indices_right = expand_multi_index(b.get(),
            //                            multi_index_map);
            //
            //                            if (new_indices_left.size() != new_indices_right.size())
            //                                throw 0;
            //
            //                            boost::optional<expression> expanded_delta;
            //
            //                            for (const auto& index_pair :
            //                                 boost::range::combine(new_indices_left,
            //                                 new_indices_right))
            //                            {
            //                                expression delta = kronecker_delta_expr(
            //                                    0, boost::get<0>(index_pair),
            //                                    boost::get<1>(index_pair));
            //
            //                                if (expanded_delta)
            //                                {
            //                                    expanded_delta =
            //                                    binary_operator_expr(binary_op_tag::multiplies,
            //                                                                          *expanded_delta,
            //                                                                          delta);
            //                                }
            //                                else
            //                                {
            //                                    expanded_delta = delta;
            //                                };
            //                            }
            //
            //                            QUBUS_ASSERT(expanded_delta, "Invalid expression.");
            //
            //                            return *expanded_delta;
            //                        })
            .case_(for_all_multi(index_decls, alias, body),
                   [&] {
                       std::unique_ptr<expression> result = clone(body.get());

                       std::vector<std::unique_ptr<expression>> element_indices;

                       for (const auto& decl : index_decls.get())
                       {
                           element_indices.emplace_back(var(decl));

                           result = for_all(decl, std::move(result));
                       }

                       multi_index_map[alias.get().id()] = std::move(element_indices);

                       return result;
                   })
            .case_(for_all_multi(index_decls, body),
                   [&] {
                       std::unique_ptr<expression> result = clone(body.get());

                       for (const auto& decl : index_decls.get())
                       {
                           result = for_all(decl, std::move(result));
                       }

                       return result;
                   })
            .case_(sum_multi(body, index_decls, alias),
                   [&] {
                       std::unique_ptr<expression> result = clone(body.get());

                       std::vector<std::unique_ptr<expression>> element_indices;

                       for (const auto& decl : index_decls.get())
                       {
                           element_indices.emplace_back(var(decl));

                           result = sum(decl, std::move(result));
                       }

                       multi_index_map[alias.get().id()] = std::move(element_indices);

                       return result;
                   })
            .case_(sum_multi(body, index_decls), [&] {
                std::unique_ptr<expression> result = clone(body.get());

                for (const auto& decl : index_decls.get())
                {
                    result = sum(decl, std::move(result));
                }

                return result;
            });

    return qubus::pattern::substitute(expr, m);
}

function_declaration expand_multi_indices(function_declaration decl)
{
    auto new_body = expand_multi_indices(decl.body());

    return function_declaration(decl.name(), decl.params(), decl.result(), std::move(new_body));
}
}
}