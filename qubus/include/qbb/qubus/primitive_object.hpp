#ifndef QBB_QUBUS_PRIMITIVE_OBJECT_HPP
#define QBB_QUBUS_PRIMITIVE_OBJECT_HPP

#include <qbb/qubus/object.hpp>
#include <qbb/qubus/local_address_space.hpp>

#include <hpx/include/components.hpp>

namespace qbb
{
namespace qubus
{

class primitive_object : public basic_object, public hpx::components::component_base<primitive_object>
{
public:
    primitive_object() = default;
    primitive_object(type object_type_);
    virtual ~primitive_object() = default;

    type object_type() const override;
    hpx::id_type id() const override;

    void register_instance(local_address_space::handle data);
private:
    type object_type_;
    local_address_space::handle data_;
};

}
}

#endif
