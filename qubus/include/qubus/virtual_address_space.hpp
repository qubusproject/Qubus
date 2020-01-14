#ifndef QUBUS_VIRTUAL_ADDRESS_SPACE_HPP
#define QUBUS_VIRTUAL_ADDRESS_SPACE_HPP

#include <qubus/object_id.hpp>
#include <qubus/instance_token.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <memory>

namespace qubus
{

class virtual_address_space
{
public:
    virtual ~virtual_address_space() = default;


    virtual hpx::future<instance_token> resolve_object(object_id id) = 0;

    /** Invalidate all instances of the specified object in this scope
     *
     *  Any sub-scope may use the provided source of thruth to update its instance.
     *
     *  Note: The source of truth might not be the primary instance. It could also refer to
     *        an already updated copy.
     *
     * @param id ID of the object
     * @return a future signifying the pending operation
     */
    virtual hpx::future<void> invalidate_object(object_id id) = 0;

    /** Free all instances of the specified object in this scope
     *
     * @param id ID of the object
     * @return a future signifying the pending operation
     */
    virtual hpx::future<void> free_object(object_id id) = 0;

};

class virtual_address_space_wrapper
: public hpx::components::component_base<virtual_address_space_wrapper>
{
public:
    class client : public virtual_address_space,
                   public hpx::components::client_base<client, virtual_address_space_wrapper>
    {
    public:
        using base_type = hpx::components::client_base<client, virtual_address_space_wrapper>;

        client() = default;

        client(hpx::id_type&& id);
        client(hpx::future<hpx::id_type>&& id);

        ~client() override = default;

        hpx::future<instance_token> resolve_object(object_id id) override;

        hpx::future<void> invalidate_object(object_id id) override;
        hpx::future<void> free_object(object_id id) override;
    };

    virtual_address_space_wrapper() = default;
    explicit virtual_address_space_wrapper(
        std::unique_ptr<virtual_address_space> wrapped_address_space_);

    hpx::future<instance_token> resolve_object(object_id id);

    hpx::future<void> invalidate_object(object_id id);
    hpx::future<void> free_object(object_id id);

    HPX_DEFINE_COMPONENT_ACTION(virtual_address_space_wrapper, resolve_object,
                                resolve_object_action);
    HPX_DEFINE_COMPONENT_ACTION(virtual_address_space_wrapper, invalidate_object,
                                invalidate_object_action);
    HPX_DEFINE_COMPONENT_ACTION(virtual_address_space_wrapper, free_object,
                                free_object_action);

private:
    std::unique_ptr<virtual_address_space> wrapped_address_space_;
};

} // namespace qubus

#endif
