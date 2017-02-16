#include <qbb/qubus/isl/flow.hpp>

inline namespace qbb
{
namespace qubus
{
namespace isl
{

union_access_info::union_access_info(isl_union_access_info* handle_) : handle_(handle_)
{
}

union_access_info::union_access_info(const union_access_info& other)
: handle_(isl_union_access_info_copy(other.native_handle()))
{
}

union_access_info::~union_access_info()
{
    isl_union_access_info_free(handle_);
}

union_access_info& union_access_info::operator=(union_access_info other)
{
    isl_union_access_info_free(handle_);

    handle_ = other.release();

    return *this;
}

void union_access_info::set_must_source(isl::union_map source)
{
    handle_ = isl_union_access_info_set_must_source(handle_, source.release());
}

void union_access_info::set_may_source(isl::union_map source)
{
    handle_ = isl_union_access_info_set_may_source(handle_, source.release());
}

void union_access_info::set_schedule(isl::schedule sched)
{
    handle_ = isl_union_access_info_set_schedule(handle_, sched.release());
}

isl_union_access_info* union_access_info::native_handle() const
{
    return handle_;
}

isl_union_access_info* union_access_info::release() noexcept
{
    isl_union_access_info* temp = handle_;

    handle_ = nullptr;

    return temp;
}

union_access_info union_access_info::from_sink(union_map sink)
{
    return union_access_info(isl_union_access_info_from_sink(sink.release()));
}

union_flow::union_flow(isl_union_flow* handle_) : handle_(handle_)
{
}

union_flow::~union_flow()
{
    isl_union_flow_free(handle_);
}

union_map union_flow::get_must_dependence() const
{
    return union_map(isl_union_flow_get_must_dependence(handle_));
}

union_map union_flow::get_may_dependence() const
{
    return union_map(isl_union_flow_get_may_dependence(handle_));
}

isl_union_flow* union_flow::native_handle() const
{
    return handle_;
}

isl_union_flow* union_flow::release() noexcept
{
    isl_union_flow* temp = handle_;

    handle_ = nullptr;

    return temp;
}

union_flow compute_flow(union_access_info access_info)
{
    return union_flow(isl_union_access_info_compute_flow(access_info.release()));
}

int compute_flow(union_map sink, union_map must_source, union_map may_source, union_map schedule,
                 union_map& must_dep, union_map& may_dep, union_map& must_no_source,
                 union_map& may_no_source)
{
    isl_union_map* must_dep_;
    isl_union_map* may_dep_;
    isl_union_map* must_no_source_;
    isl_union_map* may_no_source_;

    int result = isl_union_map_compute_flow(sink.release(), must_source.release(),
                                            may_source.release(), schedule.release(), &must_dep_,
                                            &may_dep_, &must_no_source_, &may_no_source_);

    must_dep = union_map(must_dep_);
    may_dep = union_map(may_dep_);
    must_no_source = union_map(must_no_source_);
    may_no_source = union_map(may_no_source_);

    return result;
}
}
}
}