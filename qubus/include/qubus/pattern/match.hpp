#ifndef QUBUS_PATTERN_MATCH_HPP
#define QUBUS_PATTERN_MATCH_HPP

namespace qubus
{
namespace pattern
{
    
namespace detail
{
    template<typename Matcher>
    class matcher_cleanup_guard
    {
    public:
        explicit matcher_cleanup_guard(const Matcher& matcher_)
        : matcher_(matcher_)
        {
        }
        
        ~matcher_cleanup_guard()
        {
            matcher_.reset();
        }
    private:
        const Matcher& matcher_;
    };
}

template<typename T,typename Matcher>
inline auto match(const T& value, const Matcher& matcher)
{
    detail::matcher_cleanup_guard<Matcher> guard(matcher);
    
    return matcher.match(value);
}

template<typename T,typename Matcher>
inline auto try_match(const T& value, const Matcher& matcher)
{
    detail::matcher_cleanup_guard<Matcher> guard(matcher);
    
    return matcher.try_match(value);
}
}
}

#endif