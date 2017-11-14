#ifndef QUBUS_OBJECT_DESCRIPTION_HPP
#define QUBUS_OBJECT_DESCRIPTION_HPP

#include <qubus/IR/type.hpp>
#include <qubus/abi_info.hpp>

#include <qubus/util/integers.hpp>
#include <qubus/util/unused.hpp>

#include <hpx/include/serialization.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace qubus
{

template <typename Derived>
class object_description_base
{
};

template <typename T>
struct is_object_description : std::is_base_of<object_description_base<T>, T>
{
};

class object_description
{
public:
    object_description() = default;

    template <typename T,
              typename Enabled = typename std::enable_if<is_object_description<T>::value>::type>
    object_description(T value_)
    : self_(std::make_shared<object_description_wrapper<T>>(std::move(value_)))
    {
    }

    template <typename T>
    const T* try_as() const
    {
        if (auto typed_self = dynamic_cast<const object_description_wrapper<T>*>(self_.get()))
            return &typed_self->value();

        return nullptr;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& self_;
    }

private:
    class object_description_interface
    {
    public:
        object_description_interface() = default;
        virtual ~object_description_interface() = default;

        object_description_interface(const object_description_interface&) = delete;
        object_description_interface& operator=(const object_description_interface&) = delete;

        template <typename Archive>
        void serialize(Archive& QUBUS_UNUSED(ar), unsigned QUBUS_UNUSED(version))
        {
        }

        HPX_SERIALIZATION_POLYMORPHIC(object_description_interface);
    };

    template <typename T>
    class object_description_wrapper : public object_description_interface
    {
    public:
        object_description_wrapper() = default;

        explicit object_description_wrapper(T value_) : value_(std::move(value_))
        {
        }

        const T& value() const
        {
            return value_;
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
        {
            ar& hpx::serialization::base_object<object_description_interface>(*this);

            ar& value_;
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(object_description_wrapper);

    private:
        T value_;
    };

    std::shared_ptr<object_description_interface> self_;
};

class array_description : public object_description_base<array_description>
{
public:
    array_description() = default;

    array_description(type value_type_, std::vector<util::index_t> shape_)
    : value_type_(std::move(value_type_)), shape_(std::move(shape_))
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    const std::vector<util::index_t>& shape() const
    {
        return shape_;
    }

    util::index_t rank() const
    {
        return util::to_uindex(shape_.size());
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& value_type_;
        ar& shape_;
    }

private:
    type value_type_;
    std::vector<util::index_t> shape_;
};

class scalar_description : public object_description_base<scalar_description>
{
public:
    scalar_description() = default;

    explicit scalar_description(type value_type_) : value_type_(std::move(value_type_))
    {
    }

    const type& value_type() const
    {
        return value_type_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& value_type_;
    }

private:
    type value_type_;
};

class struct_description : public object_description_base<struct_description>
{
public:
    struct member
    {
        member(std::string id, object_description description)
        : id(std::move(id)), description(std::move(description))
        {
        }

        std::string id;
        object_description description;

        template <typename Archive>
        void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
        {
            ar& id;
            ar& description;
        }
    };

    struct_description() = default;

    explicit struct_description(types::struct_ type_, std::vector<member> members_)
    : type_(std::move(type_)), members_(std::move(members_))
    {
    }

    const types::struct_& type() const
    {
        return type_;
    }

    const std::vector<member>& members() const
    {
        return members_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QUBUS_UNUSED(version))
    {
        ar& type_;
        ar& members_;
    }

private:
    types::struct_ type_;
    std::vector<member> members_;
};

type compute_type(const object_description& desc);

struct object_layout
{
    object_layout(object_description description, long int position, long int size)
    : description(std::move(description)), position(position), size(size)
    {
    }

    object_layout(object_description description, long int position, long int size,
                  std::vector<object_layout> partitions)
    : description(std::move(description)),
      position(position),
      size(size),
      partitions(std::move(partitions))
    {
    }

    object_description description;

    long int position;
    long int size;

    std::vector<object_layout> partitions;
};

object_layout compute_layout(const object_description& desc, const abi_info& abi);
}

#endif
