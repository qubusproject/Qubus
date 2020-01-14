#include <qubus/object.hpp>

#include <utility>

namespace qubus
{

object_id object::id() const
{
    return id_;
}

} // namespace qubus