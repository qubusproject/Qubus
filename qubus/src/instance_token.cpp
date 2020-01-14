#include <qubus/instance_token.hpp>

#include <qubus/host_memory.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/unreachable.hpp>
#include <qubus/util/unused.hpp>

#include <cstring>
#include <utility>

HPX_REGISTER_COMPONENT(hpx::components::component<qubus::instance_token_server>,
                       qubus_instance_token_server);

using acquire_instance_action = qubus::instance_token_server::acquire_instance_action;
HPX_REGISTER_ACTION(acquire_instance_action, qubus_instance_token_server_acquire_instance_action);

namespace qubus
{

namespace
{

class instance_unwrapper
{
public:
    using result_type = object_instance<host_allocator>;

    explicit instance_unwrapper(host_allocator& allocator_, std::size_t requested_alignment_)
    : allocator_(&allocator_), requested_alignment_(requested_alignment_)
    {
    }

    result_type operator()(object_instance<host_allocator> instance) const
    {
        QUBUS_ASSERT(allocator_, "Invalid object.");

        return instance;
    }

    result_type operator()(instance_handle<host_allocator> instance) const
    {
        QUBUS_ASSERT(allocator_, "Invalid object.");

        auto object_type = instance.get_instance().object_type();

        const auto& source_data = instance.get_instance().data();

        auto size = source_data.size();

        auto data = allocator_->allocate(size, requested_alignment_);

        std::memcpy(data.ptr(), source_data.ptr(), size);

        return object_instance<host_allocator>(std::move(object_type), std::move(data));
    }

    result_type operator()(std::monostate) const
    {
        QUBUS_UNREACHABLE();
    }

private:
    host_allocator* allocator_;
    std::size_t requested_alignment_;
};

struct instance_normalizer
{
    using result_type = const object_instance<host_allocator>&;

    result_type operator()(const object_instance<host_allocator>& instance) const
    {
        return instance;
    }

    result_type operator()(const instance_handle<host_allocator>& instance) const
    {
        return instance.get_instance();
    }

    result_type operator()(std::monostate) const
    {
        QUBUS_UNREACHABLE();
    }
};

} // namespace

transferable_object_instance::transferable_object_instance(
    instance_handle<host_allocator> instance_, hpx::id_type source_locality_,
    std::uintptr_t allocator_addr_, std::size_t requested_alignment_)
: instance_(std::move(instance_)),
  source_locality_(std::move(source_locality_)),
  allocator_addr_(allocator_addr_),
  requested_alignment_(requested_alignment_)
{
}

object_instance<host_allocator> transferable_object_instance::unwrap()
{
    QUBUS_ASSERT(hpx::find_here() == source_locality_,
                 "Unwrapping is only possible on the source locality.");

    auto alloc = util::bit_cast<host_allocator*>(allocator_addr_);

    return std::visit(instance_unwrapper(*alloc, requested_alignment_), std::move(instance_));
}

void transferable_object_instance::load(hpx::serialization::input_archive& ar,
                                        unsigned int QUBUS_UNUSED(version))
{
    type object_type;
    ar >> object_type;

    std::size_t size = 0;
    ar >> size;

    ar >> allocator_addr_;

    ar >> source_locality_;

    QUBUS_ASSERT(hpx::find_here() == source_locality_,
                 "Deserialization is only possible on the source locality.");

    auto alloc = util::bit_cast<host_allocator*>(allocator_addr_);

    auto data = alloc->allocate(size, requested_alignment_);

    ar >> hpx::serialization::make_array(data.ptr(), data.size());

    instance_ = object_instance<host_allocator>(std::move(object_type), std::move(data));
}

void transferable_object_instance::save(hpx::serialization::output_archive& ar,
                                        unsigned int QUBUS_UNUSED(version)) const
{
    const object_instance<host_allocator>& inst = std::visit(instance_normalizer(), instance_);

    ar << inst.object_type();

    ar << allocator_addr_;

    ar << source_locality_;

    ar << hpx::serialization::make_array(inst.data().ptr(), inst.data().size());
}

instance_token_server::instance_token_server(instance_handle<host_allocator> handle_)
: handle_(std::move(handle_))
{
}

transferable_object_instance
instance_token_server::acquire_instance(hpx::id_type source_locality, std::uintptr_t allocator_addr,
                                        std::size_t requested_alignment) const
{
    return transferable_object_instance(handle_, std::move(source_locality), allocator_addr,
                                        requested_alignment);
}

instance_token::instance_token(hpx::id_type&& id) : base_type(std::move(id))
{
}

instance_token::instance_token(hpx::future<hpx::id_type>&& id) : base_type(std::move(id))
{
}

} // namespace qubus
