#ifndef QUBUS_BASIC_ADDRESS_SPACE_HPP
#define QUBUS_BASIC_ADDRESS_SPACE_HPP

#include <qubus/virtual_address_space.hpp>

#include <memory>
#include <vector>

namespace qubus
{

class basic_address_space : public virtual_address_space
{
public:
    ~basic_address_space() override = default;

    void add_subspace(std::unique_ptr<virtual_address_space> subspace);

    hpx::future<object_instance> resolve_object(const object& obj) override;
    object_instance try_resolve_object(const object& obj) const override;
private:
    std::vector<std::unique_ptr<virtual_address_space>> subspaces_;
};

}

#endif
