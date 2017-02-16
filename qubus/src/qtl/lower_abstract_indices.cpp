#include <qbb/qubus/qtl/lower_abstract_indices.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>
#include <qbb/qubus/pattern/substitute.hpp>

#include <qbb/qubus/qtl/pattern/all.hpp>

#include <qbb/qubus/qtl/deduce_iteration_space.hpp>

#include <qbb/util/handle.hpp>

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
    using qubus::pattern::_;
    using qubus::pattern::variable;
    using pattern::for_all;
    using pattern::index;

    variable<variable_declaration> decl;
    variable<const expression&> body;

    auto m = qubus::pattern::make_matcher<expression, std::unique_ptr<expression>>().case_(
        for_all(decl, body), [&]() -> std::unique_ptr<expression> {

            using qubus::pattern::value;

            auto bounds = deduce_iteration_space(decl.get(), body.get());

            std::unique_ptr<expression> lower_bound = std::move(bounds[0]);
            std::unique_ptr<expression> upper_bound = std::move(bounds[1]);

            variable_declaration loop_index{types::integer()};
            loop_index.annotations().add("qubus.debug.name",
                                         decl.get().annotations().lookup("qubus.debug.name"));

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
    using qubus::pattern::variable;
    using pattern::delta;

    variable<const expression &> i, j;

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

function_declaration lower_abstract_indices(function_declaration decl)
{
    auto new_body = lower_abstract_indices(decl.body());

    decl.substitute_body(std::move(new_body));

    return decl;
}
}
}