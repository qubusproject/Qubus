#include <qubus/IR/assembly_inference_pass.hpp>

namespace qubus
{

assembly_inference_result
assembly_inference_pass::run(const module& mod, analysis_manager<module>& manager,
                             pass_resource_manager& resource_manager_) const
{
}

QUBUS_REGISTER_MODULE_ANALYSIS_PASS(assembly_inference_pass);

} // namespace qubus
