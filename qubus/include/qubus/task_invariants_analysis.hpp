#ifndef QUBUS_INVARIANT_ANALYSIS_HPP
#define QUBUS_INVARIANT_ANALYSIS_HPP

#include <qubus/pass_manager.hpp>

#include <qubus/IR/expression.hpp>

#include <memory>

namespace qubus
{

class basic_alias_analysis_result;

class task_invariants_analysis_result
{
public:
    explicit task_invariants_analysis_result(const basic_alias_analysis_result& basic_alias_analysis_);

    bool is_invariant(const expression& expr, const expression& context) const;

private:
    const basic_alias_analysis_result* basic_alias_analysis_;
};

class task_invariants_analysis_pass
{
public:
    using result_type = task_invariants_analysis_result;

    task_invariants_analysis_result run(const expression& root, analysis_manager& manager,
                                        pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
