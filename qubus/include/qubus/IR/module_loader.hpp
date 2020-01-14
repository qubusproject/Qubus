#ifndef QUBUS_IR_MODULE_LOADER_HPP
#define QUBUS_IR_MODULE_LOADER_HPP

#include <qubus/IR/module.hpp>
#include <qubus/IR/assembly.hpp>

#include <qubus/util/exception.hpp>

#include <memory>

namespace qubus
{

class module_resolver
{
public:
   virtual ~module_resolver() noexcept = default;

   virtual const module& lookup_module(const symbol_id& id) const = 0;
};

class module_loader
{
public:
    virtual ~module_loader() noexcept = default;

    [[nodiscard]] virtual assembly load(const module& interface_module, const module_resolver& mod_resolver) const = 0;
};

std::unique_ptr<module_loader> create_module_loader();

}

#endif
