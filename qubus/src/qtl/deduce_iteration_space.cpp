#include <qbb/qubus/qtl/deduce_iteration_space.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/qubus/qtl/pattern/all.hpp>

#include <cstddef>

namespace qbb
{
namespace qubus
{
namespace qtl
{

std::array<std::unique_ptr<expression>, 2> deduce_iteration_space(const variable_declaration& idx,
                                                                  const expression& expr)
{
    using qubus::pattern::value;
    using qubus::pattern::_;
    using qubus::pattern::variable;
    using pattern::tensor;
    using pattern::index;

    variable<variable_declaration> decl;
    variable<std::size_t> index_pos;

    variable<util::index_t> extent;

    auto m =
        qubus::pattern::make_matcher<expression, std::array<std::unique_ptr<expression>, 2>>()
            .case_(subscription(tensor(decl), bind_to(any_of(index(value(idx))), index_pos)),
                   [&] {
                       auto lower_bound = integer_literal(0);

                       auto tensor = var(decl.get());
                       auto index_position = integer_literal(index_pos.get());

                       std::vector<std::unique_ptr<expression>> args;
                       args.reserve(2);
                       args.push_back(std::move(tensor));
                       args.push_back(std::move(index_position));

                       auto upper_bound = intrinsic_function("extent", std::move(args));

                       return std::array<std::unique_ptr<expression>, 2>{
                           {std::move(lower_bound), std::move(upper_bound)}};
                   })
            .case_(delta(extent, index(value(idx)), _) || delta(extent, _, index(value(idx))), [&] {
                auto lower_bound = integer_literal(0);
                auto upper_bound = integer_literal(extent.get());

                return std::array<std::unique_ptr<expression>, 2>{
                    {std::move(lower_bound), std::move(upper_bound)}};
            });

    auto result = qubus::pattern::search(expr, m);

    if (result)
    {
        return std::move(*result);
    }
    else
    {
        throw 0;
    }
}
}
}
}