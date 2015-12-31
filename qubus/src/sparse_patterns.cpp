#include <qbb/qubus/sparse_patterns.hpp>

#include <qbb/qubus/deduce_iteration_space.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <boost/optional.hpp>

#include <vector>
#include <tuple>
#include <algorithm>

namespace qbb
{
namespace qubus
{

namespace
{

template <typename Iterator, typename T>
bool contains(Iterator first, Iterator last, const T& value)
{
    return std::find(first, last, value) != last;
}

expression generate_init_part(expression lhs)
{
    using pattern::_;

    pattern::variable<variable_declaration> var;
    pattern::variable<std::vector<variable_declaration>> indices;

    auto m = pattern::make_matcher<expression, void>().case_(
        pattern::protect(
            subscription(variable_ref(var), bind_to(all_of(pattern::index()), indices))),
        [&]
        {
        });

    pattern::match(lhs, m);

    std::vector<variable_declaration> init_indices;
    std::vector<expression> init_index_refs;
    std::vector<std::array<expression, 2>> bounds;

    for (std::size_t i = 0; i < indices.get().size(); ++i)
    {
        init_indices.emplace_back(types::integer());
        init_index_refs.push_back(variable_ref_expr(init_indices.back()));
        bounds.push_back(deduce_iteration_space(indices.get()[i], lhs));
    }

    expression init_lhs = binary_operator_expr(
        binary_op_tag::assign, subscription_expr(variable_ref_expr(var.get()), init_index_refs),
        integer_literal_expr(0));

    for (std::size_t i = init_indices.size(); i-- > 0;)
    {
        init_lhs = for_expr(init_indices[i], bounds[i][0], bounds[i][1], init_lhs);
    }

    return init_lhs;
}

enum class computational_part_kind
{
    vectorizable,
    remainder
};

expression generate_computational_part(expression current_expr,
                                       const variable_declaration& the_sparse_tensor,
                                       const std::vector<variable_declaration>& dense_indices,
                                       const std::vector<variable_declaration>& sparse_indices,
                                       computational_part_kind kind)
{
    using pattern::_;

    pattern::variable<variable_declaration> idx;

    expression data = member_access_expr(variable_ref_expr(the_sparse_tensor), "data");

    variable_declaration new_body{types::unknown()};

    variable_declaration i{types::integer()};
    variable_declaration ii{types::integer()};

    util::index_t block_width = 4;

    expression innermost_dense_extent =
        subscription_expr(member_access_expr(variable_ref_expr(the_sparse_tensor), "shape"),
                          {integer_literal_expr(0)});

    expression skeleton;

    if (kind == computational_part_kind::vectorizable)
    {
        skeleton = for_expr(ii, integer_literal_expr(0), integer_literal_expr(block_width),
                            variable_ref_expr(new_body));
    }
    else if (kind == computational_part_kind::remainder)
    {
        skeleton = for_expr(ii, integer_literal_expr(0),
                            binary_operator_expr(binary_op_tag::modulus, innermost_dense_extent,
                                                 integer_literal_expr(block_width)),
                            variable_ref_expr(new_body));
    }

    expression block_index = binary_operator_expr(binary_op_tag::divides, variable_ref_expr(i),
                                                  integer_literal_expr(block_width));

    variable_declaration j{types::integer()};
    skeleton = for_expr(j, integer_literal_expr(0),
                        subscription_expr(member_access_expr(data, "cl"), {block_index}), skeleton);

    expression innermost_dense_extent_rounded =
        binary_operator_expr(binary_op_tag::multiplies,
                             binary_operator_expr(binary_op_tag::divides, innermost_dense_extent,
                                                  integer_literal_expr(block_width)),
                             integer_literal_expr(block_width));

    if (kind == computational_part_kind::vectorizable)
    {
        skeleton = for_expr(i, integer_literal_expr(0), innermost_dense_extent_rounded,
                            integer_literal_expr(block_width), skeleton);
    }
    else if (kind == computational_part_kind::remainder)
    {
        skeleton = for_expr(i, innermost_dense_extent_rounded, innermost_dense_extent,
                            integer_literal_expr(block_width), skeleton);
    }

    std::map<variable_declaration, expression> index_map;

    for (auto iter = dense_indices.begin(), end = dense_indices.end(); iter != end; ++iter)
    {
        variable_declaration idx{types::integer()};

        auto bounds = deduce_iteration_space(*iter, current_expr);

        skeleton = for_expr(idx, bounds[0], bounds[1], skeleton);

        index_map.emplace(*iter, variable_ref_expr(idx));
    }

    auto cs = member_access_expr(data, "cs");
    auto col = member_access_expr(data, "col");
    auto val = member_access_expr(data, "val");

    expression outer_indices_linearized = integer_literal_expr(0);

    for (auto iter = sparse_indices.begin(), end = sparse_indices.end() - 2; iter != end; ++iter)
    {
        variable_declaration idx{types::integer()};

        skeleton = for_expr(idx, integer_literal_expr(0), integer_literal_expr(16), skeleton);

        index_map.emplace(*iter, variable_ref_expr(idx));

        outer_indices_linearized = binary_operator_expr(
            binary_op_tag::plus,
            binary_operator_expr(binary_op_tag::multiplies, outer_indices_linearized,
                                 integer_literal_expr(16)),
            variable_ref_expr(idx));
    }

    expression block_idx = binary_operator_expr(
        binary_op_tag::divides,
        binary_operator_expr(binary_op_tag::plus, binary_operator_expr(binary_op_tag::multiplies,
                                                                       outer_indices_linearized,
                                                                       integer_literal_expr(16)),
                             variable_ref_expr(i)),
        integer_literal_expr(block_width));

    auto sparse_idx = binary_operator_expr(
        binary_op_tag::plus, subscription_expr(cs, {block_idx}),
        binary_operator_expr(binary_op_tag::plus,
                             binary_operator_expr(binary_op_tag::multiplies, variable_ref_expr(j),
                                                  integer_literal_expr(block_width)),
                             variable_ref_expr(ii)));

    pattern::variable<expression> lhs, rhs;

    auto m3 = pattern::make_matcher<expression, expression>().case_(
        subscription(pattern::sparse_tensor(pattern::value(the_sparse_tensor)), _), [&]
        {
            return subscription_expr(val, {sparse_idx});
        });

    current_expr = pattern::substitute(current_expr, m3);

    auto sparse_index = subscription_expr(col, {sparse_idx});
    auto innermost_dense_index = expression(
        binary_operator_expr(binary_op_tag::plus, variable_ref_expr(i), variable_ref_expr(ii)));

    index_map.emplace(*(sparse_indices.end() - 2), innermost_dense_index);
    index_map.emplace(*(sparse_indices.end() - 1), sparse_index);

    auto m4 =
        pattern::make_matcher<expression, expression>().case_(index(idx), [&]
                                                              {
                                                                  return index_map.at(idx.get());
                                                              });

    current_expr = pattern::substitute(current_expr, m4);

    auto new_code = expand_macro(macro_expr({new_body}, skeleton), {current_expr});

    return new_code;
}

/** \brief A simple expression analyzer for sparse patterns.
*/
class simple_sparse_pattern_analysis
{
public:
    explicit simple_sparse_pattern_analysis(const expression& expr_) : is_valid_(false)
    {
        auto tensor_def = collect_outer_indices(expr_);

        auto rhs = decompose_definition(tensor_def);

        if (!rhs)
            return;

        auto body = collect_top_level_contractions(*rhs);

        if (!body)
            return;

        this->body = *body;

        is_valid_ = extract_sparse_tensor(this->body);

        auto new_end = std::remove_if(
            dense_indices.begin(), dense_indices.end(), [&](const variable_declaration& value)
            {
                return contains(sparse_indices.begin(), sparse_indices.end(), value);
            });

        dense_indices.erase(new_end, dense_indices.end());
    }

    explicit operator bool() const
    {
        return is_valid_;
    }

    std::vector<variable_declaration> dense_indices;
    std::vector<variable_declaration> sparse_indices;
    expression lhs;
    expression body;
    variable_declaration the_sparse_tensor;

private:
    bool is_valid_;

    expression collect_outer_indices(const expression& expr)
    {
        pattern::variable<variable_declaration> idx;
        pattern::variable<expression> body;

        expression current_expr = expr;

        auto m =
            pattern::make_matcher<expression, void>().case_(for_all(idx, body), [&, this]
                                                            {
                                                                current_expr = body.get();

                                                                dense_indices.push_back(idx.get());
                                                            });

        pattern::for_each(expr, m);

        return current_expr;
    }

    boost::optional<expression> decompose_definition(const expression& tensor_def)
    {
        pattern::variable<expression> lhs, rhs;

        auto m = pattern::make_matcher<expression, void>().case_(
            binary_operator(pattern::value(binary_op_tag::assign), lhs, protect(rhs)), [&, this]
            {
                this->lhs = lhs.get();
            });

        auto result = pattern::try_match(tensor_def, m);

        if (result)
        {
            return rhs.get();
        }
        else
        {
            return boost::none;
        }
    }

    boost::optional<expression> collect_top_level_contractions(const expression& rhs)
    {
        pattern::variable<variable_declaration> idx;
        pattern::variable<expression> body;

        expression current_expr = rhs;

        bool contains_contractions = false;

        auto m =
            pattern::make_matcher<expression, void>().case_(sum(body, idx), [&, this]
                                                            {
                                                                current_expr = body.get();

                                                                dense_indices.push_back(idx.get());

                                                                contains_contractions = true;
                                                            });

        pattern::for_each(rhs, m);

        if (contains_contractions)
        {
            return current_expr;
        }
        else
        {
            return boost::none;
        }
    }

    bool extract_sparse_tensor(const expression& body)
    {
        pattern::variable<variable_declaration> var;
        pattern::variable<std::vector<variable_declaration>> indices;

        boost::optional<variable_declaration> the_sparse_tensor;
        bool num_of_sparse_tensors = 0;

        auto m = pattern::make_matcher<expression, void>().case_(
            subscription(pattern::sparse_tensor(var), bind_to(all_of(pattern::index()), indices)),
            [&, this]
            {
                ++num_of_sparse_tensors;

                if (the_sparse_tensor)
                    return;

                the_sparse_tensor = var.get();
                for (const auto& idx : indices.get())
                {
                    sparse_indices.push_back(idx);
                }
            });

        pattern::for_each(body, m);

        if (the_sparse_tensor && num_of_sparse_tensors == 1)
        {
            this->the_sparse_tensor = *the_sparse_tensor;

            return true;
        }
        else
        {
            return false;
        }
    }
};
}

expression lower_sparse_contraction(expression expr)
{
    simple_sparse_pattern_analysis analysis(expr);

    if (!analysis)
        throw 0; // error: Invalid sparse pattern.

    auto rhs = binary_operator_expr(binary_op_tag::plus_assign, analysis.lhs, analysis.body);

    auto init_lhs_part = generate_init_part(analysis.lhs);

    auto vectorizable_part =
        generate_computational_part(rhs, analysis.the_sparse_tensor, analysis.dense_indices,
                                    analysis.sparse_indices, computational_part_kind::vectorizable);

    auto remainder_part =
        generate_computational_part(rhs, analysis.the_sparse_tensor, analysis.dense_indices,
                                    analysis.sparse_indices, computational_part_kind::remainder);

    auto new_code = compound_expr({init_lhs_part, vectorizable_part, remainder_part});

    return new_code;
}

expression optimize_sparse_patterns(expression expr)
{
    return lower_sparse_contraction(expr);
}

function_declaration optimize_sparse_patterns(function_declaration decl)
{
    auto new_body = optimize_sparse_patterns(decl.body());

    decl.substitute_body(std::move(new_body));

    return decl;
}
}
}