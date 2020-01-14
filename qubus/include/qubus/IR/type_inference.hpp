#ifndef QUBUS_TYPE_INFERENCE_HPP
#define QUBUS_TYPE_INFERENCE_HPP

#include <qubus/IR/assembly_inference_pass.hpp>
#include <qubus/IR/expression.hpp>
#include <qubus/IR/pass_manager.hpp>
#include <qubus/IR/type.hpp>

namespace qubus
{

type typeof_(const expression& expr);

class type_inference_result
{
public:
    explicit type_inference_result(assembly_inference_result& assembly_inference_,
                                   pass_resource_manager& resource_manager_);

    type get_type(const expression& expr) const;

private:
    assembly_inference_result* m_assembly_inference_;
    pass_resource_manager* m_resource_manager;
};

class type_inference_pass
{
public:
    using result_type = type_inference_result;

    type_inference_result run(const function& func, function_analysis_manager& manager,
                              pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;

private:
};

} // namespace qubus

#endif