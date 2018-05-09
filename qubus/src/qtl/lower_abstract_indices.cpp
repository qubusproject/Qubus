#include <qubus/qtl/lower_abstract_indices.hpp>

#include <qubus/IR/qir.hpp>
#include <qubus/IR/unique_variable_generator.hpp>
#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>
#include <qubus/pattern/substitute.hpp>

#include <qubus/qtl/pattern/all.hpp>

#include <qubus/qtl/deduce_iteration_space.hpp>

#include <qubus/util/handle.hpp>

#include <boost/optional.hpp>

#include <array>
#include <utility>

namespace qubus
{
namespace qtl
{

namespace
{

std::unique_ptr<expression> lower_abstract_indices_impl(const expression& expr)
{
    using pattern::for_all;
    using pattern::index;
    using qubus::pattern::_;
    using qubus::pattern::variable;

    variable<variable_declaration> decl;
    variable<const expression&> body;

    unique_variable_generator var_gen;

    auto m = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        for_all(decl, body), [&]() -> std::unique_ptr<expression> {

            using qubus::pattern::value;

            auto bounds = deduce_iteration_space(decl.get(), body.get());

            std::unique_ptr<expression> lower_bound = std::move(bounds[0]);
            std::unique_ptr<expression> upper_bound = std::move(bounds[1]);

            variable_declaration loop_index = var_gen.create_new_variable(types::integer(), decl.get().name());

            auto m2 = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
                index(protect(decl)), [&] { return var(loop_index); });

            auto new_body = qubus::pattern::substitute(body.get(), m2);

            return for_(std::move(loop_index), std::move(lower_bound), std::move(upper_bound),
                        std::move(new_body));
        });

    return qubus::pattern::substitute(expr, m);
}

std::unique_ptr<expression> fixup_kronecker_deltas(const expression& expr)
{
    using pattern::delta;
    using qubus::pattern::variable;

    variable<const expression&> i, j;

    auto m = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        delta(i, j), [&] {
            std::vector<std::unique_ptr<expression>> args;
            args.reserve(2);
            args.push_back(clone(i.get()));
            args.push_back(clone(j.get()));

            return intrinsic_function("delta", std::move(args));
        });

    return qubus::pattern::substitute(expr, m);
}
}

std::unique_ptr<expression> lower_abstract_indices(const expression& expr)
{
    auto lowered_expr = lower_abstract_indices_impl(expr);

    return fixup_kronecker_deltas(*lowered_expr);
}
}
}