#ifndef QUBUS_STATIC_SCHEDULE_HPP
#define QUBUS_STATIC_SCHEDULE_HPP

#include <qubus/IR/affine_constraints.hpp>

#include <qubus/IR/expression.hpp>
#include <qubus/IR/macro_expr.hpp>
#include <qubus/IR/variable_declaration.hpp>

#include <qubus/isl/map.hpp>
#include <qubus/isl/schedule_node.hpp>

#include <qubus/util/optional_ref.hpp>

#include <boost/optional.hpp>

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace qubus
{

struct static_schedule_statement
{
    static_schedule_statement(std::string name, const expression& expr);

    std::string name;
    std::reference_wrapper<const expression> expr;
};

class static_schedule_context
{
public:
    explicit static_schedule_context(std::shared_ptr<affine_expr_context> affine_ctx_);

    static_schedule_statement& register_statement(const expression& stmt);

    affine_expr_context& affine_ctx();

private:
    std::shared_ptr<affine_expr_context> affine_ctx_;

    std::size_t free_statement_id_ = 0;
    std::unordered_map<std::string, static_schedule_statement> statement_table_;
};

class static_schedule
{
public:
    explicit static_schedule(std::unordered_map<const expression*, isl::schedule_node> ir_schedule_map_);

    const isl::schedule_node& get_schedule_node(const expression& expr) const;

private:
    std::unordered_map<const expression*, isl::schedule_node> ir_schedule_map_;
};
}

#endif
