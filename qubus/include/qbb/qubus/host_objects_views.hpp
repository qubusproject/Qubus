#ifndef QBB_QUBUS_HOST_OBJECTS_VIEWS_HPP_HPP
#define QBB_QUBUS_HOST_OBJECTS_VIEWS_HPP_HPP

#include <qbb/qubus/object_view_traits.hpp>
#include <qbb/qubus/associated_qubus_type.hpp>
#include <qbb/qubus/IR/type.hpp>

#include <qbb/util/integers.hpp>
#include <qbb/util/assert.hpp>

namespace qbb
{
namespace qubus
{

struct array_metadata
{
    void* data;
    void* shape;
};

template <typename T, util::index_t Rank>
class cpu_array_view
{
public:
    using value_type = T;

    template <typename... Indices>
    T& operator()(Indices... indices)
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{{util::to_uindex(indices)...}};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    template <typename... Indices>
    const T& operator()(Indices... indices) const
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{util::to_uindex(indices)...};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    util::index_t rank() const
    {
        return rank_;
    }

    util::index_t extent(util::index_t dim) const
    {
        return shape_[dim];
    }

    // TODO: implement shape

    static cpu_array_view<T, Rank> construct(void* metadata)
    {
        auto array_md = static_cast<array_metadata*>(metadata);

        return cpu_array_view<T, Rank>(Rank, static_cast<util::index_t*>(array_md->shape),
                                       static_cast<T*>(array_md->data));
    }

private:
    cpu_array_view(util::index_t rank_, util::index_t* shape_, T* data_)
    : rank_(rank_), shape_(shape_), data_(data_)
    {
    }

    util::index_t rank_;
    util::index_t* shape_;
    T* data_;
};

template<typename T, util::index_t Rank>
struct object_view_traits<cpu_array_view<T, Rank>>
{
    static type associated_type()
    {
        return types::array(associated_qubus_type<T>::get());
    }
};

template<typename T, util::index_t Rank>
using host_array_view = cpu_array_view<T, Rank>;

template <typename T, util::index_t Rank>
class cpu_tensor_view
{
public:
    using value_type = T;

    template <typename... Indices>
    T& operator()(Indices... indices)
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{{util::to_uindex(indices)...}};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    template <typename... Indices>
    const T& operator()(Indices... indices) const
    {
        static_assert(Rank == sizeof...(indices), "insufficient number of indices");

        util::index_t linear_index = 0;

        std::array<util::index_t, Rank> indices_{util::to_uindex(indices)...};

        for (util::index_t i = 0; i < Rank; ++i)
        {
            linear_index = shape_[i] * linear_index + indices_[i];
        }

        QBB_ASSERT(linear_index >= 0, "Linear index can't be negative.");

        return data_[linear_index];
    }

    util::index_t rank() const
    {
        return rank_;
    }

    util::index_t extent(util::index_t dim) const
    {
        return shape_[dim];
    }

    // TODO: implement shape

    static cpu_tensor_view<T, Rank> construct(void* metadata)
    {
        auto array_md = static_cast<array_metadata*>(metadata);

        return cpu_tensor_view<T, Rank>(Rank, static_cast<util::index_t*>(array_md->shape),
                                        static_cast<T*>(array_md->data));
    }

private:
    cpu_tensor_view(util::index_t rank_, util::index_t* shape_, T* data_)
            : rank_(rank_), shape_(shape_), data_(data_)
    {
    }

    util::index_t rank_;
    util::index_t* shape_;
    T* data_;
};

template<typename T, util::index_t Rank>
struct object_view_traits<cpu_tensor_view<T, Rank>>
{
    static type associated_type()
    {
        return types::array(associated_qubus_type<T>::get());
    }
};

template <typename T, util::index_t Rank>
using host_tensor_view = cpu_tensor_view<T, Rank>;

}
}

#endif
