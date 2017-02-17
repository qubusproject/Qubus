#ifndef QUBUS_ACTION_SUPPORT_HPP
#define QUBUS_ACTION_SUPPORT_HPP

namespace qubus
{

template<typename Action, typename Indices>
struct action_thunk_mixin_impl;

template<std::size_t, typename T>
struct type_generator
{
    using type = T;
};

template<typename Action, std::size_t... Indices>
struct action_thunk_mixin_impl<Action, std::index_sequence<Indices...>>
{
private:
template<typename... Args>
static void thunk_body(Action *action, Args... args)
{
    (*action)(hpx::find_here(),
              hpx::util::tuple_element<Indices, typename Action::arguments_type>::type::construct(
                      args)...);
}

public:
static constexpr void *thunk = reinterpret_cast<void *>(
        static_cast<void (*)(Action *, typename type_generator<Indices, void *>::type...)>(
                &thunk_body));
};

template<typename Action>
struct action_thunk_mixin
        : action_thunk_mixin_impl<
                Action,
                std::make_index_sequence<hpx::util::tuple_size<typename Action::arguments_type>::value>>
{
};

template<typename Action>
struct foreign_computelet_traits<
        Action, typename std::enable_if<hpx::traits::is_action<Action>::value>::type>
        : action_thunk_mixin<Action>
{
};

}

#endif
