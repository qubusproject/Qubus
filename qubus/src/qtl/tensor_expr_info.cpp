#include <hpx/config.hpp>

#include <qbb/qubus/qtl/tensor_expr_info.hpp>

#include <qbb/qubus/qtl/kronecker_delta_folding_pass.hpp>
#include <qbb/qubus/qtl/lower_abstract_indices.hpp>
#include <qbb/qubus/qtl/lower_top_level_sums.hpp>
#include <qbb/qubus/qtl/multi_index_handling.hpp>
#include <qbb/qubus/qtl/sparse_patterns.hpp>

#include <qbb/qubus/IR/qir.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
namespace qtl
{

tensor_expr_info::tensor_expr_info(type result_type_,
                                   std::tuple<tensor_expr_closure, std::vector<object>> ir_info)
: args_(std::move(std::get<1>(ir_info)))
{
    auto closure = std::move(std::get<0>(ir_info));

    variable_declaration result(result_type_);

    std::unique_ptr<expression> lhs = var(result);

    std::vector<std::unique_ptr<expression>> lhs_indices;

    for (const auto& idx : closure.free_indices)
    {
        if (idx.alias)
        {
            lhs_indices.push_back(var(*idx.alias));
        }
        else
        {
            QBB_ASSERT(idx.indices.size() == 1,
                       "Unnamed multi-indices are currently not supported.");

            lhs_indices.push_back(var(idx.indices[0]));
        }
    }

    lhs = subscription(std::move(lhs), std::move(lhs_indices));

    std::unique_ptr<expression> current_root = assign(std::move(lhs), clone(*closure.rhs));

    for (const auto& idx : closure.free_indices | boost::adaptors::reversed)
    {
        if (idx.alias)
        {
            current_root = for_all(idx.indices, *idx.alias, std::move(current_root));
        }
        else
        {
            QBB_ASSERT(idx.indices.size() == 1,
                       "Unnamed multi-indices are currently not supported.");

            current_root = for_all(idx.indices[0], std::move(current_root));
        }
    }

    function_declaration entry_point("entry", closure.params, result, std::move(current_root));

    entry_point = expand_multi_indices(entry_point);

    entry_point = fold_kronecker_deltas(entry_point);

    entry_point = optimize_sparse_patterns(entry_point);

    entry_point = lower_top_level_sums(entry_point);

    entry_point = lower_abstract_indices(entry_point);

    stored_computelet_ = make_computelet(entry_point);
}

computelet tensor_expr_info::stored_computelet() const
{
    return stored_computelet_;
}

const std::vector<object>& tensor_expr_info::args() const
{
    return args_;
}
}
}
}
