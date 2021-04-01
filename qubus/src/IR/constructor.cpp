#include <qubus/IR/constructor.hpp>

#include <qubus/IR/types.hpp>

namespace qubus
{

constructor::constructor(const object_type& parent_type_, std::vector<initializer> initializers_,
                     std::unique_ptr<expression> body_)
    : parent_type_(&parent_type_), initializers_(std::move(initializers_)), body_(std::move(body_))
{
    QUBUS_ASSERT(body_ != nullptr, "The constructor body must not be null.");
}



}
