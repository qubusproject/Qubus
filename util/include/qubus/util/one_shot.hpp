#ifndef QUBUS_UTIL_ONE_SHOT_HPP
#define QUBUS_UTIL_ONE_SHOT_HPP

#include <qubus/util/function_traits.hpp>

#include <memory>
#include <utility>
#include <functional>

namespace qubus
{
namespace util
{

template<typename Signature, typename IndexSequence>
class one_shot_impl;

template<typename Signature, std::size_t... Indices>
class one_shot_impl<Signature, std::index_sequence<Indices...>>
{
public:
    using result_type = typename util::function_traits<Signature>::result_type;

    one_shot_impl() = default;

    template<typename Function>
    one_shot_impl(Function function_)
    : self_(std::make_unique<one_shot_wrapper<Function>>(std::move(function_)))
    {
    }

    one_shot_impl(const one_shot_impl&) = delete;
    one_shot_impl& operator=(const one_shot_impl&) = delete;

    one_shot_impl(one_shot_impl&&) = default;
    one_shot_impl& operator=(one_shot_impl&&) = default;

    template<typename... Args>
    auto operator()(Args&&... args)
    {
        if (!*this)
            throw std::bad_function_call();

        auto self = std::move(self_);

        return self->invoke(std::forward<Args>(args)...);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(self_);
    }
private:
    class one_shot_interface
    {
    public:
        one_shot_interface() = default;
        virtual ~one_shot_interface() = default;

        one_shot_interface(const one_shot_interface&) = delete;
        one_shot_interface& operator=(const one_shot_interface&) = delete;

        one_shot_interface(one_shot_interface&&) = delete;
        one_shot_interface& operator=(one_shot_interface&&) = delete;

        virtual result_type invoke(typename util::function_traits<Signature>::template arg<Indices>::type... args) = 0;
    };

    template<typename Function>
    class one_shot_wrapper : public one_shot_interface
    {
    public:
        explicit one_shot_wrapper(Function function_)
        : function_(std::move(function_))
        {
        }

        result_type invoke(typename util::function_traits<Signature>::template arg<Indices>::type... args) override
        {
            return function_(args...);
        }
    private:
        Function function_;
    };

    std::unique_ptr<one_shot_interface> self_;

protected:
    ~one_shot_impl() = default;
};

template<typename Signature>
class one_shot : public one_shot_impl<Signature, std::make_index_sequence<util::function_traits<Signature>::arity>>
{
public:
    using base_type = one_shot_impl<Signature, std::make_index_sequence<util::function_traits<Signature>::arity>>;

    using result_type = typename base_type::result_type;

    one_shot() = default;

    template<typename Function>
    one_shot(Function function_)
    : base_type(std::move(function_))
    {
    }

    explicit operator bool() const
    {
        return base_type::operator bool();
    }
};

}
}

#endif