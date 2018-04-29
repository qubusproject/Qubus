#ifndef QUBUS_MODULE_LIBRARY_HPP
#define QUBUS_MODULE_LIBRARY_HPP

#include <qubus/IR/module.hpp>
#include <qubus/IR/symbol_id.hpp>
#include <qubus/exception.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>

#include <stdexcept>
#include <unordered_map>

namespace qubus
{

class duplicate_module_error : public virtual exception, public virtual std::runtime_error
{
public:
    explicit duplicate_module_error(symbol_id module_id_);

    const symbol_id& module_id() const
    {
        return module_id_;
    }

private:
    symbol_id module_id_;
};

class missing_module_error : public virtual exception, public virtual std::runtime_error
{
public:
    explicit missing_module_error(symbol_id module_id_);

    const symbol_id& module_id() const
    {
        return module_id_;
    }

private:
    symbol_id module_id_;
};

class module_library_server : public hpx::components::component_base<module_library_server>
{
public:
    void add(std::unique_ptr<module> m);

    std::unique_ptr<module> lookup(const symbol_id& id) const;

    HPX_DEFINE_COMPONENT_ACTION(module_library_server, add, add_action);
    HPX_DEFINE_COMPONENT_ACTION(module_library_server, lookup, lookup_action);

private:
    std::unordered_map<symbol_id, std::unique_ptr<module>> module_index_;
};

class module_library : public hpx::components::client_base<module_library, module_library_server>
{
public:
    using base_type = hpx::components::client_base<module_library, module_library_server>;

    module_library() = default;
    module_library(hpx::id_type&& id_);
    module_library(hpx::future<hpx::id_type>&& id_);

    [[nodiscard]] hpx::future<void> add(std::unique_ptr<module> m) const;

    [[nodiscard]] hpx::future<std::unique_ptr<module>> lookup(const symbol_id& id) const;
};
}

#endif
