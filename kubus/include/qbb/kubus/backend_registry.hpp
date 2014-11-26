#ifndef QBB_KUBUS_BACKEND_REGISTRY_HPP
#define QBB_KUBUS_BACKEND_REGISTRY_HPP

#include <qbb/kubus/backend.hpp>

#include <string>
#include <map>
#include <memory>

namespace qbb
{
namespace kubus
{

class backend_registry
{
public:
    using iterator = std::map<std::string, std::unique_ptr<backend>>::iterator;
    using const_iterator = std::map<std::string, std::unique_ptr<backend>>::const_iterator;

    void register_backend(std::string id, std::unique_ptr<backend> backend);
    backend* lookup_backend(std::string id) const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

private:
    std::map<std::string, std::unique_ptr<backend>> backend_dictionary_;
};
}
}

#endif