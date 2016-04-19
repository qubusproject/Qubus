#ifndef QBB_QUBUS_FOREIGN_COMPUTELET_HPP
#define QBB_QUBUS_FOREIGN_COMPUTELET_HPP

#include <hpx/config.hpp>

#include <qbb/qubus/architecture_identifier.hpp>

#include <qbb/qubus/IR/type.hpp>

#include <hpx/include/actions.hpp>
#include <hpx/runtime/serialization/serialize.hpp>
#include <hpx/runtime/serialization/base_object.hpp>

#include <qbb/util/unused.hpp>

#include <vector>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <algorithm>
#include <utility>

namespace qbb
{
namespace qubus
{

template <typename T, typename Enable = void>
struct foreign_computelet_traits;

class foreign_computelet
{
public:
    foreign_computelet() = default;
    foreign_computelet(type result_type_, std::vector<type> argument_types_);

    class version
    {
    public:
        version() = default;
        virtual ~version() = default;

        virtual const architecture_identifier& target() const = 0;

        virtual const void* get_function() const = 0;
        virtual const void* get_data() const = 0;

        template <typename Archive>
        void serialize(Archive& QBB_UNUSED(ar), unsigned QBB_UNUSED(version))
        {
        }

        HPX_SERIALIZATION_POLYMORPHIC_ABSTRACT(version);
    };

    template <typename T>
    foreign_computelet& add_version(architecture_identifier target, T version)
    {
        versions_.push_back(
            std::make_shared<version_wrapper<T>>(std::move(target), std::move(version)));

        return *this;
    }

    std::shared_ptr<version> lookup_version(const architecture_identifier& target) const
    {
        auto pos = std::find_if(versions_.begin(), versions_.end(), [&target](const auto& value)
                                {
                                    return value->target() == target;
                                });

        return *pos;
    }

    const type& result_type() const
    {
        return result_type_;
    }

    const std::vector<type>& argument_types() const
    {
        return argument_types_;
    }

    template <typename Archive>
    void serialize(Archive& ar, unsigned QBB_UNUSED(version))
    {
        ar& versions_;
    }

private:
    template <typename T>
    class version_wrapper : public version
    {
    public:
        version_wrapper() = default;

        explicit version_wrapper(architecture_identifier target_, T value_)
        : target_(std::move(target_)), value_(std::move(value_))
        {
        }

        const architecture_identifier& target() const override final
        {
            return target_;
        }

        const void* get_function() const override final
        {
            return foreign_computelet_traits<T>::thunk;
        }

        const void* get_data() const override final
        {
            return &value_;
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned QBB_UNUSED(version))
        {
            ar& hpx::serialization::base_object<foreign_computelet::version>(*this);

            ar& target_;
            ar& value_;
        }

        HPX_SERIALIZATION_POLYMORPHIC_TEMPLATE(version_wrapper);

    private:
        architecture_identifier target_;
        T value_;
    };

    type result_type_;
    std::vector<type> argument_types_;
    std::vector<std::shared_ptr<version>> versions_;
};

inline bool operator==(const foreign_computelet& QBB_UNUSED(lhs),
                       const foreign_computelet& QBB_UNUSED(rhs))
{
    return false;
}

inline bool operator!=(const foreign_computelet& lhs, const foreign_computelet& rhs)
{
    return !(lhs == rhs);
}
}
}

#endif