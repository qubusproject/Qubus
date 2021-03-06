#ifndef QUBUS_ISL_AST_BUILDER_HPP
#define QUBUS_ISL_AST_BUILDER_HPP

#include <qubus/isl/context.hpp>
#include <qubus/isl/schedule.hpp>
#include <qubus/isl/set.hpp>
#include <qubus/isl/map.hpp>
#include <qubus/isl/ast.hpp>
#include <qubus/isl/id.hpp>
#include <qubus/isl/pw_aff.hpp>
#include <qubus/isl/pw_multi_aff.hpp>
#include <qubus/isl/space.hpp>

#include <isl/ast_build.h>

#include <functional>

namespace qubus
{
namespace isl
{

class ast_builder_ref;
    
class ast_builder_base
{
public:
    explicit ast_builder_base(isl_ast_build* handle_);
    
    ast_node build_ast_from_schedule(union_map schedule);
    ast_node build_node_from_schedule(schedule sched);

    ast_expr build_expr_from_pw_aff(pw_aff f);
    ast_expr build_access_from_pw_multi_aff(pw_multi_aff pma);

    ast_expr build_expr_from_set(set s);

    void set_options(union_map options);
    
    void set_create_leaf(std::function<ast_node(ast_builder_ref)> callback);
    
    void set_at_each_domain(std::function<ast_node(ast_node, ast_builder_ref)> callback);
    
    void set_before_each_for(std::function<id(ast_builder_ref)> callback);
    void set_after_each_for(std::function<ast_node(ast_node, ast_builder_ref)> callback);

    union_map get_schedule() const;
    space get_schedule_space() const;

    isl_ast_build* native_handle() const;
protected:
    ast_builder_base(const ast_builder_base&) = default;
    ast_builder_base& operator=(const ast_builder_base&) = default;
    
    ast_builder_base(ast_builder_base&&) = default;
    ast_builder_base& operator=(ast_builder_base&&) = default;
private:
    isl_ast_build* handle_;
};

class ast_builder : public ast_builder_base
{
public:
    explicit ast_builder(isl_ast_build* handle_);

    ast_builder(const ast_builder& other) = delete;

    explicit ast_builder(const context& ctx);
    explicit ast_builder(set params);

    ~ast_builder();
    
    ast_builder& operator=(const ast_builder& other) = delete;
};

class ast_builder_ref : public ast_builder_base
{
public:
    using ast_builder_base::ast_builder_base;
};

}
}

#endif