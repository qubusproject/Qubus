#include <qbb/qubus/qtl/sparse_patterns.hpp>

#include <qbb/qubus/qtl/deduce_iteration_space.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/qubus/qtl/pattern/all.hpp>

#include <qbb/util/observer_ptr.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <tuple>
#include <vector>

namespace qubus
{
namespace qtl
{

namespace
{

template <typename Iterator, typename T>
bool contains(Iterator first, Iterator last, const T& value)
{
    return std::find(first, last, value) != last;
}

std::unique_ptr<expression> generate_init_part(const expression& lhs)
{
    using qubus::pattern::_;
    using qubus::pattern::variable;
    using pattern::index;

    variable<variable_declaration> var;
    variable<std::vector<variable_declaration>> indices;

    auto m = qubus::pattern::make_matcher<expression, void>().case_(
        qubus::pattern::protect(subscription(variable_ref(var), bind_to(all_of(index()), indices))),
        [&] {});

    qubus::pattern::match(lhs, m);

    std::vector<variable_declaration> init_indices;
    std::vector<std::unique_ptr<expression>> init_index_refs;
    std::vector<std::array<std::unique_ptr<expression>, 2>> bounds;

    for (std::size_t i = 0; i < indices.get().size(); ++i)
    {
        init_indices.emplace_back(types::integer());
        init_index_refs.push_back(qubus::var(init_indices.back()));
        bounds.push_back(deduce_iteration_space(indices.get()[i], lhs));
    }

    std::unique_ptr<expression> init_lhs = assign(
        subscription(qubus::var(var.get()), std::move(init_index_refs)), integer_literal(0));

    for (std::size_t i = init_indices.size(); i-- > 0;)
    {
        init_lhs = for_(std::move(init_indices[i]), std::move(bounds[i][0]),
                        std::move(bounds[i][1]), std::move(init_lhs));
    }

    return init_lhs;
}

enum class computational_part_kind
{
    vectorizable,
    remainder
};

std::unique_ptr<expression> generate_computational_part(
    std::unique_ptr<expression> current_expr, const variable_declaration& the_sparse_tensor,
    const std::vector<variable_declaration>& dense_indices,
    const std::vector<variable_declaration>& sparse_indices, computational_part_kind kind)
{
    using qubus::pattern::_;
    using qubus::pattern::variable;
    using pattern::index;

    variable<variable_declaration> idx;

    auto data = member_access(var(the_sparse_tensor), "data");

    variable_declaration new_body{types::unknown()};

    variable_declaration i{types::integer()};
    variable_declaration ii{types::integer()};

    util::index_t block_width = 4;

    auto innermost_dense_extent =
        subscription(member_access(var(the_sparse_tensor), "shape"), integer_literal(0));

    std::unique_ptr<expression> skeleton;

    if (kind == computational_part_kind::vectorizable)
    {
        skeleton = for_(ii, integer_literal(0), integer_literal(block_width), var(new_body));
    }
    else if (kind == computational_part_kind::remainder)
    {
        skeleton =
            for_(ii, integer_literal(0),
                 clone(*innermost_dense_extent) % integer_literal(block_width), var(new_body));
    }

    auto block_index = var(i) / integer_literal(block_width);

    variable_declaration j{types::integer()};
    skeleton = for_(j, integer_literal(0),
                    subscription(member_access(clone(*data), "cl"), clone(*block_index)),
                    std::move(skeleton));

    auto innermost_dense_extent_rounded =
        (clone(*innermost_dense_extent) / integer_literal(block_width)) *
        integer_literal(block_width);

    if (kind == computational_part_kind::vectorizable)
    {
        skeleton = for_(i, integer_literal(0), clone(*innermost_dense_extent_rounded),
                        integer_literal(block_width), std::move(skeleton));
    }
    else if (kind == computational_part_kind::remainder)
    {
        skeleton = for_(i, clone(*innermost_dense_extent_rounded), clone(*innermost_dense_extent),
                        integer_literal(block_width), std::move(skeleton));
    }

    std::map<variable_declaration, std::unique_ptr<expression>> index_map;

    for (auto iter = dense_indices.begin(), end = dense_indices.end(); iter != end; ++iter)
    {
        variable_declaration idx{types::integer()};

        auto bounds = deduce_iteration_space(*iter, *current_expr);

        skeleton = for_(idx, std::move(bounds[0]), std::move(bounds[1]), std::move(skeleton));

        index_map.emplace(*iter, var(idx));
    }

    auto cs = member_access(clone(*data), "cs");
    auto col = member_access(clone(*data), "col");
    auto val = member_access(clone(*data), "val");

    std::unique_ptr<expression> outer_indices_linearized = integer_literal(0);

    for (auto iter = sparse_indices.begin(), end = sparse_indices.end() - 2; iter != end; ++iter)
    {
        variable_declaration idx{types::integer()};

        skeleton = for_(idx, integer_literal(0), integer_literal(16), std::move(skeleton));

        index_map.emplace(*iter, var(idx));

        outer_indices_linearized =
            std::move(outer_indices_linearized) * integer_literal(16) + var(idx);
    }

    auto block_idx = (std::move(outer_indices_linearized) * integer_literal(16) + var(i)) /
                     integer_literal(block_width);

    auto sparse_idx = subscription(std::move(cs), std::move(block_idx)) +
                      var(j) * integer_literal(block_width) + var(ii);

    // pattern::variable<expression_old> lhs, rhs;

    auto m3 = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        subscription(pattern::sparse_tensor(qubus::pattern::value(the_sparse_tensor)), _),
        [&] { return subscription(std::move(val), clone(*sparse_idx)); });

    current_expr = qubus::pattern::substitute(*current_expr, m3);

    auto sparse_index = subscription(std::move(col), clone(*sparse_idx));
    auto innermost_dense_index = var(i) + var(ii);

    index_map.emplace(*(sparse_indices.end() - 2), std::move(innermost_dense_index));
    index_map.emplace(*(sparse_indices.end() - 1), std::move(sparse_index));

    auto m4 = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        index(idx), [&] { return clone(*index_map.at(idx.get())); });

    current_expr = qubus::pattern::substitute(*current_expr, m4);

    std::vector<std::unique_ptr<expression>> macro_args;
    macro_args.reserve(1);
    macro_args.push_back(std::move(current_expr));

    auto new_code =
        expand_macro(*make_macro({new_body}, std::move(skeleton)), std::move(macro_args));

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

        this->body.reset(&body->get());

        is_valid_ = extract_sparse_tensor(*this->body);

        auto new_end = std::remove_if(
            dense_indices.begin(), dense_indices.end(), [&](const variable_declaration& value) {
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
    util::observer_ptr<const expression> lhs;
    util::observer_ptr<const expression> body;
    variable_declaration the_sparse_tensor;

private:
    bool is_valid_;

    std::reference_wrapper<const expression> collect_outer_indices(const expression& expr)
    {
        using qubus::pattern::variable;
        using pattern::for_all;

        variable<variable_declaration> idx;
        variable<const expression&> body;

        std::reference_wrapper<const expression> current_expr = expr;

        auto m = qubus::pattern::make_matcher<expression, void>().case_(for_all(idx, body), [&, this] {
            current_expr = body.get();

            dense_indices.push_back(idx.get());
        });

        qubus::pattern::for_each(expr, m);

        return current_expr;
    }

    boost::optional<std::reference_wrapper<const expression>>
    decompose_definition(const expression& tensor_def)
    {
        using qubus::pattern::variable;

        variable<const expression &> lhs, rhs;

        auto m = qubus::pattern::make_matcher<expression, void>().case_(
            binary_operator(qubus::pattern::value(binary_op_tag::assign), lhs, protect(rhs)),
            [&, this] { this->lhs.reset(&lhs.get()); });

        auto result = qubus::pattern::try_match(tensor_def, m);

        if (result)
        {
            return std::cref(rhs.get());
        }
        else
        {
            return boost::none;
        }
    }

    boost::optional<std::reference_wrapper<const expression>>
    collect_top_level_contractions(const expression& rhs)
    {
        using qubus::pattern::variable;
        using pattern::sum;

        variable<variable_declaration> idx;
        variable<const expression&> body;

        std::reference_wrapper<const expression> current_expr = rhs;

        bool contains_contractions = false;

        auto m = qubus::pattern::make_matcher<expression, void>().case_(sum(body, idx), [&, this] {
            current_expr = body.get();

            dense_indices.push_back(idx.get());

            contains_contractions = true;
        });

        qubus::pattern::for_each(rhs, m);

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
        using qubus::pattern::variable;

        variable<variable_declaration> var;
        variable<std::vector<variable_declaration>> indices;

        boost::optional<variable_declaration> the_sparse_tensor;
        std::size_t num_of_sparse_tensors = 0;

        auto m = qubus::pattern::make_matcher<expression, void>().case_(
            subscription(pattern::sparse_tensor(var), bind_to(all_of(pattern::index()), indices)),
            [&, this] {
                ++num_of_sparse_tensors;

                if (the_sparse_tensor)
                    return;

                the_sparse_tensor = var.get();
                for (const auto& idx : indices.get())
                {
                    sparse_indices.push_back(idx);
                }
            });

        qubus::pattern::for_each(body, m);

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

std::unique_ptr<expression> lower_sparse_contraction(const expression& expr)
{
    simple_sparse_pattern_analysis analysis(expr);

    // TODO: Test if the expression is sparse but otherwise invalid. In this case throw an error.
    if (!analysis)
        return clone(expr);

    auto rhs = plus_assign(clone(*analysis.lhs), clone(*analysis.body));

    auto init_lhs_part = generate_init_part(*analysis.lhs);

    auto vectorizable_part =
        generate_computational_part(clone(*rhs), analysis.the_sparse_tensor, analysis.dense_indices,
                                    analysis.sparse_indices, computational_part_kind::vectorizable);

    auto remainder_part = generate_computational_part(
        std::move(rhs), analysis.the_sparse_tensor, analysis.dense_indices, analysis.sparse_indices,
        computational_part_kind::remainder);

    std::vector<std::unique_ptr<expression>> tasks;
    tasks.push_back(std::move(init_lhs_part));
    tasks.push_back(std::move(vectorizable_part));
    tasks.push_back(std::move(remainder_part));

    std::unique_ptr<expression> new_code = sequenced_tasks(std::move(tasks));

    return new_code;
}

std::unique_ptr<expression> optimize_sparse_patterns(const expression& expr)
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