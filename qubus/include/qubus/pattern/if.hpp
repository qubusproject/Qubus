#ifndef QUBUS_IF_HPP
#define QUBUS_IF_HPP

#include <qubus/IR/if_expr.hpp>

#include <qubus/pattern/variable.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{

template <typename Condition, typename ThenBranch, typename ElseBranch>
class if_pattern
{
public:
    if_pattern(Condition condition_, ThenBranch then_branch_, ElseBranch else_branch_)
    : condition_(std::move(condition_)), then_branch_(std::move(then_branch_)),
      else_branch_(std::move(else_branch_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const if_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<if_expr>())
        {
            if (condition_.match(concret_value->condition()))
            {
                if (then_branch_.match(concret_value->then_branch()))
                {
                    if (else_branch_.match(concret_value->else_branch()))
                    {
                        if (var)
                        {
                            var->set(*concret_value);
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    }

    void reset() const
    {
        condition_.reset();
        then_branch_.reset();
        else_branch_.reset();
    }

private:
    Condition condition_;
    ThenBranch then_branch_;
    ElseBranch else_branch_;
};

template <typename Condition, typename ThenBranch, typename ElseBranch>
if_pattern<Condition, ThenBranch, ElseBranch> if_(Condition condition, ThenBranch then_branch,
                                                  ElseBranch else_branch)
{
    return if_pattern<Condition, ThenBranch, ElseBranch>(condition, then_branch, else_branch);
}

}
}

#endif
