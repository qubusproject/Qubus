#include <qbb/qubus/pass_manager.hpp>

namespace qbb
{
namespace qubus
{

isl::context& pass_resource_manager::get_isl_ctx()
{
    return isl_ctx_;
}

pass_manager::pass_manager()
: analysis_man_(resource_manager_)
{
}

}
}