#include <qubus/pass_manager.hpp>

namespace qubus
{

isl::context& pass_resource_manager::get_isl_ctx()
{
    return isl_ctx_;
}

std::vector<analysis_pass> analysis_pass_registry::construct_all_passes() const
{
    std::lock_guard<std::mutex> guard(analysis_pass_table_mutex_);

    std::vector<analysis_pass> passes;

    for (const auto& id_and_constructor : analysis_pass_constructor_table_)
    {
        passes.push_back(id_and_constructor.second());
    }

    return passes;
}

analysis_pass_registry& analysis_pass_registry::get_instance()
{
    static analysis_pass_registry instance;

    return instance;
}

analysis_node::analysis_node(analysis_pass pass_, analysis_manager& manager_,
                             pass_resource_manager& resource_manager_)
: pass_(std::move(pass_)), manager_(manager_), resource_manager_(&resource_manager_)
{
}

void analysis_node::add_dependent(analysis_node& dependent)
{
    dependents_.push_back(&dependent);
}

const analysis_pass& analysis_node::pass() const
{
    return pass_;
}

void analysis_node::invalidate()
{
    for (const auto& dependent : dependents_)
    {
        dependent->invalidate();
    }
}

analysis_manager::analysis_manager(pass_resource_manager& resource_manager_)
{
    auto passes = analysis_pass_registry::get_instance().construct_all_passes();

    for (auto&& pass : passes)
    {
        auto id = pass.id();

        auto node = std::make_unique<analysis_node>(std::move(pass), *this, resource_manager_);

        analysis_table_.emplace(std::move(id), std::move(node));
    }

    for (const auto& id_and_node : analysis_table_)
    {
        auto& node = *id_and_node.second;

        for (const auto& required_analysis : node.pass().required_analyses())
        {
            auto search_result = analysis_table_.find(required_analysis);

            if (search_result != analysis_table_.end())
            {
                search_result->second->add_dependent(node);
            }
            else
            {
                throw 0; // Unknown analysis
            }
        }
    }
}

void analysis_manager::invalidate(const preserved_analyses_info& preserved_analyses)
{
    for (auto& id_and_node : analysis_table_)
    {
        auto search_result =
            std::find(preserved_analyses.begin(), preserved_analyses.end(), id_and_node.first);

        if (search_result == preserved_analyses.end())
        {
            analysis_table_.at(id_and_node.first)->invalidate();
        }
    }
}

void analysis_manager::invalidate()
{
    for (auto& id_and_node : analysis_table_)
    {
        analysis_table_.at(id_and_node.first)->invalidate();
    }
}

pass_manager::pass_manager() : analysis_man_(resource_manager_)
{
}

preserved_analyses_info pass_manager::run(function_declaration& fun)
{
    for (const auto& transformation : optimization_pipeline_)
    {
        auto preserved_analyses = transformation.run(fun, analysis_man_);

        analysis_man_.invalidate(preserved_analyses);
    }

    return {};
}
}