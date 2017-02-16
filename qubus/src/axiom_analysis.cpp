#include <qbb/qubus/axiom_analysis.hpp>

#include <qbb/qubus/IR/constant_folding.hpp>

#include <qbb/qubus/IR/qir.hpp>
#include <qbb/qubus/pattern/IR.hpp>
#include <qbb/qubus/pattern/core.hpp>

#include <qbb/util/assert.hpp>

#include <stack>
#include <utility>

inline namespace qbb
{
namespace qubus
{

namespace
{
std::unique_ptr<axiom_scope> generate_axiom_scope_tree(const expression& ctx,
                                                       std::vector<axiom> axioms)
{
    using pattern::_;

    pattern::variable<variable_declaration> idx;
    pattern::variable<const expression &> lower_bound, upper_bound, condition, then_branch,
        increment, body;
    pattern::variable<util::optional_ref<const expression>> else_branch;

    auto m =
        pattern::make_matcher<expression, std::unique_ptr<axiom_scope>>()
            .case_(for_(idx, lower_bound, upper_bound, increment, body),
                   [&] {
                       std::vector<axiom> body_axioms;
                       std::vector<std::unique_ptr<axiom_scope>> subscopes;
                       subscopes.reserve(ctx.arity());

                       // The iteration interval is expressed via two axioms to aid the
                       // consumers of this information. In general, it is better to decompose all axioms which are
                       // formed through conjunction into distinct axioms (The conjunction is implied by our
                       // definition of an axiom). As an example, this is beneficial if the consumer imposes additional
                       // requirement on valid axioms. If these requirements do not hold for a part of this axiom
                       // one only needs to throw away that portion and not the entire composite axiom.

                       auto lower_bound_axiom =
                           axiom(greater_equal(variable_ref(idx.get()), clone(lower_bound.get())));

                       pattern::variable<util::index_t> value;

                       auto m2 =
                           pattern::make_matcher<expression, axiom>()
                               .case_(integer_literal(value),
                                      [&] {
                                          if (value.get() == 1)
                                          {
                                              return axiom(less(variable_ref(idx.get()),
                                                                clone(upper_bound.get())));
                                          }
                                          else
                                          {
                                              auto bound =
                                                  clone(upper_bound.get()) -
                                                  (clone(upper_bound.get()) -
                                                   clone(lower_bound.get()) - integer_literal(1)) %
                                                      clone(increment.get());

                                              return axiom(less(variable_ref(idx.get()),
                                                                fold_constant_expressions(*bound)));
                                          }
                                      })
                               .case_(_, [&] {
                                   return axiom(
                                       less(variable_ref(idx.get()), clone(upper_bound.get())));
                               });

                       auto upper_bound_axiom = pattern::match(increment.get(), m2);

                       auto stride_axiom =
                           axiom(equal_to((variable_ref(idx.get()) - clone(lower_bound.get())) %
                                              clone(increment.get()),
                                          integer_literal(0)));

                       // We can assume that the range is non-empty in the loop body
                       // since we would not execute it otherwise.
                       auto non_empty_axiom = axiom(less(clone(lower_bound.get()), clone(upper_bound.get())));

                       body_axioms.push_back(std::move(lower_bound_axiom));
                       body_axioms.push_back(std::move(upper_bound_axiom));
                       body_axioms.push_back(std::move(stride_axiom));
                       body_axioms.push_back(std::move(non_empty_axiom));

                       auto lower_bound_scope = generate_axiom_scope_tree(lower_bound.get(), {});
                       auto upper_bound_scope = generate_axiom_scope_tree(upper_bound.get(), {});
                       auto increment_scope = generate_axiom_scope_tree(increment.get(), {});
                       auto body_scope =
                           generate_axiom_scope_tree(body.get(), std::move(body_axioms));

                       subscopes.push_back(std::move(lower_bound_scope));
                       subscopes.push_back(std::move(upper_bound_scope));
                       subscopes.push_back(std::move(increment_scope));
                       subscopes.push_back(std::move(body_scope));

                       return std::make_unique<axiom_scope>(ctx, axioms, std::move(subscopes));
                   })
            .case_(if_(condition, then_branch, else_branch),
                   [&] {
                       std::vector<std::unique_ptr<axiom_scope>> subscopes;
                       subscopes.reserve(ctx.arity());

                       // Note: Currently, we just forward the entire condition. As stated within the for_ case,
                       //       it is beneficial to decompose conjunctions which we should also do in this case at least
                       //       for trivial cases. No need to build a full-fledged optimizer for logical expressions though ;-).

                       auto then_axiom = axiom(clone(condition.get()));

                       auto condition_scope = generate_axiom_scope_tree(condition.get(), {});
                       auto then_scope = generate_axiom_scope_tree(then_branch.get(), {then_axiom});

                       subscopes.push_back(std::move(condition_scope));
                       subscopes.push_back(std::move(then_scope));

                       if (else_branch.get())
                       {
                           auto else_axiom = axiom(logical_not(clone(condition.get())));

                           auto else_scope =
                               generate_axiom_scope_tree(*else_branch.get(), {else_axiom});

                           subscopes.push_back(std::move(else_scope));
                       }

                       return std::make_unique<axiom_scope>(ctx, axioms, std::move(subscopes));
                   })
            .case_(_, [&] {
                std::vector<std::unique_ptr<axiom_scope>> subscopes;
                subscopes.reserve(ctx.arity());

                for (const auto& expr : ctx.sub_expressions())
                {
                    subscopes.push_back(generate_axiom_scope_tree(expr, {}));
                }

                return std::make_unique<axiom_scope>(ctx, axioms, std::move(subscopes));
            });

    auto root = pattern::match(ctx, m);

    QBB_ASSERT(root->arity() == ctx.arity(), "Unexpected number of subscopes.");

    return root;
}
}

axiom_scope::axiom_scope(const expression& context_, std::vector<axiom> axioms_,
                         std::vector<std::unique_ptr<axiom_scope>> subscopes_)
: context_(&context_),
  axioms_(std::move(axioms_)),
  parent_(nullptr),
  subscopes_(std::move(subscopes_))
{
    for (auto& scope : this->subscopes_)
    {
        scope->set_parent(this);
    }
}

axiom::axiom(std::unique_ptr<const expression> expr_) : expr_(std::move(expr_))
{
}

const expression& axiom::as_expr() const
{
    return *expr_;
}

axiom_analysis_result::axiom_analysis_result(std::unique_ptr<axiom_scope> root_scope_)
: root_scope_(std::move(root_scope_))
{
    auto root = this->root_scope_.get();

    std::stack<const axiom_scope*> unfinished_scopes_;

    unfinished_scopes_.push(root);

    while (!unfinished_scopes_.empty())
    {
        const auto& current_scope = *unfinished_scopes_.top();
        unfinished_scopes_.pop();

        axiom_scope_index_.emplace(&current_scope.context(), &current_scope);

        for (const auto& subscope : current_scope.subscopes())
        {
            unfinished_scopes_.push(&subscope);
        }
    }
}

std::vector<axiom> axiom_analysis_result::get_valid_axioms(const expression& ctx) const
{
    auto search_result = axiom_scope_index_.find(&ctx);

    QBB_ASSERT(search_result != axiom_scope_index_.end(), "Invalid context.");

    auto root = search_result->second;

    std::stack<const axiom_scope*> unfinished_scopes_;

    unfinished_scopes_.push(root);

    std::vector<axiom> axioms;

    while (!unfinished_scopes_.empty())
    {
        const auto& current_scope = *unfinished_scopes_.top();
        unfinished_scopes_.pop();

        for (const auto& axiom : current_scope.axioms())
        {
            axioms.push_back(axiom);
        }

        if (auto parent = current_scope.parent())
        {
            unfinished_scopes_.push(parent);
        }
    }

    return axioms;
}

axiom_analysis_result axiom_analysis_pass::run(const expression& root, analysis_manager& manager,
                                               pass_resource_manager& resource_manager) const
{
    return axiom_analysis_result(generate_axiom_scope_tree(root, {}));
}

std::vector<analysis_id> axiom_analysis_pass::required_analyses() const
{
    return {};
}

QUBUS_REGISTER_ANALYSIS_PASS(axiom_analysis_pass);
}
}