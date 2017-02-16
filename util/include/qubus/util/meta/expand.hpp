#ifndef QBB_UTIL_META_EXPAND_HPP
#define QBB_UTIL_META_EXPAND_HPP

namespace qubus
{
namespace util
{
namespace meta
{

struct expand
{
    template <typename... Expressions>
    explicit expand(Expressions...)
    {
    }
};

}
}
}

#endif