#include <hpx/config.hpp>

#include <qbb/kubus/tensor_expr_info.hpp>

#include <qbb/kubus/IR/kir.hpp>

#include <qbb/kubus/runtime.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace kubus
{

tensor_expr_info::tensor_expr_info(
    type result_type_, std::tuple<tensor_expr_closure, std::vector<std::shared_ptr<object>>> ir_info)
: result_type_(std::move(result_type_)), closure_(std::move(std::get<0>(ir_info))),
  args_(std::move(std::get<1>(ir_info)))
{
}

const plan& tensor_expr_info::compiled_plan() const
{

    if (!plan_)
    {
        variable_declaration result(result_type_);

        expression lhs = variable_ref_expr(result);

        std::vector<expression> lhs_indices;

        for (const auto& idx : closure_.free_indices)
        {
            if (idx.alias)
            {
                lhs_indices.push_back(variable_ref_expr(*idx.alias));
            }
            else
            {
                QBB_ASSERT(idx.indices.size() == 1, "Unnamed multi-indices are currently not supported.");

                lhs_indices.push_back(variable_ref_expr(idx.indices[0]));
            }
        }

        lhs = subscription_expr(lhs, lhs_indices);

        expression current_root = binary_operator_expr(binary_op_tag::assign, lhs, closure_.rhs);

        for (const auto& idx : closure_.free_indices | boost::adaptors::reversed)
        {
            if (idx.alias)
            {
                current_root = for_all_expr(idx.indices, *idx.alias, current_root);
            }
            else
            {
                QBB_ASSERT(idx.indices.size() == 1, "Unnamed multi-indices are currently not supported.");

                current_root = for_all_expr(idx.indices[0], current_root);
            }

        }

        function_declaration entry_point("entry", closure_.params, result, current_root);

        plan_ = get_runtime().compile(entry_point);
    }

    return *plan_;
}

const std::vector<std::shared_ptr<object>>& tensor_expr_info::args() const
{
    return args_;
}
}
}
