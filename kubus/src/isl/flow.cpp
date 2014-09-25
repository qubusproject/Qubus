#include <qbb/kubus/isl/flow.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

int compute_flow(union_map sink, union_map must_source, union_map may_source, union_map schedule,
                 union_map& must_dep, union_map& may_dep, union_map& must_no_source,
                 union_map& may_no_source)
{
    isl_union_map* must_dep_;
    isl_union_map* may_dep_;
    isl_union_map* must_no_source_;
    isl_union_map* may_no_source_;

    int result = isl_union_map_compute_flow(sink.release(), must_source.release(), may_source.release(), schedule.release(), &must_dep_, &may_dep_,
                                      &must_no_source_, &may_no_source_);
    
    must_dep = union_map(must_dep_);
    may_dep = union_map(may_dep_);
    must_no_source = union_map(must_no_source_);
    may_no_source = union_map(may_no_source_);
    
    return result;
}
}
}
}