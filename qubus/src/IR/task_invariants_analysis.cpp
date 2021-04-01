#include <qubus/IR/task_invariants_analysis.hpp>

#include <qubus/IR/alias_analysis.hpp>

#include <qubus/pattern/IR.hpp>
#include <qubus/pattern/core.hpp>

#include <qubus/util/assert.hpp>

namespace qubus
{

namespace
{
bool is_referencing_invariant_object(const expression& obj, const expression& context,
                                     const basic_alias_analysis_result& basic_alias_analysis)
{
    using pattern::_;
    using pattern::value;

    pattern::variable<const expression&> modified_object;
    pattern::variable<std::shared_ptr<variable_declaration>> var;

    bool is_modified = false;

    auto m =
        pattern::make_matcher<expression, void>()
            .case_(assign(modified_object, _) || plus_assign(modified_object, _),
                   [&] {
                       const access_expr* modifing_access =
                           modified_object.get().try_as<access_expr>();
                       const access_expr* obj_access = obj.try_as<access_expr>();

                       QUBUS_ASSERT(modifing_access && obj_access, "Invalid object during access.");

                       auto query_result = basic_alias_analysis.alias(access(*obj_access),
                                                                      access(*modifing_access));

                       if (query_result != alias_result::noalias)
                       {
                           is_modified = true;
                       }
                   })
            .case_(for_(var, _, _, _, _), [&] {
                auto modifing_access = variable_ref(var.get());
                const access_expr* obj_access = obj.try_as<access_expr>();

                QUBUS_ASSERT(obj_access, "Invalid object during access.");

                // FIXME: Currently, we use the non-caching variant of the basic alias analysis algorithm
                //        since the cache gets confused by temporary expression objects.
                auto query_result =
                        basic_alias_analysis.alias(access(*obj_access), access(*modifing_access), false);

                if (query_result != alias_result::noalias)
                {
                    is_modified = true;
                }
            });

    pattern::for_each(context, m);

    return !is_modified;
}
}

task_invariants_analysis_result::task_invariants_analysis_result(
    const basic_alias_analysis_result& basic_alias_analysis_)
: basic_alias_analysis_(&basic_alias_analysis_)
{
}

bool task_invariants_analysis_result::is_invariant(const expression& expr,
                                                   const expression& context) const
{
    using pattern::_;

    auto m =
        pattern::make_matcher<expression, bool>()
            .case_(pattern::access(),
                   [&](const expression& self) {
                       return is_referencing_invariant_object(self, context,
                                                              *basic_alias_analysis_);
                   })
            // Literals are invariant by definition
            .case_(double_literal(_) || float_literal(_) || integer_literal(_), [] { return true; })
            // Special rule for array shapes
            // TODO: We should generalize this. Maybe with a special type attribute
            // TODO: We should test if the object is actually a built-in array. For now it should be fine, though.
            .case_(member_access(_, pattern::value("shape")), [&] { return true; })
            // The following operations are all pure functions. Their results are invariant if all
            // their arguments are invariants themselves.
            .case_(binary_operator(_, _, _) || unary_operator(_, _) || member_access(_, _) ||
                       subscription(_, _),
                   [&](const expression& self) {
                       for (const auto& child : self.sub_expressions())
                       {
                           if (!is_invariant(child, context))
                               return false;
                       }

                       return true;
                   })
            .case_(_, [] { return false; });

    return pattern::match(expr, m);
}

task_invariants_analysis_result
task_invariants_analysis_pass::run(const expression& root, analysis_manager& manager,
                                   pass_resource_manager& resource_manager) const
{

    return task_invariants_analysis_result(manager.get_analysis<basic_alias_analysis_pass>(root));
}

std::vector<analysis_id> task_invariants_analysis_pass::required_analyses() const
{
    return {get_analysis_id<basic_alias_analysis_pass>()};
}

QUBUS_REGISTER_ANALYSIS_PASS(task_invariants_analysis_pass);
}