#ifndef QUBUS_PATTERN_SPAWN_HPP
#define QUBUS_PATTERN_SPAWN_HPP

#include <qubus/IR/spawn_expr.hpp>

#include <qubus/pattern/variable.hpp>
#include <qubus/pattern/sequence.hpp>

#include <utility>
#include <functional>

namespace qubus
{
namespace pattern
{

template <typename SpawnedPlan, typename Arguments>
class spawn_pattern
{
public:
    spawn_pattern(SpawnedPlan spawned_plan_, Arguments arguments_)
    : spawned_plan_(std::move(spawned_plan_)), arguments_(std::move(arguments_))
    {
    }

    template <typename BaseType>
    bool match(const BaseType& value, const variable<const spawn_expr&>* var = nullptr) const
    {
        if (auto concret_value = value.template try_as<spawn_expr>())
        {
            if (spawned_plan_.match(concret_value->spawned_plan()))
            {
                if (arguments_.match(concret_value->arguments()))
                {
                    if (var)
                    {
                        var->set(*concret_value);
                    }

                    return true;
                }
            }
        }

        return false;
    }

    void reset() const
    {
        spawned_plan_.reset();
        arguments_.reset();
    }
private:
    SpawnedPlan spawned_plan_;
    Arguments arguments_;
};

template <typename SpawnedPlan, typename Arguments>
spawn_pattern<SpawnedPlan, Arguments> spawn(SpawnedPlan spawned_plan, Arguments arguments)
{
    return spawn_pattern<SpawnedPlan, Arguments>(spawned_plan, arguments);
}

template <typename SpawnedPlan, typename... Arguments>
auto spawn_n(SpawnedPlan spawned_plan, Arguments... arguments)
{
    return spawn(spawned_plan, sequence(arguments...));
}

}
}

#endif