#ifndef QBB_QUBUS_ISL_FLOW_HPP
#define QBB_QUBUS_ISL_FLOW_HPP

#include <qbb/kubus/isl/map.hpp>

#include <isl/flow.h>

namespace qbb
{
namespace qubus
{
namespace isl
{

int compute_flow(union_map sink, union_map must_source, union_map may_source, union_map schedule,
                 union_map& must_dep, union_map& may_dep, union_map& must_no_source,
                 union_map& may_no_source);

}
}
}

#endif