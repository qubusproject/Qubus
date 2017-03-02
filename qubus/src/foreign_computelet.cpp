#include <qubus/foreign_computelet.hpp>

#include <utility>

namespace qubus
{

const foreign_computelet_registry::version&
foreign_computelet_registry::lookup_version(const std::string& kernel_id,
                                            const architecture_identifier& target) const
{
    std::lock_guard<std::mutex> lock(version_table_mutex_);

    auto search_result = version_table_.find(kernel_id);

    if (search_result != version_table_.end())
    {
        auto& subtable = search_result->second;

        auto subtable_search_result =
            std::find_if(subtable.begin(), subtable.end(),
                         [&target](const auto& entry) { return entry->target() == target; });

        if (subtable_search_result != subtable.end())
            return **subtable_search_result;

        throw 0; // Unknown version
    }
    else
    {
        throw 0; // Unknown version
    }
}

foreign_computelet_registry& get_foreign_computelet_registry()
{
    static foreign_computelet_registry registry;

    return registry;
}

foreign_computelet::foreign_computelet(std::string id_, type result_type_, std::vector<type> argument_types_)
: id_(std::move(id_)), result_type_(std::move(result_type_)),
  argument_types_(std::move(argument_types_))
{
}
}