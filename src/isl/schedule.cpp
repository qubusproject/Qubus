#include <qbb/kubus/isl/schedule.hpp>

namespace qbb
{
namespace kubus
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

schedule::~schedule()
{
    isl_schedule_free(handle_);
}

void schedule::dump()
{
    isl_schedule_dump(handle_);
}

union_map schedule::get_map() const
{
    return union_map(isl_schedule_get_map(handle_));
}

isl_schedule* schedule::native_handle() const
{
    return handle_;
}
}
}
}