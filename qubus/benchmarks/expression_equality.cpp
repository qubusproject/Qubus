#include <hpx/config.hpp>

#include <nonius/nonius.h++>

#include <qubus/IR/qir.hpp>

#include <iterator>
#include <string>

void simple_expression_tree_setup()
{
    using namespace qubus;

    auto expr = integer_literal(42);
    auto expr2 = integer_literal(42);
}

void simple_expression_tree_compare_match()
{
    using namespace qubus;

    auto expr = integer_literal(42);
    auto expr2 = integer_literal(42);

    *expr == *expr2;
}

void simple_expression_tree_compare_mismatch()
{
    using namespace qubus;

    auto expr = integer_literal(42);
    auto expr2 = integer_literal(7);

    *expr == *expr2;
}

void complex_expression_tree_setup()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}, 2));
    variable_declaration var2(types::array(types::double_{}, 2));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));
    indices.push_back(var(j));

    auto body =
        subscription(var(var1), clone(indices)) + subscription(var(var2), clone(indices));

    auto expr = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(0), integer_literal(10), std::move(body)));
}

void complex_expression_tree_compare_match()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}, 2));
    variable_declaration var2(types::array(types::double_{}, 2));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));
    indices.push_back(var(j));

    auto body =
            subscription(var(var1), clone(indices)) + subscription(var(var2), clone(indices));

    auto expr = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(0), integer_literal(10), std::move(body)));

    *expr == *expr;
}

void complex_expression_tree_compare_early_mismatch()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}, 2));
    variable_declaration var2(types::array(types::double_{}, 2));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    std::vector<std::unique_ptr<expression>> indices;
    indices.push_back(var(i));
    indices.push_back(var(j));

    auto body =
            subscription(var(var1), clone(indices)) + subscription(var(var2), clone(indices));

    auto expr = for_(i, integer_literal(0), integer_literal(10),
                     for_(j, integer_literal(0), integer_literal(10), std::move(body)));

    auto expr2 = integer_literal(42);

    *expr == *expr2;
}

int main()
{
    nonius::configuration cfg;
    cfg.output_file = "expression_equality.html";

    nonius::benchmark benchmarks[] = {
        nonius::benchmark("Simple expression tree setup", simple_expression_tree_setup),
        nonius::benchmark("Simple expression tree compare (match)",
                          simple_expression_tree_compare_match),
        nonius::benchmark("Simple expression tree compare (mismatch)",
                          simple_expression_tree_compare_mismatch),
        nonius::benchmark("Complex expression tree setup", complex_expression_tree_setup),
        nonius::benchmark("Complex expression tree compare (match)", complex_expression_tree_setup),
        nonius::benchmark("Complex expression tree compare (early mismatch)",
                          complex_expression_tree_setup)};

    nonius::go(cfg, std::begin(benchmarks), std::end(benchmarks), nonius::html_reporter());

    nonius::configuration cfg2;

    nonius::go(cfg2, std::begin(benchmarks), std::end(benchmarks), nonius::standard_reporter());
}
