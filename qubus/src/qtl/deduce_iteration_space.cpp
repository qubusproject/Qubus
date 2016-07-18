#include <qbb/qubus/qtl/deduce_iteration_space.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

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
    using pattern::value;
    using pattern::_;

    pattern::variable<variable_declaration> decl;
    pattern::variable<std::size_t> index_pos;

    pattern::variable<util::index_t> extent;

    auto m =
        pattern::make_matcher<expression, std::array<std::unique_ptr<expression>, 2>>()
            .case_(subscription(tensor(decl),
                                bind_to(any_of(index(value(idx))), index_pos)),
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

    auto result = pattern::search(expr, m);

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