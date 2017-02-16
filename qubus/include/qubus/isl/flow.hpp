#ifndef QUBUS_ISL_FLOW_HPP
#define QUBUS_ISL_FLOW_HPP

#include <qubus/isl/map.hpp>
#include <qubus/isl/schedule.hpp>

#include <isl/flow.h>

namespace qubus
{
namespace isl
{

class union_access_info
{
public:
    explicit union_access_info(isl_union_access_info* handle_);

    union_access_info(const union_access_info& other);

    ~union_access_info();

    union_access_info& operator=(union_access_info other);

    void set_must_source(isl::union_map source);
    void set_may_source(isl::union_map source);
    void set_schedule(isl::schedule sched);

    isl_union_access_info* native_handle() const;
    isl_union_access_info* release() noexcept;

    static union_access_info from_sink(union_map sink);
private:
    isl_union_access_info* handle_;
};

class union_flow
{
public:
    explicit union_flow(isl_union_flow* handle_);

    union_flow(const union_flow& other) = delete;
    union_flow& operator=(const union_flow& other) = delete;

    union_flow(union_flow&& other) = default;
    union_flow& operator=(union_flow&& other) = default;

    ~union_flow();

    union_map get_must_dependence() const;
    union_map get_may_dependence() const;

    isl_union_flow* native_handle() const;
    isl_union_flow* release() noexcept;
private:
    isl_union_flow* handle_;
};

union_flow compute_flow(union_access_info access_info);

int compute_flow(union_map sink, union_map must_source, union_map may_source, union_map schedule,
                 union_map& must_dep, union_map& may_dep, union_map& must_no_source,
                 union_map& may_no_source);

}
}

#endif