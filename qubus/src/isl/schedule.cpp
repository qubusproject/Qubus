#include <qbb/qubus/isl/schedule.hpp>

namespace qubus
{
namespace isl
{

schedule_constraints::schedule_constraints(const union_set& domain)
: handle_{isl_schedule_constraints_on_domain(isl_union_set_copy(domain.native_handle()))}
{
}

schedule_constraints::schedule_constraints(const schedule_constraints& other)
: handle_{isl_schedule_constraints_copy(other.native_handle())}
{
}

schedule_constraints::~schedule_constraints()
{
    isl_schedule_constraints_free(handle_);
}

void schedule_constraints::set_validity_constraints(const union_map& validity)
{
    handle_ = isl_schedule_constraints_set_validity(handle_,
                                                    isl_union_map_copy(validity.native_handle()));
}

void schedule_constraints::set_coincidence_constraints(const union_map& coincidence)
{
    handle_ = isl_schedule_constraints_set_coincidence(
        handle_, isl_union_map_copy(coincidence.native_handle()));
}

void schedule_constraints::set_proximity_constraints(const union_map& proximity)
{
    handle_ = isl_schedule_constraints_set_proximity(handle_,
                                                     isl_union_map_copy(proximity.native_handle()));
}

isl_schedule_constraints* schedule_constraints::native_handle() const
{
    return handle_;
}

isl_schedule_constraints* schedule_constraints::release() noexcept
{
    isl_schedule_constraints* temp = handle_;

    handle_ = nullptr;

    return temp;
}

schedule::schedule(schedule_constraints constraints)
: handle_{isl_schedule_constraints_compute_schedule(constraints.release())}
{
}

schedule::schedule(isl_schedule* handle_)
: handle_(handle_)
{
}

schedule::schedule(const schedule& other)
: handle_(isl_schedule_copy(other.native_handle()))
{
}

schedule::~schedule()
{
    isl_schedule_free(handle_);
}

schedule& schedule::operator=(const schedule& other)
{
    isl_schedule_free(handle_);
    
    handle_ = isl_schedule_copy(other.native_handle());
    
    return *this;
}

void schedule::dump()
{
    isl_schedule_dump(handle_);
}

schedule_node schedule::get_root() const
{
    return schedule_node(isl_schedule_get_root(handle_));
}

union_map schedule::get_map() const
{
    return union_map(isl_schedule_get_map(handle_));
}

union_set schedule::get_domain() const
{
    return union_set(isl_schedule_get_domain(handle_));
}

namespace
{
extern "C" isl_stat add_band_to_forest(isl_band* child, void* user) noexcept;
}

std::vector<band> schedule::get_band_forest() const
{
    std::vector<band> result;
    
    isl_band_list* forest = isl_schedule_get_band_forest(handle_);
    
    isl_band_list_foreach(forest, add_band_to_forest, &result);
    
    isl_band_list_free(forest);
    
    return result;
}

isl_schedule* schedule::native_handle() const
{
    return handle_;
}

isl_schedule* schedule::release() noexcept
{
    isl_schedule* tmp = handle_;
    handle_ = nullptr;
    return tmp;
}

schedule schedule::from_domain(union_set domain)
{
    return schedule(isl_schedule_from_domain(domain.release()));
}

namespace
{
extern "C" {

isl_stat add_band_to_forest(isl_band* child, void* user) noexcept
{
    auto& children = *static_cast<std::vector<band>*>(user);

    children.emplace_back(child);

    return isl_stat_ok;
}
}
}

schedule get_schedule(const schedule_node& node)
{
    return schedule(isl_schedule_node_get_schedule(node.native_handle()));
}

namespace
{
extern "C"
{

isl_schedule_node* qbb_qubus_isl_map_schedule_node_thunk(isl_schedule_node* node, void* user)
{
    auto& callback = *static_cast<std::function<schedule_node(schedule_node)>*>(user);

    return callback(schedule_node(node)).release();
}

}
}

schedule map_schedule_node(schedule s, std::function<schedule_node(schedule_node)> callback)
{
    return schedule(isl_schedule_map_schedule_node_bottom_up(s.release(), &qbb_qubus_isl_map_schedule_node_thunk, &callback));
}

}
}