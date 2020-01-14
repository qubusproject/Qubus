#include <qubus/IR/pass_manager.hpp>

namespace qubus
{

isl::context& pass_resource_manager::get_isl_ctx()
{
    return isl_ctx_;
}

template class analysis_pass_registry<function>;
template class analysis_pass_registry<assembly>;

template class pass_manager<function>;
template class pass_manager<assembly>;

for_all_functions_assembly_pass::for_all_functions_assembly_pass(function_pass_manager function_passes)
: m_function_passes(std::move(function_passes))
{
}

transformation_result<assembly>
for_all_functions_assembly_pass::run(assembly input, analysis_manager<assembly>& manager) const
{
    for (function& func : input.functions())
    {
        m_function_passes.run(std::move(func));
    }

    return transformation_result<assembly>(std::move(input), {});
}

} // namespace qubus