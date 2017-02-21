#include <qubus/basic_address_space.hpp>

#include <utility>

namespace qubus
{

void basic_address_space::add_subspace(std::unique_ptr<virtual_address_space> subspace)
{
    subspaces_.push_back(std::move(subspace));
}

hpx::future<object_instance> basic_address_space::resolve_object(const object& obj)
{
    return hpx::make_ready_future(try_resolve_object(obj));
}

object_instance basic_address_space::try_resolve_object(const object& obj) const
{
    return obj.primary_instance();
}

}