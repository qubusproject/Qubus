#ifndef QUBUS_STATIC_SCHEDULE_ANALYSIS_HPP
#define QUBUS_STATIC_SCHEDULE_ANALYSIS_HPP

#include <qbb/qubus/static_schedule.hpp>

#include <qbb/qubus/pass_manager.hpp>

#include <boost/optional.hpp>

#include <qbb/util/optional_ref.hpp>

namespace qubus
{

class static_schedule_analysis_result
{
public:
    explicit static_schedule_analysis_result(
        std::unordered_map<const expression*, static_schedule> schedule_table_);

    util::optional_ref<const static_schedule> get_schedule_by_root(const expression& root) const;
    util::optional_ref<const static_schedule> get_schedule_containing(const expression& expr) const;

private:
    std::unordered_map<const expression*, static_schedule> schedule_table_;
};

class static_schedule_analysis_pass
{
public:
    using result_type = static_schedule_analysis_result;

    static_schedule_analysis_result run(const expression& root, analysis_manager& manager,
                                        pass_resource_manager& resource_manager) const;

    std::vector<analysis_id> required_analyses() const;
};
}

#endif
