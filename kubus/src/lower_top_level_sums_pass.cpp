#include <qbb/kubus/lower_top_level_sums_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <vector>
#include <string>
#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

qbb::util::sparse_multi_method<expression(const qbb::util::virtual_<expression>&,
                                          std::vector<std::string>&)> remove_top_level_sums = {};

expression remove_top_level_sums_sum_expr(const sum_expr& expr,
                                          std::vector<std::string>& sum_index_ids)
{
    for (const auto& index : expr.indices())
    {
        sum_index_ids.push_back(index.as<index_expr>().id());
    }

    return remove_top_level_sums(expr.body(), sum_index_ids);
}

expression remove_top_level_sums_otherwise(const expression& expr, std::vector<std::string>&)
{
    return expr;
}

qbb::util::sparse_multi_method<expression(const qbb::util::virtual_<expression>&)>
lower_top_level_sums_ = {};

expression lower_top_level_sums_binary_op_expr(const binary_operator_expr& expr)
{
    if (expr.tag() == binary_op_tag::assign)
    {
        std::vector<std::string> sum_index_ids;

        auto new_rhs = remove_top_level_sums(expr.right(), sum_index_ids);

        auto initializer =
            binary_operator_expr(binary_op_tag::assign, expr.left(), integer_literal_expr(0));

        expression new_expr =
            binary_operator_expr(binary_op_tag::plus_assign, expr.left(), new_rhs);

        for (const auto& sum_index_id : sum_index_ids | boost::adaptors::reversed)
        {
            new_expr = for_all_expr(index_expr(sum_index_id), new_expr);
        }

        return compound_expr({std::move(initializer), std::move(new_expr)});
    }
    else
    {
        return expr;
    }
}

expression lower_top_level_sums_for_all_expr(const for_all_expr& expr)
{
    return for_all_expr(expr.index(), lower_top_level_sums_(expr.body()));
}

expression lower_top_level_sums_for_expr(const for_expr& expr)
{
    return for_expr(expr.index(), expr.lower_bound(), expr.upper_bound(), lower_top_level_sums_(expr.body()));
}

expression lower_top_level_sums_compound_expr(const compound_expr& expr)
{
    std::vector<expression> new_sub_expressions;

    for (const auto& sub_expr : expr.body())
    {
        new_sub_expressions.push_back(lower_top_level_sums_(sub_expr));
    }

    return compound_expr(new_sub_expressions);
}

void init_lower_top_level_sums()
{
    remove_top_level_sums.add_specialization(remove_top_level_sums_sum_expr);
    remove_top_level_sums.set_fallback(remove_top_level_sums_otherwise);

    lower_top_level_sums_.add_specialization(lower_top_level_sums_binary_op_expr);
    lower_top_level_sums_.add_specialization(lower_top_level_sums_for_expr);
    lower_top_level_sums_.add_specialization(lower_top_level_sums_for_all_expr);
    lower_top_level_sums_.add_specialization(lower_top_level_sums_compound_expr);
}

std::once_flag lower_top_level_sums_init_flag = {};
}

expression lower_top_level_sums(const expression& expr)
{
    std::call_once(lower_top_level_sums_init_flag, init_lower_top_level_sums);

    return lower_top_level_sums_(expr);
}
}
}