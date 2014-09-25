#ifndef QBB_KUBUS_ISL_SCHEDULE_HPP
#define QBB_KUBUS_ISL_SCHEDULE_HPP

#include <qbb/kubus/isl/set.hpp>
#include <qbb/kubus/isl/map.hpp>

#include <isl/schedule.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class schedule_constraints
{
public:
    explicit schedule_constraints(const union_set& domain);

    schedule_constraints(const schedule_constraints& other);

    ~schedule_constraints();

    void set_validity_constraints(const union_map& validity);

    void set_coincidence_constraints(const union_map& coincidence);

    void set_proximity_constraints(const union_map& proximity);

    isl_schedule_constraints* native_handle() const;

    isl_schedule_constraints* release() noexcept;

private:
    isl_schedule_constraints* handle_;
};

class schedule
{
public:
    explicit schedule(schedule_constraints constraints);

    ~schedule();

    void dump();

    union_map get_map() const;

    isl_schedule* native_handle() const;

private:
    isl_schedule* handle_;
};
}
}
}

#endif