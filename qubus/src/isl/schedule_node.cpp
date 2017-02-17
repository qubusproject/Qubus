#include <qubus/isl/schedule_node.hpp>

#include <qubus/isl/space.hpp>

#include <isl/val.h>
#include <isl/list.h>

namespace qubus
{
namespace isl
{
schedule_node::schedule_node(isl_schedule_node* handle_) : handle_(handle_)
{
}

schedule_node::schedule_node(const schedule_node& other)
: handle_(isl_schedule_node_copy(other.native_handle()))
{
}

schedule_node::~schedule_node()
{
    isl_schedule_node_free(handle_);
}

schedule_node& schedule_node::operator=(const schedule_node& other)
{
    isl_schedule_node_free(handle_);

    handle_ = isl_schedule_node_copy(other.native_handle());

    return *this;
}

isl_schedule_node_type schedule_node::get_type() const
{
    return isl_schedule_node_get_type(handle_);
}

schedule_node schedule_node::operator[](int idx)
{
    return schedule_node(isl_schedule_node_get_child(handle_, idx));
}

int schedule_node::n_children() const
{
    return isl_schedule_node_n_children(handle_);
}

bool schedule_node::has_children() const
{
    return isl_schedule_node_has_children(handle_);
}

schedule_node schedule_node::parent() const
{
    return schedule_node(isl_schedule_node_parent(isl_schedule_node_copy(handle_)));
}

union_map schedule_node::band_get_partial_schedule_union_map() const
{
    return union_map(isl_schedule_node_band_get_partial_schedule_union_map(handle_));
}

int schedule_node::band_n_member() const
{
    return isl_schedule_node_band_n_member(handle_);
}

bool schedule_node::band_is_permutable() const
{
    return isl_schedule_node_band_get_permutable(handle_);
}

void schedule_node::band_member_set_coincident(int pos, bool is_coincident)
{
    handle_ = isl_schedule_node_band_member_set_coincident(handle_, pos, is_coincident ? 1 : 0);
}

void schedule_node::band_member_set_ast_loop_type(int pos, isl_ast_loop_type type)
{
    handle_ = isl_schedule_node_band_member_set_ast_loop_type(handle_, pos, type);
}

void schedule_node::band_member_set_isolate_ast_loop_type(int pos, isl_ast_loop_type type)
{
    handle_ = isl_schedule_node_band_member_set_isolate_ast_loop_type(handle_, pos, type);
}

void schedule_node::band_set_ast_build_options(union_set options)
{
    handle_ = isl_schedule_node_band_set_ast_build_options(handle_, options.release());
}

union_set schedule_node::get_domain() const
{
    return union_set(isl_schedule_node_get_domain(handle_));
}

union_map schedule_node::get_prefix_schedule_union_map() const
{
    return union_map(isl_schedule_node_get_prefix_schedule_union_map(handle_));
}

union_map schedule_node::get_prefix_schedule_relation() const
{
    return union_map(isl_schedule_node_get_prefix_schedule_relation(handle_));
}

union_map schedule_node::get_subtree_schedule_union_map() const
{
    return union_map(isl_schedule_node_get_subtree_schedule_union_map(handle_));
}

isl_schedule_node* schedule_node::native_handle() const
{
    return handle_;
}

isl_schedule_node* schedule_node::release() noexcept
{
    isl_schedule_node* tmp = handle_;

    handle_ = nullptr;

    return tmp;
}

schedule_node schedule_node::from_domain(union_set domain)
{
    return schedule_node(isl_schedule_node_from_domain(domain.release()));
}

schedule_node schedule_node::from_extension(union_map extension)
{
    return schedule_node(isl_schedule_node_from_extension(extension.release()));
}

schedule_node insert_partial_schedule(schedule_node parent, multi_union_pw_affine_expr schedule)
{
    return schedule_node(
        isl_schedule_node_insert_partial_schedule(parent.release(), schedule.release()));
}

schedule_node insert_context(schedule_node parent, set context)
{
    return schedule_node(isl_schedule_node_insert_context(parent.release(), context.release()));
}

schedule_node insert_filter(schedule_node parent, union_set filter)
{
    return schedule_node(isl_schedule_node_insert_filter(parent.release(), filter.release()));
}

schedule_node insert_set(schedule_node parent, std::vector<union_set> filters)
{
    isl_ctx* ctx = isl_schedule_node_get_ctx(parent.native_handle());

    isl_union_set_list* filters_ = isl_union_set_list_alloc(ctx, filters.size());

    for (auto& filter : filters)
    {
        filters_ = isl_union_set_list_add(filters_, filter.release());
    }

    return schedule_node(isl_schedule_node_insert_set(parent.release(), filters_));
}

schedule_node insert_sequence(schedule_node parent, std::vector<union_set> filters)
{
    isl_ctx* ctx = isl_schedule_node_get_ctx(parent.native_handle());

    isl_union_set_list* filters_ = isl_union_set_list_alloc(ctx, filters.size());

    for (auto& filter : filters)
    {
        filters_ = isl_union_set_list_add(filters_, filter.release());
    }

    return schedule_node(isl_schedule_node_insert_sequence(parent.release(), filters_));
}

schedule_node insert_mark(schedule_node parent, id mark)
{
    return schedule_node(isl_schedule_node_insert_mark(parent.release(), mark.release()));
}

schedule_node group(schedule_node node, id group_id)
{
    return schedule_node(isl_schedule_node_group(node.release(), group_id.release()));
}

schedule_node tile_band(schedule_node node, std::vector<long int> sizes)
{
    isl_ctx* ctx = isl_schedule_node_get_ctx(node.native_handle());

    isl_val_list* sizes_list = isl_val_list_alloc(ctx, sizes.size());

    for (auto size : sizes)
    {
        sizes_list = isl_val_list_add(sizes_list, isl_val_int_from_si(ctx, size));
    }

    isl_space* s = isl_space_set_alloc(ctx, 0, sizes.size());

    isl_multi_val* sizes_ = isl_multi_val_from_val_list(s, sizes_list);

    return schedule_node(isl_schedule_node_band_tile(node.release(), sizes_));
}

schedule_node graft_before(schedule_node node, schedule_node graft)
{
    return schedule_node(isl_schedule_node_graft_before(node.release(), graft.release()));
}

schedule_node graft_after(schedule_node node, schedule_node graft)
{
    return schedule_node(isl_schedule_node_graft_after(node.release(), graft.release()));
}

}
}