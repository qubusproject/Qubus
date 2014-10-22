#ifndef QBB_KUBUS_PATTERN_MATCH_HPP
#define QBB_KUBUS_PATTERN_MATCH_HPP

namespace qbb
{
namespace kubus
{
namespace pattern
{
template<typename T,typename Matcher>
inline auto match(const T& value, const Matcher& matcher)
{
    return matcher.match(value);
}
}
}
}

#endif