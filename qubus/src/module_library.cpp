#include <qubus/module_library.hpp>

#include <utility>

using server_type = hpx::components::component<qubus::module_library_server>;
HPX_REGISTER_COMPONENT(server_type, qubus_module_library_server);

typedef qubus::module_library_server::add_action add_action;
HPX_REGISTER_ACTION(add_action, qubus_module_library_server_add_action);

typedef qubus::module_library_server::lookup_action lookup_action;
HPX_REGISTER_ACTION(lookup_action, qubus_module_library_server_lookup_action);

namespace qubus
{

duplicate_module_error::duplicate_module_error(symbol_id module_id_)
: std::runtime_error("The module " + module_id_.string() + " already exists."),
  module_id_(std::move(module_id_))
{
}

missing_module_error::missing_module_error(symbol_id module_id_)
: std::runtime_error("The module " + module_id_.string() + " is missing from the library."),
  module_id_(std::move(module_id_))
{
}

void module_library_server::add(std::unique_ptr<module> m)
{
    auto id = m->id();

    auto [pos, has_been_added] = module_index_.emplace(id, std::move(m));

    if (!has_been_added)
        throw duplicate_module_error(id);
}

std::unique_ptr<module> module_library_server::lookup(const symbol_id& id) const
{
    auto search_result = module_index_.find(id);

    if (search_result != module_index_.end())
    {
        return clone(*search_result->second);
    }
    else
    {
        throw missing_module_error(id);
    }
}

module_library::module_library(hpx::id_type&& id_) : base_type(std::move(id_))
{
}

module_library::module_library(hpx::future<hpx::id_type>&& id_) : base_type(std::move(id_))
{
}

hpx::future<void> module_library::add(std::unique_ptr<module> m) const
{
    return hpx::async<module_library_server::add_action>(this->get_id(), std::move(m));
}

hpx::future<std::unique_ptr<module>> module_library::lookup(const symbol_id& id) const
{
    return hpx::async<module_library_server::lookup_action>(this->get_id(), id);
}
}