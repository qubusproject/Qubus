#ifndef QUBUS_IR_ASSEMBLY_INFERENCE_PASS_HPP
#define QUBUS_IR_ASSEMBLY_INFERENCE_PASS_HPP

#include <qubus/IR/pass_manager.hpp>
#include <qubus/IR/assembly.hpp>

namespace qubus
{
class assembly_inference_result
{
public:
    const assembly& infered_assembly() const
    {
        return m_infered_assembly;
    }
private:
    assembly m_infered_assembly;
};

class assembly_inference_pass
{
public:
    using result_type = assembly_inference_result;

    assembly_inference_result run(const function& func, function_analysis_manager& manager,
                                  pass_resource_manager& resource_manager_) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
