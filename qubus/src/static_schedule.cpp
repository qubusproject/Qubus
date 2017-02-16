#include <qbb/qubus/static_schedule.hpp>

#include <algorithm>
#include <utility>

inline namespace qbb
{
namespace qubus
{
static_schedule_statement::static_schedule_statement(std::string name, const expression& expr)
: name(std::move(name)), expr(expr)
{
}

static_schedule_context::static_schedule_context(std::shared_ptr<affine_expr_context> affine_ctx_)
: affine_ctx_(std::move(affine_ctx_))
{
}

static_schedule_statement& static_schedule_context::register_statement(const expression& stmt)
{
    auto name = "stmt" + std::to_string(free_statement_id_++);

    return statement_table_.emplace(name, static_schedule_statement(name, stmt)).first->second;
}

affine_expr_context& static_schedule_context::affine_ctx()
{
    return *affine_ctx_;
}

static_schedule::static_schedule(std::unordered_map<const expression*, isl::schedule_node> ir_schedule_map_)
: ir_schedule_map_(std::move(ir_schedule_map_))
{
}

const isl::schedule_node& static_schedule::get_schedule_node(const expression& expr) const
{
    auto search_result = ir_schedule_map_.find(&expr);

    if (search_result != ir_schedule_map_.end())
    {
        return search_result->second;
    }
    else
    {
        throw 0; // Error: Invalid expression
    }
}
}
}