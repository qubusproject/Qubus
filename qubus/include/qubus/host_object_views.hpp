#ifndef QUBUS_HOST_OBJECTS_VIEWS_HPP_HPP
#define QUBUS_HOST_OBJECTS_VIEWS_HPP_HPP

#include <qubus/object.hpp>

#include <qubus/dataflow.hpp>
#include <qubus/local_address_space.hpp>

#include <qubus/IR/type.hpp>
#include <qubus/associated_qubus_type.hpp>
#include <qubus/object_view_traits.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/parallel/executors.hpp> // Workaround for missing includes.
#include <hpx/runtime/threads/executors/pool_executor.hpp>

#include <qubus/util/assert.hpp>
#include <qubus/util/integers.hpp>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace qubus
{

class host_view_context
{
public:
    explicit host_view_context(distributed_access_token access_token_,
                               local_address_space::handle associated_handle_)
    : access_token_(std::move(access_token_)), associated_handle_(std::move(associated_handle_))
    {
    }

    host_view_context(const host_view_context&) = delete;
    host_view_context& operator=(const host_view_context&) = delete;

    host_view_context(host_view_context&&) = delete;
    host_view_context& operator=(host_view_context&&) = delete;

private:
    distributed_access_token access_token_;
    local_address_space::handle associated_handle_;
};

template <typename T>
class cpu_scalar_view
{
public:
    using value_type = T;

    const T& get() const
    {
        return *value_;
    }

    T& get()
    {
        return *value_;
    }

    template <typename AddressSpace>
    [[nodiscard]] static hpx::future<cpu_scalar_view<T>>
    construct(object obj, hpx::future<distributed_access_token> access_token,
              AddressSpace& addr_space)
    {
        auto hnd = addr_space.resolve_object(obj).get();

        auto value = static_cast<T*>(hnd.data().ptr());

        auto ctx = std::make_shared<host_view_context>(access_token.get(), std::move(hnd));

        return hpx::make_ready_future(cpu_scalar_view<T>(value, std::move(ctx)));
    }

    static cpu_scalar_view<T> construct_from_reference(void* ref)
    {
        auto value = static_cast<T*>(ref);

        auto ctx = std::shared_ptr<host_view_context>();

        return cpu_scalar_view<T>(value, std::move(ctx));
    }

    template <typename AddressSpace>
    [[nodiscard]] static hpx::future<cpu_scalar_view<T>>
    construct_from_locked_object(object obj, AddressSpace& addr_space)
    {
        auto hnd = addr_space.resolve_object(obj).get();

        auto value = static_cast<T*>(hnd.data().ptr());

        auto ctx = std::make_shared<host_view_context>(distributed_access_token(), std::move(hnd));

        return hpx::make_ready_future(cpu_scalar_view<T>(value, std::move(ctx)));
    }

private:
    cpu_scalar_view(T* value_, std::shared_ptr<host_view_context> ctx_)
    : value_(value_), ctx_(std::move(ctx_))
    {
    }

    T* value_;

    std::shared_ptr<host_view_context> ctx_;
};

template <typename T>
struct object_view_traits<cpu_scalar_view<T>>
{
    static constexpr bool is_immutable = std::is_const<T>::value;

    static type associated_type()
    {
        return associated_qubus_type<T>::get();
    }
};

template <typename T>
using host_scalar_view = cpu_scalar_view<T>;

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

        QUBUS_ASSERT(linear_index >= 0, "Linear index can't be negative.");

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

        QUBUS_ASSERT(linear_index >= 0, "Linear index can't be negative.");

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

    template <typename AddressSpace>
    [[nodiscard]] static hpx::future<cpu_array_view<T, Rank>>
    construct(object obj, hpx::future<distributed_access_token> access_token,
              AddressSpace& addr_space)
    {
        auto hnd = addr_space.resolve_object(obj).get();

        auto base_ptr = hnd.data().ptr();

        auto shape_ptr = static_cast<util::index_t*>(base_ptr) + 1;

        auto data_ptr = static_cast<T*>(static_cast<void*>(shape_ptr + Rank));

        auto service_executor = std::make_shared<hpx::threads::executors::pool_executor>("/qubus/service");

        return access_token.then(
            *service_executor,
            [hnd, shape_ptr, data_ptr, service_executor](hpx::future<distributed_access_token> token) mutable {
                auto ctx = std::make_shared<host_view_context>(token.get(), std::move(hnd));

                return cpu_array_view<T, Rank>(Rank, shape_ptr, data_ptr, std::move(ctx));
            });
    }

    static cpu_array_view<T, Rank> construct_from_reference(void* ref)
    {
        auto base_ptr = ref;

        auto shape_ptr = static_cast<util::index_t*>(base_ptr) + 1;

        auto data_ptr = static_cast<T*>(static_cast<void*>(shape_ptr + Rank));

        auto ctx = std::shared_ptr<host_view_context>();

        return cpu_array_view<T, Rank>(Rank, shape_ptr, data_ptr, std::move(ctx));
    }

    template <typename AddressSpace>
    [[nodiscard]] static hpx::future<cpu_array_view<T, Rank>>
    construct_from_locked_object(object obj, AddressSpace& addr_space)
    {
        auto hnd = addr_space.resolve_object(obj).get();

        auto base_ptr = hnd.data().ptr();

        auto shape_ptr = static_cast<util::index_t*>(base_ptr) + 1;

        auto data_ptr = static_cast<T*>(static_cast<void*>(shape_ptr + Rank));

        auto ctx = std::make_shared<host_view_context>(distributed_access_token(), std::move(hnd));

        return hpx::make_ready_future(
            cpu_array_view<T, Rank>(Rank, shape_ptr, data_ptr, std::move(ctx)));
    }

private:
    cpu_array_view(util::index_t rank_, util::index_t* shape_, T* data_,
                   std::shared_ptr<host_view_context> ctx_)
    : rank_(rank_), shape_(shape_), data_(data_), ctx_(std::move(ctx_))
    {
    }

    util::index_t rank_;
    util::index_t* shape_;
    T* data_;

    std::shared_ptr<host_view_context> ctx_;
};

template <typename T, util::index_t Rank>
struct object_view_traits<cpu_array_view<T, Rank>>
{
    static constexpr bool is_immutable = std::is_const<T>::value;

    static type associated_type()
    {
        return types::array(associated_qubus_type<T>::get());
    }
};

template <typename T, util::index_t Rank>
using host_array_view = cpu_array_view<T, Rank>;

template <typename T, util::index_t Rank>
using cpu_tensor_view = host_array_view<T, Rank>;

template <typename T, util::index_t Rank>
using host_tensor_view = cpu_tensor_view<T, Rank>;

template <typename T, long int Rank>
class mutable_cpu_sparse_tensor_view
{
public:
    using value_type = T;

    template <typename AddressSpace>
    [[nodiscard]] static hpx::future<mutable_cpu_sparse_tensor_view<T, Rank>>
    construct(object obj, hpx::future<distributed_access_token> access_token,
              AddressSpace& addr_space)
    {
        /*auto ctx =
            std::make_shared<host_view_context>(access_token.get(), local_address_space::handle());

        auto tensor_components = obj.components();

        auto data = tensor_components.at(0);

        auto data_components = data.components();

        auto values = cpu_array_view<T, 1>::construct(data_components.at(0)).get();
        auto col = cpu_array_view<util::index_t, 1>::construct(data_components.at(1)).get();
        auto cs = cpu_array_view<util::index_t, 1>::construct(data_components.at(2)).get();
        auto cl = cpu_array_view<util::index_t, 1>::construct(data_components.at(3)).get();

        auto shape = cpu_array_view<util::index_t, 1>::construct(tensor_components.at(1)).get();

        return hpx::make_ready_future(
            mutable_cpu_sparse_tensor_view<T, Rank>(col, cl, cs, values, shape, std::move(ctx)));*/

        std::terminate();
    }

    void dump()
    {
        // FIXME: This does not work for arbitrary tensors.
        std::cout << "sparse tensor\n" << std::endl;

        std::cout << col_.extent(0) << std::endl;
        std::cout << cl_.extent(0) << std::endl;
        std::cout << cs_.extent(0) << std::endl;
        std::cout << values_.extent(0) << std::endl;

        std::cout << std::endl;

        for (util::index_t i = 0; i < cl_.extent(0); ++i)
        {
            std::cout << cl_(i) << "  ";
        }
        std::cout << std::endl;

        for (util::index_t i = 0; i < cs_.extent(0); ++i)
        {
            std::cout << cs_(i) << "  ";
        }
        std::cout << std::endl;

        std::cout << std::endl;

        util::index_t N = 10;

        for (util::index_t i = 0; i < (N / 4) * 4; i += 4)
        {
            for (util::index_t k = 0; k < 4; ++k)
            {
                for (util::index_t j = 0; j < cl()(i / 4); ++j)
                {
                    std::cout << values()(cs()(i / 4) + 4 * j + k) << " ";
                }
                std::cout << std::endl;
            }

            std::cout << "-----------" << std::endl;
        }

        for (util::index_t i = (N / 4) * 4; i < N; i += 4)
        {
            for (util::index_t k = 0; k < N % 4; ++k)
            {
                for (util::index_t j = 0; j < cl()(i / 4); ++j)
                {
                    std::cout << values()(cs()(i / 4) + 4 * j + k) << " ";
                }
                std::cout << std::endl;
            }

            std::cout << "-----------" << std::endl;
        }
    }

    cpu_array_view<util::index_t, 1> col()
    {
        return col_;
    }

    cpu_array_view<util::index_t, 1> cl()
    {
        return cl_;
    }

    cpu_array_view<util::index_t, 1> cs()
    {
        return cs_;
    }

    cpu_array_view<T, 1> values()
    {
        return values_;
    }

    cpu_array_view<util::index_t, 1> shape()
    {
        return shape_;
    };

private:
    mutable_cpu_sparse_tensor_view(cpu_array_view<util::index_t, 1> col_,
                                   cpu_array_view<util::index_t, 1> cl_,
                                   cpu_array_view<util::index_t, 1> cs_,
                                   cpu_array_view<T, 1> values_,
                                   cpu_array_view<util::index_t, 1> shape_,
                                   std::shared_ptr<host_view_context> ctx_)
    : col_(std::move(col_)),
      cl_(std::move(cl_)),
      cs_(std::move(cs_)),
      values_(std::move(values_)),
      shape_(std::move(shape_)),
      ctx_(std::move(ctx_))
    {
    }

    cpu_array_view<util::index_t, 1> col_;
    cpu_array_view<util::index_t, 1> cl_;
    cpu_array_view<util::index_t, 1> cs_;
    cpu_array_view<T, 1> values_;
    cpu_array_view<util::index_t, 1> shape_;

    std::shared_ptr<host_view_context> ctx_;
};

template <typename T, util::index_t Rank>
using mutable_host_sparse_tensor_view = mutable_cpu_sparse_tensor_view<T, Rank>;

template <typename T, util::index_t Rank>
struct object_view_traits<mutable_cpu_sparse_tensor_view<T, Rank>>
{
    static constexpr bool is_immutable = false;

    static type associated_type()
    {
        return types::sparse_tensor(associated_qubus_type<T>::get());
    }
};

template <typename View, typename AddressSpace>
hpx::future<View> get_view_for_locked_object(object obj, AddressSpace& addr_space)
{
    return View::construct_from_locked_object(std::move(obj), addr_space);
}
} // namespace qubus

#endif
