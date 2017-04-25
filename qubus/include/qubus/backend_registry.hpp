#ifndef QUBUS_BACKEND_REGISTRY_HPP
#define QUBUS_BACKEND_REGISTRY_HPP

#include <qubus/backend.hpp>
#include <qubus/host_backend.hpp>

#include <boost/dll.hpp>
#include <boost/optional.hpp>

#include <qubus/util/optional_ref.hpp>

#include <exception>
#include <string>
#include <vector>

namespace qubus
{

class host_backend_already_set_exception : public virtual std::exception
{
public:
    const char* what() const noexcept override
    {
        return "Host backend has already been set.";
    }
};

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

    class host_backend_entry
    {
    public:
        host_backend_entry(std::string id_, host_backend& backend_, boost::dll::shared_library library_);

        const std::string& id() const;
        host_backend& get_backend() const;

    private:
        std::string id_;
        host_backend* backend_;
        boost::dll::shared_library library_;
    };

    using iterator = std::vector<entry>::iterator;
    using const_iterator = std::vector<entry>::const_iterator;

    void register_host_backend(std::string id, host_backend& backend,
                               boost::dll::shared_library library);
    void register_backend(std::string id, backend& backend, boost::dll::shared_library library);

    util::optional_ref<host_backend> get_host_backend() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

private:
    boost::optional<host_backend_entry> host_backend_;
    std::vector<entry> backends_;
};
}

#endif