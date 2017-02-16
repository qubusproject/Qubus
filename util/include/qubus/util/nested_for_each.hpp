#ifndef QBB_UTIL_NESTED_FOR_EACH_HPP
#define QBB_UTIL_NESTED_FOR_EACH_HPP

#include <vector>
#include <iterator>

namespace qubus
{
namespace util
{

namespace detail
{

template <typename Iterator, typename RangeSelector, typename F, typename ScratchSpace>
void nested_for_each_impl(Iterator first, Iterator last, RangeSelector range_selector, F f,
                          ScratchSpace scratch_space)
{
    if (first != last)
    {

        auto next = std::next(first);

        for (auto&& value : range_selector(*first))
        {
            scratch_space.push_back(value);

            nested_for_each_impl(next, last, range_selector, f, scratch_space);
            
            scratch_space.pop_back();
        }
    }
    else
    {
        f(scratch_space);
    }
}
}

template <typename Iterator, typename RangeSelector, typename F>
void nested_for_each(Iterator first, Iterator last, RangeSelector range_selector, F f)
{
    using value_type = typename std::decay<decltype(*begin(range_selector(*first)))>::type;

    std::vector<value_type> scratch_space;

    detail::nested_for_each_impl(first, last, range_selector, f, scratch_space);
}

}
}

#endif