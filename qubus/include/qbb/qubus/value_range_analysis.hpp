#ifndef QUBUS_VALUE_RANGE_ANALYSIS_HPP
#define QUBUS_VALUE_RANGE_ANALYSIS_HPP

#include <qbb/qubus/axiom_analysis.hpp>
#include <qbb/qubus/value_set_analysis.hpp>

#include <qbb/qubus/pass_manager.hpp>

#include <boost/optional.hpp>

#include <memory>

namespace qubus
{

struct value_range
{
    std::unique_ptr<expression> lower_bound;
    std::unique_ptr<expression> upper_bound;
};

class value_range_analysis_result
{
public:
    explicit value_range_analysis_result(const value_set_analysis_result& value_set_analysis_,
                                         const axiom_analysis_result& axiom_analysis_,
                                         pass_resource_manager& resource_manager_);

    boost::optional<value_range> determine_value_range(const expression& expr,
                                                       const expression& context) const;

private:
    const value_set_analysis_result* value_set_analysis_;
    const axiom_analysis_result* axiom_analysis_;
    pass_resource_manager* resource_manager_;
};

class value_range_analysis_pass
{
public:
    using result_type = value_range_analysis_result;

    value_range_analysis_result run(const expression& root, analysis_manager& manager,
                                    pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
