#include <qubus/backend_registry.hpp>

#include <algorithm>
#include <utility>

namespace qubus
{

backend_registry::entry::entry(std::string id_, backend& backend_, boost::dll::shared_library library_)
: id_(std::move(id_)), backend_(&backend_), library_(std::move(library_))
{
}

const std::string& backend_registry::entry::id() const
{
    return id_;
}

backend& backend_registry::entry::get_backend() const
{
    return *backend_;
}

void backend_registry::register_backend(std::string id, backend& backend, boost::dll::shared_library library)
{
    auto search_result = std::find_if(backends_.begin(), backends_.end(), [&id] (const backend_registry::entry& entry) { return entry.id() == id; });

    if (search_result != backends_.end())
        throw 0;

    backend_registry::entry entry(std::move(id), backend, std::move(library));

    backends_.push_back(std::move(entry));
}

backend_registry::iterator backend_registry::begin()
{
    return backends_.begin();
}

backend_registry::iterator backend_registry::end()
{
    return backends_.end();
}

backend_registry::const_iterator backend_registry::begin() const
{
    return backends_.begin();
}

backend_registry::const_iterator backend_registry::end() const
{
    return backends_.end();
}
}