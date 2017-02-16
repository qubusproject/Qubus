#include <qbb/qubus/static_schedule_analysis.hpp>

#include <qbb/qubus/task_invariants_analysis.hpp>

#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/qubus/IR/type_inference.hpp>

#include <qbb/qubus/isl/affine_expr.hpp>
#include <qbb/qubus/isl/ast_builder.hpp>
#include <qbb/qubus/isl/constraint.hpp>
#include <qbb/qubus/isl/flow.hpp>
#include <qbb/qubus/isl/map.hpp>
#include <qbb/qubus/isl/multi_affine_expr.hpp>
#include <qbb/qubus/isl/printer.hpp>
#include <qbb/qubus/isl/schedule.hpp>
#include <qbb/qubus/isl/set.hpp>

#include <qbb/util/assert.hpp>
#include <qbb/util/unreachable.hpp>
#include <qbb/util/unused.hpp>

#include <functional>

namespace qubus
{

static_schedule_analysis_result::static_schedule_analysis_result(
    std::unordered_map<const expression*, static_schedule> schedule_table_)
: schedule_table_(std::move(schedule_table_))
{
}

util::optional_ref<const static_schedule>
static_schedule_analysis_result::get_schedule_by_root(const expression& root) const
{
    auto search_result = schedule_table_.find(&root);

    if (search_result != schedule_table_.end())
    {
        return search_result->second;
    }
    else
    {
        return {};
    }
}

util::optional_ref<const static_schedule>
static_schedule_analysis_result::get_schedule_containing(const expression& expr) const
{
    auto cursor = expr.cursor();
    auto schedule = get_schedule_by_root(expr);

    while (cursor && !schedule)
    {
        cursor.move_up();
        schedule = get_schedule_by_root(*cursor);
    }

    return schedule;
}

namespace
{

enum class static_schedule_accuracy
{
    exact,
    approximative
};

class domain
{
public:
    std::vector<affine_constraint> constraints;
};

boost::optional<isl::set> try_construct_statement(const expression& expr,
                                                  const domain& iteration_domain,
                                                  static_schedule_context& ctx,
                                                  isl::context& isl_ctx)
{
    auto& stmt = ctx.register_statement(expr);

    auto iteration_space = ctx.affine_ctx().construct_corresponding_space(isl_ctx);

    auto domain = isl::set::universe(iteration_space);

    for (const auto& constraint : iteration_domain.constraints)
    {
        domain = intersect(std::move(domain), constraint.convert(isl_ctx));
    }

    domain.set_tuple_name(stmt.name);

    domain = simplify(std::move(domain));

    return domain;
}

struct sub_schedule
{
    using ir_schedule_map_type = std::unordered_map<const expression*, isl::schedule_node>;

    sub_schedule(isl::union_set domain,
                 std::function<isl::schedule_node(isl::schedule_node)> construct_schedule,
                 std::function<void(isl::schedule_node, ir_schedule_map_type&)> register_mapping,
                 static_schedule_accuracy accuracy)
    : domain(std::move(domain)),
      construct_schedule(std::move(construct_schedule)),
      register_mapping(std::move(register_mapping)),
      accuracy(std::move(accuracy))
    {
    }

    isl::union_set domain;
    std::function<isl::schedule_node(isl::schedule_node)> construct_schedule;
    std::function<void(isl::schedule_node, ir_schedule_map_type&)> register_mapping;
    static_schedule_accuracy accuracy;
};

void add_schedule(std::unordered_map<const expression*, static_schedule>& schedule_table,
                  const expression& root, sub_schedule root_schedule)
{
    auto sched_root = isl::schedule_node::from_domain(simplify(root_schedule.domain));

    sched_root = root_schedule.construct_schedule(sched_root[0]);

    std::unordered_map<const expression*, isl::schedule_node> ir_schedule_map;

    root_schedule.register_mapping(std::move(sched_root), ir_schedule_map);

    schedule_table.emplace(&root, static_schedule(std::move(ir_schedule_map)));
}

boost::optional<sub_schedule>
construct_schedule(std::unordered_map<const expression*, static_schedule>& schedule_table,
                   const expression& expr, domain iteration_domain, static_schedule_context& ctx,
                   isl::context& isl_ctx,
                   const task_invariants_analysis_result& task_invariant_analysis);

void restart_construction(std::unordered_map<const expression*, static_schedule>& schedule_table,
                          const expression& root, isl::context& isl_ctx,
                          const task_invariants_analysis_result& task_invariant_analysis)
{
    auto is_invariant = [&root, &task_invariant_analysis](const expression& expr) {
        return task_invariant_analysis.is_invariant(expr, root);
    };

    auto aff_ctx = std::make_shared<affine_expr_context>(std::move(is_invariant));

    static_schedule_context ctx(std::move(aff_ctx));

    domain iteration_domain;

    auto root_schedule = construct_schedule(schedule_table, root, iteration_domain, ctx, isl_ctx,
                                            task_invariant_analysis);

    if (root_schedule)
    {
        add_schedule(schedule_table, root, *std::move(root_schedule));
    }
}

boost::optional<sub_schedule>
construct_schedule(std::unordered_map<const expression*, static_schedule>& schedule_table,
                   const expression& expr, domain iteration_domain, static_schedule_context& ctx,
                   isl::context& isl_ctx,
                   const task_invariants_analysis_result& task_invariant_analysis)
{
    using pattern::_;

    pattern::variable<variable_declaration> var;
    pattern::variable<const expression &> a, b, c, d;
    pattern::variable<std::vector<std::reference_wrapper<const expression>>> expressions;
    pattern::variable<util::optional_ref<const expression>> opt;
    pattern::variable<execution_order> order;

    auto m =
        pattern::make_matcher<expression, boost::optional<sub_schedule>>()
            .case_(for_(var, a, b, c, d),
                   [&](const expression& self) -> boost::optional<sub_schedule> {
                       auto lower_bound = try_construct_affine_expr(a.get(), ctx.affine_ctx());
                       auto upper_bound = try_construct_affine_expr(b.get(), ctx.affine_ctx());
                       auto increment = try_construct_affine_expr(c.get(), ctx.affine_ctx());

                       if (!lower_bound || !upper_bound || !increment)
                       {
                           restart_construction(schedule_table, d.get(), isl_ctx,
                                                task_invariant_analysis);

                           return boost::none;
                       }

                       // The increment needs to be constant to form a static schedule.
                       if (!is_const(*increment))
                       {
                           restart_construction(schedule_table, d.get(), isl_ctx,
                                                task_invariant_analysis);

                           return boost::none;
                       }

                       auto iterator = ctx.affine_ctx().declare_variable(var.get());

                       auto lower_bound_constraint = greater_equal(iterator, *lower_bound);
                       auto upper_bound_constraint = less(iterator, *upper_bound);

                       QBB_ASSERT(is_const(*increment),
                                  "The increment needs to be constant to form a static schedule.");
                       auto increment_constraint =
                           equal_to(iterator % *increment, ctx.affine_ctx().create_literal(0));

                       iteration_domain.constraints.push_back(lower_bound_constraint);
                       iteration_domain.constraints.push_back(upper_bound_constraint);
                       iteration_domain.constraints.push_back(increment_constraint);

                       auto body_schedule =
                           construct_schedule(schedule_table, d.get(), iteration_domain, ctx,
                                              isl_ctx, task_invariant_analysis);

                       if (!body_schedule)
                           return boost::none;

                       auto construct_schedule = [
                           construct_body = std::move(body_schedule->construct_schedule),
                           domain = body_schedule->domain, iterator, &isl_ctx
                       ](isl::schedule_node pos)
                       {
                           if (!is_empty(domain))
                           {
                               auto partial_schedule =
                                   isl::union_map::empty(isl::space(isl_ctx, 0));

                               for (const auto& set : domain.get_sets())
                               {
                                   auto range = isl::set::universe(isl::space(isl_ctx, 0, 1));

                                   auto stmt_sched =
                                       make_map_from_domain_and_range(set, std::move(range));

                                   auto sched_space = stmt_sched.get_space();

                                   QBB_ASSERT(is_variable(iterator),
                                              "The iterator should be a variable.");

                                   auto iter_pos = sched_space.find_dim_by_name(
                                       isl_dim_in, get_variable_name(iterator));

                                   QBB_ASSERT(iter_pos >= 0, "Invalid iteration position.");

                                   auto c = isl::constraint::equality(sched_space);
                                   c.set_coefficient(isl_dim_in, iter_pos, 1);
                                   c.set_coefficient(isl_dim_out, 0, -1);

                                   stmt_sched.add_constraint(c);

                                   partial_schedule = add_map(partial_schedule, stmt_sched);
                               }

                               auto node = insert_partial_schedule(
                                   pos, isl::multi_union_pw_affine_expr::from_union_map(
                                            simplify(partial_schedule)));

                               node = construct_body(node[0]).parent();

                               return node;
                           }
                           else
                           {
                               // If the loop does not contain any statements,
                               // we can eliminate the loop.
                               // Note: This is actually necessary since we can not construct
                               //       an empty schedule.

                               auto node = insert_mark(pos, isl::id(isl_ctx, "empty_schedule"));

                               node = construct_body(node[0]).parent();

                               return node;
                           }
                       };

                       auto register_mapping =
                           [&self, register_body = body_schedule->register_mapping ](
                               isl::schedule_node current,
                               std::unordered_map<const expression*, isl::schedule_node> &
                                   ir_schedule_map)
                       {
                           ir_schedule_map.emplace(&self, current);

                           register_body(current[0], ir_schedule_map);
                       };

                       return sub_schedule(
                           std::move(body_schedule->domain), std::move(construct_schedule),
                           std::move(register_mapping), static_schedule_accuracy::exact);
                   })
            .case_(if_(a, b, opt),
                   [&](const expression& self) -> boost::optional<sub_schedule> {
                       auto condition = try_extract_affine_constraint(a.get(), ctx.affine_ctx());

                       if (!condition)
                       {
                           restart_construction(schedule_table, b.get(), isl_ctx,
                                                task_invariant_analysis);

                           if (opt.get())
                           {
                               restart_construction(schedule_table, *opt.get(), isl_ctx,
                                                    task_invariant_analysis);
                           }

                           return boost::none;
                       }
                       else
                       {
                           auto then_domain = iteration_domain;
                           then_domain.constraints.push_back(*condition);

                           auto then_schedule =
                               construct_schedule(schedule_table, b.get(), then_domain, ctx,
                                                  isl_ctx, task_invariant_analysis);

                           if (!then_schedule)
                               return boost::none;

                           if (opt.get())
                           {
                               auto else_domain = iteration_domain;
                               else_domain.constraints.push_back(!*condition);

                               auto else_schedule =
                                   construct_schedule(schedule_table, *opt.get(), else_domain, ctx,
                                                      isl_ctx, task_invariant_analysis);

                               if (!else_schedule)
                                   return boost::none;

                               auto construct_schedule = [
                                   construct_then_branch =
                                       std::move(then_schedule->construct_schedule),
                                   construct_else_branch =
                                       std::move(else_schedule->construct_schedule),
                                   then_domain = then_schedule->domain,
                                   else_domain = else_schedule->domain
                               ](isl::schedule_node pos)
                               {
                                   auto node = insert_set(pos, {then_domain, else_domain});

                                   node = construct_then_branch(node[0][0]).parent().parent();
                                   node = construct_else_branch(node[1][0]).parent().parent();

                                   return node;
                               };

                               auto register_mapping =
                                   [
                                     &self, register_then_branch =
                                                std::move(then_schedule->register_mapping),
                                     register_else_branch =
                                         std::move(else_schedule->register_mapping)
                                   ](isl::schedule_node current,
                                     std::unordered_map<const expression*, isl::schedule_node> &
                                         ir_schedule_map)
                               {
                                   ir_schedule_map.emplace(&self, current);

                                   register_then_branch(current[0][0], ir_schedule_map);
                                   register_else_branch(current[1][0], ir_schedule_map);
                               };

                               auto domain = union_(then_schedule->domain, else_schedule->domain);

                               return sub_schedule(domain, std::move(construct_schedule),
                                                   std::move(register_mapping),
                                                   static_schedule_accuracy::exact);
                           }
                           else
                           {
                               auto construct_schedule = [
                                   construct_then_branch =
                                       std::move(then_schedule->construct_schedule),
                                   then_domain = then_schedule->domain
                               ](isl::schedule_node pos)
                               {
                                   auto node = insert_set(pos, {then_domain});

                                   node = construct_then_branch(node[0][0]).parent().parent();

                                   return node;
                               };

                               auto register_mapping =
                                   [
                                     &self, register_then_branch =
                                                std::move(then_schedule->register_mapping)
                                   ](isl::schedule_node current,
                                     std::unordered_map<const expression*, isl::schedule_node> &
                                         ir_schedule_map)
                               {
                                   ir_schedule_map.emplace(&self, current);

                                   register_then_branch(current[0][0], ir_schedule_map);
                               };

                               return sub_schedule(
                                   then_schedule->domain, std::move(construct_schedule),
                                   std::move(register_mapping), static_schedule_accuracy::exact);
                           }
                       }
                   })
            .case_(compound(order, expressions),
                   [&](const expression& self) -> boost::optional<sub_schedule> {
                       auto domain = isl::union_set::empty(isl::space(isl_ctx, 0));

                       std::vector<isl::union_set> domain_seq;
                       std::vector<std::function<isl::schedule_node(isl::schedule_node)>>
                           construct_children;
                       std::vector<std::function<void(isl::schedule_node,
                                                      sub_schedule::ir_schedule_map_type&)>>
                           register_children;

                       if (!expressions.get().empty())
                       {
                           for (const auto& expr : expressions.get())
                           {
                               auto child_schedule =
                                   construct_schedule(schedule_table, expr, iteration_domain, ctx,
                                                      isl_ctx, task_invariant_analysis);

                               if (!child_schedule)
                                   return boost::none;

                               domain = union_(domain, child_schedule->domain);

                               domain_seq.push_back(std::move(child_schedule->domain));

                               construct_children.push_back(
                                   std::move(child_schedule->construct_schedule));
                               register_children.push_back(
                                   std::move(child_schedule->register_mapping));
                           }

                           auto construct_schedule = [
                               domain_seq, construct_children = std::move(construct_children),
                               order = order.get()
                           ](isl::schedule_node pos)
                           {
                               auto child_count = domain_seq.size();

                               QBB_ASSERT(child_count == construct_children.size(),
                                          "Wrong number of constructors.");

                               auto node = [&] {
                                   switch (order)
                                   {
                                   case execution_order::sequential:
                                       return insert_sequence(pos, domain_seq);
                                   case execution_order::unordered:
                                       return insert_set(pos, domain_seq);
                                   default:
                                       QBB_UNREACHABLE();
                                   }
                               }();

                               for (std::size_t i = 0; i < child_count; ++i)
                               {
                                   node = construct_children[i](node[i][0]).parent().parent();
                               }

                               return node;
                           };

                           auto register_mapping =
                               [&self, register_children = std::move(register_children) ](
                                   isl::schedule_node current,
                                   std::unordered_map<const expression*, isl::schedule_node> &
                                       ir_schedule_map)
                           {
                               ir_schedule_map.emplace(&self, current);

                               auto child_count = register_children.size();

                               for (int i = 0; i < child_count; ++i)
                               {
                                   register_children[i](current[i][0], ir_schedule_map);
                               }
                           };

                           return sub_schedule(std::move(domain), std::move(construct_schedule),
                                               std::move(register_mapping),
                                               static_schedule_accuracy::exact);
                       }
                       else
                       {
                           // If the compound expression has no children,
                           // the schedule is empty and we have to emit an 'empty_schedule' mark node
                           // instead since sequence/set nodes without any children are broken.

                           auto construct_schedule = [&isl_ctx](isl::schedule_node pos) {
                               return insert_mark(pos, isl::id(isl_ctx, "empty_schedule"));
                           };

                           auto register_mapping = [&self](
                               isl::schedule_node current,
                               std::unordered_map<const expression*, isl::schedule_node>&
                                   ir_schedule_map) { ir_schedule_map.emplace(&self, current); };

                           return sub_schedule(std::move(domain), std::move(construct_schedule),
                                               std::move(register_mapping),
                                               static_schedule_accuracy::exact);
                       }
                   })
            .case_(_, [&](const expression& self) -> boost::optional<sub_schedule> {
                auto stmt = try_construct_statement(self, iteration_domain, ctx, isl_ctx);

                if (!stmt)
                    return boost::none;

                return sub_schedule(
                    isl::union_set(*std::move(stmt)), [](isl::schedule_node pos) { return pos; },
                    [&self](isl::schedule_node current,
                            std::unordered_map<const expression*, isl::schedule_node>&
                                ir_schedule_map) { ir_schedule_map.emplace(&self, current); },
                    static_schedule_accuracy::exact);
            });

    return pattern::match(expr, m);
}
}

static_schedule_analysis_result
static_schedule_analysis_pass::run(const expression& root, analysis_manager& manager,
                                   pass_resource_manager& resource_manager) const
{
    std::unordered_map<const expression*, static_schedule> schedule_table;

    restart_construction(schedule_table, root, resource_manager.get_isl_ctx(),
                         manager.get_analysis<task_invariants_analysis_pass>(root));

    return static_schedule_analysis_result(std::move(schedule_table));
}

std::vector<analysis_id> static_schedule_analysis_pass::required_analyses() const
{
    return {get_analysis_id<task_invariants_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(static_schedule_analysis_pass);
}