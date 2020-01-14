#include <qubus/local_address_space.hpp>

namespace qubus
{

local_address_space::page_fault_context::page_fault_context(
    host_address_space::page_fault_context host_ctx_)
: host_ctx_(std::move(host_ctx_))
{
}

/*local_address_space::handle
local_address_space::page_fault_context::allocate_page(long int size, long int alignment)
{
    return host_ctx_.allocate_page(size, alignment);
}*/

local_address_space::local_address_space(host_address_space& host_addr_space_)
: host_addr_space_(host_addr_space_),
  on_page_fault_(
      [](object_id QUBUS_UNUSED(id), page_fault_context) { // TODO: Add correct exception type.
          return hpx::make_exceptional_future<local_address_space::object_instance_type>(
              std::runtime_error("Page fault"));
      })
{
    host_addr_space_.on_page_fault([this](object_id id, host_address_space::page_fault_context ctx)
                                       -> hpx::future<host_address_space::object_instance_type> {
        return on_page_fault_(std::move(id), page_fault_context(std::move(ctx)));
    });
}

/*local_address_space::handle local_address_space::allocate_page(long int size, long int alignment)
{
    return host_addr_space_.get().allocate_page(size, alignment);
}*/

void local_address_space::register_page(object_id id, local_address_space::handle page)
{
    host_addr_space_.get().register_page(id, std::move(page));
}

void local_address_space::free_object(object_id id)
{
    host_addr_space_.get().free_object(id);
}

hpx::shared_future<local_address_space::handle> local_address_space::resolve_object(object_id id)
{
    return host_addr_space_.get().resolve_object(id);
}

local_address_space::handle local_address_space::try_resolve_object(object_id id) const
{
    return host_addr_space_.get().try_resolve_object(id);
}

void local_address_space::on_page_fault(
    std::function<hpx::future<local_address_space::object_instance_type>(object_id, page_fault_context)> callback)
{
    on_page_fault_.connect(std::move(callback));
}

host_address_space& local_address_space::host_addr_space()
{
    return host_addr_space_.get();
}
} // namespace qubus