#include <qubus/IR/assembly_inference_pass.hpp>

namespace qubus
{

assembly_inference_result
assembly_inference_pass::run(const function& func, function_analysis_manager& manager,
                             pass_resource_manager& resource_manager_) const
{
}

std::vector<analysis_id> assembly_inference_pass::required_analyses() const
{
    return {};
}

QUBUS_REGISTER_FUNCTION_ANALYSIS_PASS(assembly_inference_pass);

} // namespace qubus
