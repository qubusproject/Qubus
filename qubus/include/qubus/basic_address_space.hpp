#ifndef QUBUS_BASIC_ADDRESS_SPACE_HPP
#define QUBUS_BASIC_ADDRESS_SPACE_HPP

#include <qubus/virtual_address_space.hpp>

#include <memory>
#include <vector>

namespace qubus
{

class basic_address_space final : public virtual_address_space
{
public:
    ~basic_address_space() override = default;

    void add_subspace(std::unique_ptr<virtual_address_space> subspace);

    hpx::future<instance_token> resolve_object(object_id id) override;

    hpx::future<void> invalidate_object(object_id id) override;
    hpx::future<void> free_object(object_id id) override;
private:
    std::vector<std::unique_ptr<virtual_address_space>> subspaces_;
};

}

#endif
