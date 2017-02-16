#include <qbb/qubus/isl/ast.hpp>

namespace qubus
{
namespace isl
{

ast_expr::ast_expr(isl_ast_expr* handle_) : handle_{handle_}
{
}

ast_expr::ast_expr(const ast_expr& other) : handle_(isl_ast_expr_copy(other.native_handle()))
{
}

ast_expr::~ast_expr()
{
    isl_ast_expr_free(native_handle());
}

ast_expr& ast_expr::operator=(const ast_expr& other)
{
    isl_ast_expr_free(handle_);

    handle_ = isl_ast_expr_copy(other.native_handle());

    return *this;
}

isl_ast_expr_type ast_expr::type() const
{
    return isl_ast_expr_get_type(handle_);
}

id ast_expr::get_id() const
{
    return id(isl_ast_expr_get_id(handle_));
}

value ast_expr::get_value() const
{
    return value(isl_ast_expr_get_val(handle_));
}

isl_ast_op_type ast_expr::get_op_type() const
{
    return isl_ast_expr_get_op_type(handle_);
}

int ast_expr::arg_count() const
{
    return isl_ast_expr_get_op_n_arg(handle_);
}

ast_expr ast_expr::get_arg(int pos) const
{
    return ast_expr(isl_ast_expr_get_op_arg(handle_, pos));
}

isl_ast_expr* ast_expr::native_handle() const
{
    return handle_;
}

isl_ast_expr* ast_expr::release() noexcept
{
    isl_ast_expr* temp = handle_;

    handle_ = nullptr;

    return temp;
}

namespace
{
extern "C" isl_stat add_child(isl_ast_node* child, void* user) noexcept;
}

ast_node::ast_node(isl_ast_node* root_) : root_{root_}
{
}

ast_node::ast_node(const ast_node& other) : root_(isl_ast_node_copy(other.native_handle()))
{
}

ast_node::~ast_node()
{
    isl_ast_node_free(root_);
}

ast_node& ast_node::operator=(const ast_node& other)
{
    isl_ast_node_free(root_);

    root_ = isl_ast_node_copy(other.native_handle());

    return *this;
}

isl_ast_node* ast_node::native_handle() const
{
    return root_;
}

isl_ast_node* ast_node::release() noexcept
{
    isl_ast_node* temp = root_;

    root_ = nullptr;

    return temp;
}

isl_ast_node_type ast_node::type() const
{
    return isl_ast_node_get_type(root_);
}

ast_expr ast_node::for_get_iterator() const
{
    return ast_expr(isl_ast_node_for_get_iterator(root_));
}

ast_expr ast_node::for_get_init() const
{
    return ast_expr(isl_ast_node_for_get_init(root_));
}

ast_expr ast_node::for_get_cond() const
{
    return ast_expr(isl_ast_node_for_get_cond(root_));
}

ast_expr ast_node::for_get_inc() const
{
    return ast_expr(isl_ast_node_for_get_inc(root_));
}

ast_node ast_node::for_get_body() const
{
    return ast_node(isl_ast_node_for_get_body(root_));
}

ast_expr ast_node::if_get_cond() const
{
    return ast_expr(isl_ast_node_if_get_cond(root_));
}

ast_node ast_node::if_get_then() const
{
    return ast_node(isl_ast_node_if_get_then(root_));
}

boost::optional<ast_node> ast_node::if_get_else() const
{
    if (isl_ast_node_if_has_else(root_))
    {
        return ast_node(isl_ast_node_if_get_else(root_));
    }
    else
    {
        return {};
    }
}

ast_expr ast_node::user_get_expr() const
{
    return ast_expr(isl_ast_node_user_get_expr(root_));
}

std::vector<ast_node> ast_node::block_get_children() const
{
    std::vector<ast_node> result;

    isl_ast_node_list* children = isl_ast_node_block_get_children(root_);

    isl_ast_node_list_foreach(children, add_child, &result);

    isl_ast_node_list_free(children);

    return result;
}

id ast_node::mark_get_id() const
{
    return id(isl_ast_node_mark_get_id(root_));
}

ast_node ast_node::mark_get_node() const
{
    return ast_node(isl_ast_node_mark_get_node(root_));
}

namespace
{

extern "C" {

isl_stat add_child(isl_ast_node* child, void* user) noexcept
{
    auto& children = *static_cast<std::vector<ast_node>*>(user);

    children.emplace_back(child);

    return isl_stat_ok;
}
}
}
}
}