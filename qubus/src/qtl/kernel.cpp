#include <qubus/qtl/kernel.hpp>

#include <qubus/qtl/task_generator.hpp>
#include <qubus/qtl/kronecker_delta_folding_pass.hpp>
#include <qubus/qtl/legalization_pass.hpp>
#include <qubus/qtl/lower_abstract_indices.hpp>
#include <qubus/qtl/lower_top_level_sums.hpp>
#include <qubus/qtl/multi_index_handling.hpp>
#include <qubus/qtl/object_extraction_pass.hpp>
#include <qubus/qtl/sparse_patterns.hpp>

#include <qubus/IR/compound_expr.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include <algorithm>
#include <atomic>

namespace qubus
{
namespace qtl
{

namespace
{
std::atomic<kernel*> current_kernel{nullptr};
}

namespace this_kernel
{

void add_code(std::unique_ptr<expression> code)
{
    auto kernel = current_kernel.load();

    QUBUS_ASSERT(kernel, "add_code has to be called from a kernel definition.");

    kernel->add_code(std::move(code));
}

void construct(kernel& new_kernel, std::function<void()> constructor)
{
    QUBUS_ASSERT(current_kernel.load() == nullptr, "Another kernel is currently constructed.");

    current_kernel.store(&new_kernel);

    constructor();

    current_kernel.store(nullptr);
}
}

void kernel::translate_kernel(std::vector<variable_declaration> params)
{
    std::vector<std::tuple<variable_declaration, object>> parameter_map;

    for (auto& expr : computations_)
    {
        expr = extract_objects(*expr, parameter_map);

        expr = legalize_expression(*expr);

        expr = expand_multi_indices(*expr);

        expr = fold_kronecker_deltas(*expr);

        expr = optimize_sparse_patterns(*expr);

        expr = lower_top_level_sums(*expr);

        expr = lower_abstract_indices(*expr);
    }

    auto root_task = sequenced_tasks(std::move(computations_));

    auto mod = wrap_code_in_task(std::move(root_task));

    const auto& entry = mod->lookup_function("entry");

    code_ = symbol_id(entry.full_name());

    for (const auto& entry : parameter_map)
    {
        params.push_back(std::get<0>(entry));
        args_.push_back(std::get<1>(entry));
    }

    for (const auto& param : entry.params())
    {
        auto pos = std::find(params.begin(), params.end(), param);

        immutable_argument_map_.push_back(pos - params.begin());
    }

    const auto& result = entry.result();

    auto pos = std::find(params.begin(), params.end(), result);

    mutable_argument_map_.push_back(pos - params.begin());

    QUBUS_ASSERT(immutable_argument_map_.size() == entry.arity(),
                 "Wrong number of arguments mappings.");
    QUBUS_ASSERT(mutable_argument_map_.size() == 1, "Wrong number of arguments mappings.");

    get_runtime().get_module_library().add(std::move(mod)).get();
}

}
}