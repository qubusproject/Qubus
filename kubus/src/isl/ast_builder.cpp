#include <qbb/kubus/isl/ast_builder.hpp>

namespace qbb
{
namespace kubus
{
namespace isl
{

ast_builder::ast_builder(set params) : handle_{isl_ast_build_from_context(params.release())}
{
}

ast_builder::~ast_builder()
{
    isl_ast_build_free(handle_);
}

ast_node ast_builder::build_ast_from_schedule(union_map schedule)
{
    return ast_node(isl_ast_build_ast_from_schedule(handle_, schedule.release()));
}

void ast_builder::set_options(union_map options)
{
    handle_ = isl_ast_build_set_options(handle_, options.release());
}

isl_ast_build* ast_builder::native_handle() const
{
    return handle_;
}
}
}
}