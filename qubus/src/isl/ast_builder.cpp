#include <qbb/qubus/isl/ast_builder.hpp>

namespace qbb
{
namespace qubus
{
namespace isl
{
    
ast_builder_base::ast_builder_base(isl_ast_build* handle_) : handle_(handle_)
{
}

ast_node ast_builder_base::build_ast_from_schedule(union_map schedule)
{
    return ast_node(isl_ast_build_ast_from_schedule(handle_, schedule.release()));
}

ast_node ast_builder_base::build_node_from_schedule(schedule sched)
{
    return ast_node(isl_ast_build_node_from_schedule(handle_, sched.release()));
}

ast_expr ast_builder_base::build_expr_from_pw_aff(pw_aff f)
{
    return ast_expr(isl_ast_build_expr_from_pw_aff(handle_, f.release()));
}

ast_expr ast_builder_base::build_access_from_pw_multi_aff(pw_multi_aff pma)
{
    return ast_expr(isl_ast_build_access_from_pw_multi_aff(handle_, pma.release()));
}

ast_expr ast_builder_base::build_expr_from_set(set s)
{
    return ast_expr(isl_ast_build_expr_from_set(handle_, s.release()));
}

void ast_builder_base::set_options(union_map options)
{
    handle_ = isl_ast_build_set_options(handle_, options.release());
}

namespace
{
extern "C" isl_ast_node* isl_create_leaf_callback(isl_ast_build* build, void* user)
{
    auto callback = static_cast<const std::function<ast_node(ast_builder_ref)>*>(user);

    ast_builder_ref builder(build);

    return (*callback)(builder).release();
}
}

void ast_builder_base::set_create_leaf(std::function<ast_node(ast_builder_ref)> callback)
{
    handle_ = isl_ast_build_set_create_leaf(handle_, isl_create_leaf_callback, &callback);
}

namespace
{
extern "C" isl_ast_node* isl_at_each_domain_callback(isl_ast_node* node, isl_ast_build* build,
                                                     void* user)
{
    auto callback = static_cast<const std::function<ast_node(ast_node, ast_builder_ref)>*>(user);

    ast_builder_ref builder(build);

    return (*callback)(ast_node(node), builder).release();
}
}

void ast_builder_base::set_at_each_domain(std::function<ast_node(ast_node, ast_builder_ref)> callback)
{
    handle_ = isl_ast_build_set_at_each_domain(handle_, isl_at_each_domain_callback, &callback);
}

namespace
{
extern "C" isl_id* isl_before_each_for_callback(isl_ast_build* build, void* user)
{
    auto callback = static_cast<const std::function<id(ast_builder_ref)>*>(user);

    ast_builder_ref builder(build);

    return (*callback)(builder).release();
}

extern "C" isl_ast_node* isl_after_each_for_callback(isl_ast_node* node, isl_ast_build* build,
                                                     void* user)
{
    auto callback = static_cast<const std::function<ast_node(ast_node, ast_builder_ref)>*>(user);

    ast_builder_ref builder(build);

    return (*callback)(ast_node(node), builder).release();
}
}

void ast_builder_base::set_before_each_for(std::function<id(ast_builder_ref)> callback)
{
    static std::function<id(ast_builder_ref)> callback_ = callback;
    
    handle_ = isl_ast_build_set_before_each_for(handle_, isl_before_each_for_callback, &callback_);
}

void ast_builder_base::set_after_each_for(std::function<ast_node(ast_node, ast_builder_ref)> callback)
{
    handle_ = isl_ast_build_set_after_each_for(handle_, isl_after_each_for_callback, &callback);
}

union_map ast_builder_base::get_schedule() const
{
    return union_map(isl_ast_build_get_schedule(handle_));
}

space ast_builder_base::get_schedule_space() const
{
    return space(isl_ast_build_get_schedule_space(handle_));
}

isl_ast_build* ast_builder_base::native_handle() const
{
    return handle_;
}

ast_builder::ast_builder(isl_ast_build* handle_) : ast_builder_base(handle_)
{
}

ast_builder::ast_builder(const context& ctx) : ast_builder_base(isl_ast_build_alloc(ctx.native_handle()))
{
}

ast_builder::ast_builder(set params) : ast_builder_base(isl_ast_build_from_context(params.release()))
{
}

ast_builder::~ast_builder()
{
    isl_ast_build_free(native_handle());
}



}
}
}