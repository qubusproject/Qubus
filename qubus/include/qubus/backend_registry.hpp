#ifndef QUBUS_BACKEND_REGISTRY_HPP
#define QUBUS_BACKEND_REGISTRY_HPP

#include <qubus/backend.hpp>

#include <boost/dll.hpp>

#include <string>
#include <vector>

namespace qubus
{

class backend_registry
{
public:
    class entry
    {
    public:
        entry(std::string id_, backend& backend_, boost::dll::shared_library library_);

        const std::string& id() const;
        backend& get_backend() const;
    private:
        std::string id_;
        backend* backend_;
        boost::dll::shared_library library_;
    };

    using iterator = std::vector<entry>::iterator;
    using const_iterator = std::vector<entry>::const_iterator;

    void register_backend(std::string id, backend& backend, boost::dll::shared_library library);

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

private:
    std::vector<entry> backends_;
};
}

#endif