#include <qbb/qubus/multi_index_handling.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/IR.hpp>

#include <boost/range/combine.hpp>
#include <boost/optional.hpp>

#include <qbb/util/assert.hpp>

#include <map>
#include <vector>

namespace qbb
{
namespace qubus
{

namespace
{

std::vector<expression>
expand_multi_index(expression expr,
                   std::map<util::handle, std::vector<expression>>& multi_index_map)
{
    using pattern::_;

    pattern::variable<variable_declaration> decl;

    auto m = pattern::make_matcher<expression, std::vector<expression>>()
                 .case_(multi_index(decl),
                        [&]
                        {
                            return multi_index_map.at(decl.get().id());
                        })
                 .case_(_, [&](const expression& self)
                        {
                            return std::vector<expression>{self};
                        });

    return pattern::match(expr, m);
}

expression expand_multi_indices(expression expr)
{
    std::map<util::handle, std::vector<expression>> multi_index_map;

    pattern::variable<std::vector<expression>> indices;
    pattern::variable<expression> tensor;

    pattern::variable<std::vector<variable_declaration>> index_decls;
    pattern::variable<variable_declaration> alias;

    pattern::variable<expression> body, a, b;

    auto m = pattern::make_matcher<expression, expression>()
                 .case_(subscription(tensor, indices),
                        [&]
                        {
                            std::vector<expression> new_indices;

                            for (const auto& index : indices.get())
                            {
                                auto expanded_indices = expand_multi_index(index, multi_index_map);

                                new_indices.insert(new_indices.end(), expanded_indices.begin(),
                                                   expanded_indices.end());
                            }

                            return subscription_expr(tensor.get(), new_indices);
                        })
//                 TODO: Reenable the code below after implementing tuples. They are needed to handle the extent.
//                 .case_(delta(a, b),
//                        [&]
//                        {
//                            auto new_indices_left = expand_multi_index(a.get(), multi_index_map);
//                            auto new_indices_right = expand_multi_index(b.get(), multi_index_map);
//
//                            if (new_indices_left.size() != new_indices_right.size())
//                                throw 0;
//
//                            boost::optional<expression> expanded_delta;
//
//                            for (const auto& index_pair :
//                                 boost::range::combine(new_indices_left, new_indices_right))
//                            {
//                                expression delta = kronecker_delta_expr(
//                                    0, boost::get<0>(index_pair), boost::get<1>(index_pair));
//
//                                if (expanded_delta)
//                                {
//                                    expanded_delta = binary_operator_expr(binary_op_tag::multiplies,
//                                                                          *expanded_delta, delta);
//                                }
//                                else
//                                {
//                                    expanded_delta = delta;
//                                };
//                            }
//
//                            QBB_ASSERT(expanded_delta, "Invalid expression.");
//
//                            return *expanded_delta;
//                        })
                 .case_(for_all_multi(index_decls, alias, body),
                        [&]
                        {
                            expression result = body.get();

                            std::vector<expression> element_indices;

                            for (const auto& decl : index_decls.get())
                            {
                                element_indices.emplace_back(variable_ref_expr(decl));

                                result = for_all_expr(decl, result);
                            }

                            multi_index_map[alias.get().id()] = element_indices;

                            return result;
                        })
                 .case_(for_all_multi(index_decls, body),
                        [&]
                        {
                            expression result = body.get();

                            for (const auto& decl : index_decls.get())
                            {
                                result = for_all_expr(decl, result);
                            }

                            return result;
                        })
                 .case_(sum_multi(body, index_decls, alias),
                        [&]
                        {
                            expression result = body.get();

                            std::vector<expression> element_indices;

                            for (const auto& decl : index_decls.get())
                            {
                                element_indices.emplace_back(variable_ref_expr(decl));

                                result = sum_expr(decl, result);
                            }

                            multi_index_map[alias.get().id()] = element_indices;

                            return result;
                        })
                 .case_(sum_multi(body, index_decls), [&]
                        {
                            expression result = body.get();

                            for (const auto& decl : index_decls.get())
                            {
                                result = sum_expr(decl, result);
                            }

                            return result;
                        });

    return pattern::substitute(expr, m);
}
}

function_declaration expand_multi_indices(function_declaration decl)
{
    auto new_body = expand_multi_indices(decl.body());

    return function_declaration(decl.name(), decl.params(), decl.result(), new_body);
}
}
}