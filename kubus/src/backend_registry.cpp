#include <qbb/kubus/backend_registry.hpp>

#include <utility>

namespace qbb
{
namespace qubus
{

void backend_registry::register_backend(std::string id, std::unique_ptr<backend> backend)
{
    auto& entry = backend_dictionary_[id];

    if (!entry)
    {
        entry = std::move(backend);
    }
    else
    {
        throw 0;
    }
}

backend* backend_registry::lookup_backend(std::string id) const
{
    auto iter = backend_dictionary_.find(id);

    if (iter != backend_dictionary_.end())
    {
        return iter->second.get();
    }
    else
    {
        throw 0;
    }
}

backend_registry::iterator backend_registry::begin()
{
    return backend_dictionary_.begin();
}

backend_registry::iterator backend_registry::end()
{
    return backend_dictionary_.end();
}

backend_registry::const_iterator backend_registry::begin() const
{
    return backend_dictionary_.begin();
}

backend_registry::const_iterator backend_registry::end() const
{
    return backend_dictionary_.end();
}
}
}