#ifndef QUBUS_ISL_SCHEDULE_HPP
#define QUBUS_ISL_SCHEDULE_HPP

#include <qubus/isl/set.hpp>
#include <qubus/isl/map.hpp>
#include <qubus/isl/band.hpp>
#include <qubus/isl/schedule_node.hpp>

#include <isl/schedule.h>

#include <vector>
#include <functional>

namespace qubus
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

    explicit schedule(isl_schedule* handle_);
    
    schedule(const schedule& other); 
    
    ~schedule();
    
    schedule& operator=(const schedule& other);

    void dump();

    schedule_node get_root() const;
    
    union_map get_map() const;
    union_set get_domain() const;

    std::vector<band> get_band_forest() const;

    isl_schedule* native_handle() const;
    isl_schedule* release() noexcept;
    
    static schedule from_domain(union_set domain);
private:
    isl_schedule* handle_;
};

schedule get_schedule(const schedule_node& node);

schedule map_schedule_node(schedule s, std::function<schedule_node(schedule_node)> callback);

}
}

#endif