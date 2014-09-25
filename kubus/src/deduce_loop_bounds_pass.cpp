#include <qbb/kubus/deduce_loop_bounds_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>

#include <utility>
#include <tuple>
#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

using index_occurence_t = std::tuple<std::shared_ptr<tensor_variable>, qbb::util::index_t>;

qbb::util::multi_method<index_occurence_t(
    const std::string&, const qbb::util::virtual_<expression>&)> find_first_occurence_of_index = {};

index_occurence_t find_first_occurence_of_index_binary_op_expr(const std::string& index_id,
                                                               const binary_operator_expr& expr)
{
    auto lhs_result = find_first_occurence_of_index(index_id, expr.left());

    if (std::get<0>(lhs_result))
    {
        return lhs_result;
    }
    else
    {
        return find_first_occurence_of_index(index_id, expr.right());
    }
}

index_occurence_t find_first_occurence_of_index_unary_op_expr(const std::string& index_id,
                                                              const unary_operator_expr& expr)
{
    return find_first_occurence_of_index(index_id, expr.arg());
}

index_occurence_t
find_first_occurence_of_index_intrinsic_function_expr(const std::string& index_id,
                                                      const intrinsic_function_expr& expr)
{
    for (const auto& arg : expr.args())
    {
        auto result = find_first_occurence_of_index(index_id, arg);

        if (std::get<0>(result))
        {
            return result;
        }
    }

    return std::make_tuple(nullptr, 0);
}

index_occurence_t
find_first_occurence_of_index_type_conversion_expr(const std::string& index_id,
                                                   const type_conversion_expr& expr)
{
    return find_first_occurence_of_index(index_id, expr.arg());
}

index_occurence_t find_first_occurence_of_index_default(const std::string&, const expression&)
{
    return std::make_tuple(nullptr, 0);
}

index_occurence_t find_first_occurence_of_index_compound_expr(const std::string& index_id,
                                                              const compound_expr& expr)
{
    for (const auto& sub_expr : expr.body())
    {
        auto result = find_first_occurence_of_index(index_id, sub_expr);

        if (std::get<0>(result))
        {
            return result;
        }
    }

    return std::make_tuple(nullptr, 0);
}

index_occurence_t find_first_occurence_of_index_for_all_expr(const std::string& index_id,
                                                             const for_all_expr& expr)
{
    return find_first_occurence_of_index(index_id, expr.body());
}

index_occurence_t find_first_occurence_of_index_for_expr(const std::string& index_id,
                                                         const for_expr& expr)
{
    return find_first_occurence_of_index(index_id, expr.body());
}

index_occurence_t find_first_occurence_of_index_sum_expr(const std::string& index_id,
                                                         const sum_expr& expr)
{
    return find_first_occurence_of_index(index_id, expr.body());
}

index_occurence_t find_first_occurence_of_index_subscription_expr(const std::string& index_id,
                                                                  const subscription_expr& expr)
{

    if (auto tensor_access = expr.indexed_expr().try_as<tensor_access_expr>())
    {
        qbb::util::index_t pos = 0;

        for (const auto& index : expr.indices())
        {
            auto temp = index.try_as<index_expr>();

            if (temp && temp->id() == index_id)
            {
                return std::make_tuple(tensor_access->variable(), pos);
            }

            ++pos;
        }
    }

    return std::make_tuple(nullptr, 0);
}

qbb::util::sparse_multi_method<expression(const qbb::util::virtual_<expression>&)>
deduce_loop_bounds_ = {};

expression deduce_loop_bounds_for_all_expr(const for_all_expr& expr)
{
    std::shared_ptr<tensor_variable> var;
    qbb::util::index_t pos;

    std::tie(var, pos) =
        find_first_occurence_of_index(expr.index().as<index_expr>().id(), expr.body());

    auto upper_bound =
        intrinsic_function_expr("extent", {tensor_access_expr(var), integer_literal_expr(pos)});
    auto lower_bound = integer_literal_expr(0);

    return for_expr(expr.index(), std::move(lower_bound), std::move(upper_bound),
                    deduce_loop_bounds_(expr.body()));
}

expression deduce_loop_bounds_for_expr(const for_expr& expr)
{
    return for_expr(expr.index(), expr.lower_bound(), expr.upper_bound(),
                    deduce_loop_bounds_(expr.body()));
}

expression deduce_loop_bounds_compound_expr(const compound_expr& expr)
{
    std::vector<expression> new_sub_expressions;

    for (const auto& sub_expr : expr.body())
    {
        new_sub_expressions.push_back(deduce_loop_bounds_(sub_expr));
    }

    return compound_expr(new_sub_expressions);
}

expression deduce_loop_bounds_default(const expression& expr)
{
    return expr;
}

void init_deduce_loop_bounds()
{
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_binary_op_expr);
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_unary_op_expr);
    find_first_occurence_of_index.add_specialization(
        find_first_occurence_of_index_intrinsic_function_expr);
    find_first_occurence_of_index.add_specialization(
        find_first_occurence_of_index_type_conversion_expr);
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_for_all_expr);
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_for_expr);
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_sum_expr);
    find_first_occurence_of_index.add_specialization(find_first_occurence_of_index_compound_expr);
    find_first_occurence_of_index.add_specialization(
        find_first_occurence_of_index_subscription_expr);

    find_first_occurence_of_index.set_fallback(find_first_occurence_of_index_default);

    deduce_loop_bounds_.add_specialization(deduce_loop_bounds_for_all_expr);
    deduce_loop_bounds_.add_specialization(deduce_loop_bounds_for_expr);
    deduce_loop_bounds_.add_specialization(deduce_loop_bounds_compound_expr);

    deduce_loop_bounds_.set_fallback(deduce_loop_bounds_default);
}

std::once_flag deduce_loop_bounds_init_flag = {};
}

expression deduce_loop_bounds(const expression& expr)
{
    std::call_once(deduce_loop_bounds_init_flag, init_deduce_loop_bounds);

    return deduce_loop_bounds_(expr);
}
}
}