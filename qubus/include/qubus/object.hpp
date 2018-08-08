#ifndef QUBUS_OBJECT_HPP
#define QUBUS_OBJECT_HPP

#include <qubus/local_address_space.hpp>
#include <qubus/object_instance.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>

#include <qubus/IR/type.hpp>

#include <qubus/util/handle.hpp>
#include <qubus/util/hash.hpp>

#include <cstdint>

namespace qubus
{

class object_id
{
public:
    object_id(std::uint64_t id_msb_, std::uint64_t id_lsb_) : id_msb_(id_msb_), id_lsb_(id_lsb_)
    {
    }

    template <typename T>
    object_id(T id_msb_, T id_lsb_) = delete;

    explicit object_id(const hpx::naming::gid_type& gid) : object_id(gid.get_msb(), gid.get_lsb())
    {
    }

    explicit object_id(const hpx::id_type& id) : object_id(id.get_gid())
    {
    }

    std::uint64_t id_msb() const
    {
        return id_msb_;
    }

    std::uint64_t id_lsb() const
    {
        return id_lsb_;
    }

private:
    std::uint64_t id_msb_;
    std::uint64_t id_lsb_;
};

inline bool operator==(const object_id& lhs, const object_id& rhs)
{
    return lhs.id_msb() == rhs.id_msb() && lhs.id_lsb() == rhs.id_lsb();
}

inline bool operator!=(const object_id& lhs, const object_id& rhs)
{
    return !(lhs == rhs);
}

class object;

class object_server : public hpx::components::component_base<object_server>
{
public:
    object_server() = default;
    explicit object_server(type object_type_, std::size_t size_, local_address_space::handle data_);
    void finalize();

    object_server(const object_server&) = delete;
    object_server& operator=(const object_server&) = delete;

    type object_type() const;
    std::size_t size() const;
    object_id id() const;

    object_instance primary_instance() const;

    bool has_data() const;

    std::vector<object> components() const;

    HPX_DEFINE_COMPONENT_ACTION(object_server, finalize, finalize_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, object_type, object_type_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, size, size_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, primary_instance, primary_instance_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, has_data, has_data_action);
    HPX_DEFINE_COMPONENT_ACTION(object_server, components, components_action);

    const object& operator()(long int index) const;

private:
    type object_type_;
    std::size_t size_;
    local_address_space::handle data_;
    std::vector<object> components_;
};

class object : public hpx::components::client_base<object, object_server>
{
public:
    using base_type = hpx::components::client_base<object, object_server>;

    object() = default;
    object(hpx::id_type id);
    object(hpx::future<hpx::id_type>&& id);

    void finalize();

    type object_type() const;
    std::size_t size() const;

    object_id id() const;

    object_instance primary_instance() const;

    bool has_data() const;

    std::vector<object> components() const;

    friend bool operator==(const object& lhs, const object& rhs)
    {
        return lhs.id() == rhs.id();
    }

    friend bool operator!=(const object& lhs, const object& rhs)
    {
        return !(lhs == rhs);
    }
};
} // namespace qubus

namespace std
{
template <>
struct hash<qubus::object_id>
{
    using argument_type = qubus::object_id;
    using result_type = std::size_t;

    result_type operator()(const argument_type& s) const noexcept
    {
        std::size_t result = 0;

        qubus::util::hash_combine(result, s.id_lsb());
        qubus::util::hash_combine(result, s.id_msb());

        return result;
    }
};
} // namespace std

#endif