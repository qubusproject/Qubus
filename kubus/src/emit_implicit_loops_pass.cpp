#include <qbb/kubus/emit_implicit_loops_pass.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/util/multi_method.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <string>
#include <set>
#include <mutex>

namespace qbb
{
namespace kubus
{

namespace
{

qbb::util::sparse_multi_method<void(const qbb::util::virtual_<expression>&, std::set<std::string>&)>
deduce_undefined_lhs_indices = {};

void deduce_undefined_lhs_indices_binary_op_expr(const binary_operator_expr& expr,
                                                 std::set<std::string>& undefined_indices)
{
    if (expr.tag() == binary_op_tag::assign)
    {
        deduce_undefined_lhs_indices(expr.left(), undefined_indices);
    }
}

void deduce_undefined_lhs_indices_for_all_expr(const for_all_expr& expr,
                                           std::set<std::string>& undefined_indices)
{
    deduce_undefined_lhs_indices(expr.body(), undefined_indices);

    undefined_indices.erase(expr.index().as<index_expr>().id());
}

void deduce_undefined_lhs_indices_for_expr(const for_expr& expr,
                                           std::set<std::string>& undefined_indices)
{
    deduce_undefined_lhs_indices(expr.body(), undefined_indices);

    undefined_indices.erase(expr.index().as<index_expr>().id());
}

void deduce_undefined_lhs_indices_index_expr(const index_expr& expr,
                                             std::set<std::string>& undefined_indices)
{
    undefined_indices.insert(expr.id());
}

void deduce_undefined_lhs_indices_subscription_expr(const subscription_expr& expr,
                                                    std::set<std::string>& undefined_indices)
{
    for (const auto& index : expr.indices())
    {
        deduce_undefined_lhs_indices(index, undefined_indices);
    }
}

void init_deduce_undefined_lhs_indices()
{
    deduce_undefined_lhs_indices.add_specialization(deduce_undefined_lhs_indices_binary_op_expr);
    deduce_undefined_lhs_indices.add_specialization(deduce_undefined_lhs_indices_for_all_expr);
    deduce_undefined_lhs_indices.add_specialization(deduce_undefined_lhs_indices_for_expr);
    deduce_undefined_lhs_indices.add_specialization(deduce_undefined_lhs_indices_index_expr);
    deduce_undefined_lhs_indices.add_specialization(deduce_undefined_lhs_indices_subscription_expr);
}

std::once_flag deduce_undefined_lhs_indices_init_flag = {};
}

expression emit_implicit_loops(const expression& expr)
{
    std::call_once(deduce_undefined_lhs_indices_init_flag, init_deduce_undefined_lhs_indices);

    std::set<std::string> undefined_indices;

    deduce_undefined_lhs_indices(expr, undefined_indices);

    expression new_root = expr;

    for (const auto& index_id : undefined_indices | boost::adaptors::reversed)
    {
        new_root = for_all_expr(index_expr(index_id), new_root);
    }
    
    return new_root;
}
}
}