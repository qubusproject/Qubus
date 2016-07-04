#include <qbb/qubus/object_materializer.hpp>

#include <qbb/util/assert.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

namespace
{
void materialize_object_recursively(const object& obj, std::vector<local_address_space::pin>& pins,
                                    local_address_space& addr_space)
{
    auto handle = addr_space.resolve_object(obj).get();

    auto pin = handle.pin_object().get();

    pins.push_back(std::move(pin));

    for (const object& component : obj.components())
    {
        materialize_object_recursively(component, pins, addr_space);
    }
}

void materialize_object_recursively(const object& obj, std::vector<address_space::pin>& pins,
                                    address_space& addr_space)
{
    auto handle = addr_space.resolve_object(obj).get();

    auto pin = handle.pin_object().get();

    pins.push_back(std::move(pin));

    for (const object& component : obj.components())
    {
        materialize_object_recursively(component, pins, addr_space);
    }
}
}

hpx::future<std::tuple<void*, std::vector<local_address_space::pin>>>
materialize_object(object obj, local_address_space& addr_space)
{
    return hpx::async([obj = std::move(obj), &addr_space ] {
        std::vector<local_address_space::pin> pins;

        materialize_object_recursively(obj, pins, addr_space);

        QBB_ASSERT(!pins.empty(), "Unexpected number of pins.");

        void* root_ptr = pins.front().data().ptr();

        return std::make_tuple(root_ptr, std::move(pins));
    });
}

hpx::future<std::vector<address_space::pin>>
        materialize_object(object obj, address_space& addr_space)
{
    return hpx::async([obj = std::move(obj), &addr_space ] {
        std::vector<address_space::pin> pins;

        materialize_object_recursively(obj, pins, addr_space);

        QBB_ASSERT(!pins.empty(), "Unexpected number of pins.");

        return pins;
    });
}

}
}