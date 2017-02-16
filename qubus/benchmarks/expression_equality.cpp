#include <hpx/config.hpp>

#include <nonius/nonius.h++>

#include <qbb/qubus/IR/qir.hpp>

#include <iterator>
#include <string>

void simple_expression_tree_setup()
{
    using namespace qubus;

    expression expr = integer_literal_expr(42);
    expression expr2 = integer_literal_expr(42);
}

void simple_expression_tree_compare_match()
{
    using namespace qubus;

    expression expr = integer_literal_expr(42);
    expression expr2 = integer_literal_expr(42);

    expr == expr2;
}

void simple_expression_tree_compare_mismatch()
{
    using namespace qubus;

    expression expr = integer_literal_expr(42);
    expression expr2 = integer_literal_expr(7);

    expr == expr2;
}

void complex_expression_tree_setup()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}));
    variable_declaration var2(types::array(types::double_{}));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    auto body = binary_operator_expr(binary_op_tag::plus, subscription_expr(variable_ref_expr(var1), {variable_ref_expr(i), variable_ref_expr(j)}),
                                     subscription_expr(variable_ref_expr(var2), {variable_ref_expr(i), variable_ref_expr(j)}));

    expression expr = for_expr(i, integer_literal_expr(0), integer_literal_expr(10), for_expr(j, integer_literal_expr(0), integer_literal_expr(10), body));
}

void complex_expression_tree_compare_match()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}));
    variable_declaration var2(types::array(types::double_{}));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    auto body = binary_operator_expr(binary_op_tag::plus, subscription_expr(variable_ref_expr(var1), {variable_ref_expr(i), variable_ref_expr(j)}),
                                     subscription_expr(variable_ref_expr(var2), {variable_ref_expr(i), variable_ref_expr(j)}));

    expression expr = for_expr(i, integer_literal_expr(0), integer_literal_expr(10), for_expr(j, integer_literal_expr(0), integer_literal_expr(10), body));

    expr == expr;
}

void complex_expression_tree_compare_early_mismatch()
{
    using namespace qubus;

    variable_declaration var1(types::array(types::double_{}));
    variable_declaration var2(types::array(types::double_{}));

    variable_declaration i(types::integer{});
    variable_declaration j(types::integer{});

    auto body = binary_operator_expr(binary_op_tag::plus, subscription_expr(variable_ref_expr(var1), {variable_ref_expr(i), variable_ref_expr(j)}),
                                     subscription_expr(variable_ref_expr(var2), {variable_ref_expr(i), variable_ref_expr(j)}));

    expression expr = for_expr(i, integer_literal_expr(0), integer_literal_expr(10), for_expr(j, integer_literal_expr(0), integer_literal_expr(10), body));

    expression expr2 = integer_literal_expr(42);

    expr == expr2;
}

int main() {
    nonius::configuration cfg;
    cfg.output_file = "expression_equality.html";

    nonius::benchmark benchmarks[] = {
        nonius::benchmark("Simple expression tree setup", simple_expression_tree_setup),
        nonius::benchmark("Simple expression tree compare (match)", simple_expression_tree_compare_match),
        nonius::benchmark("Simple expression tree compare (mismatch)", simple_expression_tree_compare_mismatch),
        nonius::benchmark("Complex expression tree setup", complex_expression_tree_setup),
        nonius::benchmark("Complex expression tree compare (match)", complex_expression_tree_setup),
        nonius::benchmark("Complex expression tree compare (early mismatch)", complex_expression_tree_setup)
    };

    nonius::go(cfg, std::begin(benchmarks), std::end(benchmarks), nonius::html_reporter());

    nonius::configuration cfg2;

    nonius::go(cfg2, std::begin(benchmarks), std::end(benchmarks), nonius::standard_reporter());
}
