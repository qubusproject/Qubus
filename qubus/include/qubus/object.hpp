#ifndef QUBUS_OBJECT_HPP
#define QUBUS_OBJECT_HPP

#include <qubus/object_id.hpp>
#include <qubus/IR/type.hpp>

#include <qubus/util/unused.hpp>

namespace qubus
{

class object
{
public:
    object() = default;

    explicit object(object_id id_, type object_type_)
    : id_(id_), object_type_(std::move(object_type_))
    {
    }

    object_id id() const;

    const type& object_type() const
    {
        return object_type_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar & id_;
        ar & object_type_;
    }
private:
    object_id id_;
    type object_type_;
};

inline bool operator==(const object& lhs, const object& rhs)
{
    return lhs.id() == rhs.id();
}

inline bool operator!=(const object& lhs, const object& rhs)
{
    return !(lhs == rhs);
}

} // namespace qubus

#endif