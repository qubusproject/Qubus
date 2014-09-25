#ifndef QBB_KUBUS_ISL_AST_HPP
#define QBB_KUBUS_ISL_AST_HPP

#include <qbb/kubus/isl/identifier.hpp>
#include <qbb/kubus/isl/value.hpp>

#include <boost/optional.hpp>

#include <isl/ast.h>

namespace qbb
{
namespace kubus
{
namespace isl
{

class ast_expr
{
public:
    explicit ast_expr(isl_ast_expr* handle_);

    isl_ast_expr_type type() const;

    identifier get_id() const;

    value get_value() const;

    isl_ast_op_type get_op_type() const;

    int arg_count() const;

    ast_expr get_arg(int pos) const;

private:
    isl_ast_expr* handle_;
};

class ast_node
{
public:
    explicit ast_node(isl_ast_node* root_);

    ~ast_node();

    isl_ast_node* native_handle() const;

    isl_ast_node_type type() const;

    ast_expr for_get_iterator() const;

    ast_expr for_get_init() const;

    ast_expr for_get_cond() const;

    ast_expr for_get_inc() const;

    ast_node for_get_body() const;

    ast_expr if_get_cond() const;

    ast_node if_get_then() const;

    boost::optional<ast_node> if_get_else() const;

    ast_expr user_get_expr() const;

    std::vector<ast_node> block_get_children() const;

private:
    isl_ast_node* root_;
};
}
}
}

#endif