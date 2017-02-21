#ifndef QUBUS_OBJECT_INSTANCE_HPP
#define QUBUS_OBJECT_INSTANCE_HPP

#include <qubus/local_address_space.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/runtime/serialization/serialize.hpp>

#include <qubus/util/span.hpp>

#include <memory>

namespace qubus
{

class object_instance
{
public:
    object_instance();
    object_instance(local_address_space::handle local_handle_);

    ~object_instance();

    object_instance(object_instance&& other) noexcept;
    object_instance& operator=(object_instance&& other) noexcept;

    explicit operator bool() const;

    hpx::future<void> copy(util::span<char> buffer) const;

    void load(hpx::serialization::input_archive & ar, unsigned version);
    void save(hpx::serialization::output_archive & ar, unsigned version) const;

    HPX_SERIALIZATION_SPLIT_MEMBER();
private:
    class impl;

    std::unique_ptr<impl> impl_;
};

}

#endif
