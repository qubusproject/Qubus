#ifndef QBB_QUBUS_USER_DEFINED_PLAN_HPP
#define QBB_QUBUS_USER_DEFINED_PLAN_HPP
#include <hpx/config.hpp>

#include <qbb/qubus/plan.hpp>
#include <qbb/qubus/execution_context.hpp>

#include <qbb/qubus/metadata_builder.hpp>

#include <qbb/util/function_traits.hpp>
#include <qbb/util/integer_sequence.hpp>
#include <qbb/util/integers.hpp>
#include <qbb/util/assert.hpp>

#include <hpx/include/lcos.hpp>

#include <vector>
#include <array>
#include <functional>
#include <utility>
#include <type_traits>

namespace qbb
{
namespace qubus
{

using user_defined_plan_body_t = std::function<void(void* const*)>;

class user_defined_plan_t
{
public:
    std::vector<intent> intents;
    user_defined_plan_body_t body;
};

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

class user_defined_plan_builder
{
public:
    template <typename F>
    user_defined_plan_builder& body(F f)
    {
        constexpr auto arity = util::function_traits<F>::arity;

        intents_ = determine_intents<F>(util::make_index_sequence<arity>());

        body_ = create_thunk(std::move(f), util::make_index_sequence<arity>());

        return *this;
    }

    plan finalize();

private:
    template <typename F, std::size_t... Indices>
    static std::vector<intent> determine_intents(util::index_sequence<Indices...>)
    {
        return {
            std::is_const<
                typename util::function_traits<F>::template arg<Indices>::type::value_type>::value
                ? intent::in
                : intent::inout...};
    }

    template <typename F, std::size_t... Indices>
    static user_defined_plan_body_t create_thunk(F f, util::index_sequence<Indices...>)
    {
        auto thunk = [f](void* const* args)
        {
            f(util::function_traits<F>::template arg<Indices>::type::construct(args[Indices])...);
        };

        return thunk;
    }

    std::vector<intent> intents_;
    user_defined_plan_body_t body_;
};

inline user_defined_plan_builder make_computelet()
{
    return user_defined_plan_builder();
}

[[deprecated("Use make_computelet instead.")]] inline user_defined_plan_builder make_plan()
{
    return make_computelet();
}
}
}

#endif