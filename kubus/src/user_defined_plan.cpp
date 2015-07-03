#include <qbb/kubus/user_defined_plan.hpp>

#include <qbb/kubus/runtime.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{
plan user_defined_plan_builder::finalize()
{
    return get_runtime().register_user_defined_plan({std::move(intents_), std::move(body_)});
}
}
}