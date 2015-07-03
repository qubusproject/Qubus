#include <qbb/kubus/kronecker_delta_folding_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>
#include <qbb/util/multi_method.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/optional.hpp>

#include <mutex>
#include <vector>
#include <string>
#include <tuple>
#include <iterator>
#include <algorithm>
#include <utility>

namespace qbb
{
namespace qubus
{

namespace
{

using bubble_t = std::tuple<std::string, std::string>;

bubble_t make_bubble(std::string first, std::string second)
{
    return first < second ? std::make_tuple(std::move(first), std::move(second))
                          : std::make_tuple(std::move(second), std::move(first));
}

qbb::util::multi_method<expression(const qbb::util::virtual_<expression>&, std::vector<bubble_t>&)>
fold_kronecker_deltas_ = {};

template <typename T>
bool isa(const expression&)
{
    return false;
}

expression fold_kronecker_deltas_subscription_expr(const subscription_expr& expr,
                                                   std::vector<bubble_t>& bubbles)
{
    if (isa<delta_expr>(expr.indexed_expr()))
    {
        bubbles.push_back(make_bubble(expr.indices()[0].as<index_expr>().id(),
                                      expr.indices()[1].as<index_expr>().id()));

        return integer_literal_expr(1);
    }
    else
    {
        return expr;
    }
}

expression fold_kronecker_deltas_binary_op_expr(const binary_operator_expr& expr,
                                                std::vector<bubble_t>& bubbles)
{
    std::vector<bubble_t> bubbles_from_left;
    std::vector<bubble_t> bubbles_from_right;

    expression new_left = fold_kronecker_deltas_(expr.left(), bubbles_from_left);
    expression new_right = fold_kronecker_deltas_(expr.right(), bubbles_from_right);

    switch (expr.tag())
    {
    case binary_op_tag::plus:
    case binary_op_tag::minus:
    {
        boost::range::set_intersection(bubbles_from_left, bubbles_from_right,
                                       std::back_inserter(bubbles));

        std::vector<bubble_t> burst_bubbles_left;
        std::vector<bubble_t> burst_bubbles_right;

        boost::range::set_difference(bubbles_from_left, bubbles,
                                     std::back_inserter(burst_bubbles_left));
        boost::range::set_difference(bubbles_from_right, bubbles,
                                     std::back_inserter(burst_bubbles_right));

        for (auto bubble : burst_bubbles_left)
        {
            new_left = binary_operator_expr(
                binary_op_tag::multiplies,
                delta_expr( {index_expr(std::get<0>(bubble)),
                             index_expr(std::get<1>(bubble))}),
                new_left);
        }

        for (auto bubble : burst_bubbles_left)
        {
            new_right = binary_operator_expr(
                binary_op_tag::multiplies,
                delta_expr({index_expr(std::get<0>(bubble)),
                            index_expr(std::get<1>(bubble))}),
                new_right);
        }

        return binary_operator_expr(expr.tag(), new_left, new_right);
    }
    case binary_op_tag::multiplies:
    {
        boost::range::merge(bubbles_from_left, bubbles_from_right, std::back_inserter(bubbles));
        return binary_operator_expr(expr.tag(), new_left, new_right);
    }
    default:
    {
        for (auto bubble : bubbles_from_left)
        {
            new_left = binary_operator_expr(
                binary_op_tag::multiplies,
                delta_expr( {index_expr(std::get<0>(bubble)),
                             index_expr(std::get<1>(bubble))}),
                new_left);
        }

        for (auto bubble : bubbles_from_right)
        {
            new_right = binary_operator_expr(
                binary_op_tag::multiplies,
               delta_expr( {index_expr(std::get<0>(bubble)),
                            index_expr(std::get<1>(bubble))}),
                new_right);
        }

        return binary_operator_expr(expr.tag(), new_left, new_right);
    }
    }
}

boost::optional<std::string> find__index(const std::vector<std::string>& sum_indices,
                                         const bubble_t& bubble)
{
    auto iter = std::find(sum_indices.begin() , sum_indices.end(), std::get<0>(bubble));

    if (iter != sum_indices.end())
    {
        return *iter;
    }
    else
    {
        auto iter2 = std::find(sum_indices.begin() , sum_indices.end(), std::get<1>(bubble));
        
        if (iter2 != sum_indices.end())
        {
            return *iter2;
        }
        else
        {
            return {};
        }
    }
}

expression fold_kronecker_deltas_sum_expr(const sum_expr& expr, std::vector<bubble_t>& bubbles)
{
    std::vector<std::string> sum_indices;

    for (const auto& index : expr.indices())
    {
        sum_indices.push_back(index.as<index_expr>().id());
    }

    std::vector<bubble_t> bubbles_from_body;

    expression new_body = fold_kronecker_deltas_(expr.body(), bubbles_from_body);
    
    boost::range::unique(bubbles_from_body);

    std::vector<std::string> eliminated_indices;
    std::vector<bubble_t> burst_bubbles;

    for (const auto& bubble : bubbles_from_body)
    {
        if (auto index = find__index(sum_indices, bubble))
        {
            burst_bubbles.push_back(bubble);
            eliminated_indices.push_back(*index);
        }
        else
        {
            bubbles.push_back(bubble);
        }
    }

    std::vector<expression> surviving_indices;
    
    for(const auto& index : sum_indices)
    {
        if(std::find(eliminated_indices.begin(), eliminated_indices.end(), index) == eliminated_indices.end())
        {
            surviving_indices.emplace_back(index_expr(index));
        }
    }
    
    //FIXME: substitute eliminated indices
    
    if (surviving_indices.empty())
    {
        return expr.body();
    }
    else
    {
        return sum_expr(expr.body(), surviving_indices);
    }
}
}

expression fold_kronecker_deltas(expression expr)
{
    std::vector<bubble_t> bubbles;

    return fold_kronecker_deltas_(expr, bubbles);
}
}
}