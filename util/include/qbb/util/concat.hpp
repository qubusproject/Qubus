#ifndef QBB_UTIL_CONCAT_HPP
#define QBB_UTIL_CONCAT_HPP

#include <qbb/util/integer_sequence.hpp>

#include <array>

namespace qbb
{
namespace util
{

namespace detail
{

template<typename T, std::size_t LHSRank, std::size_t RHSRank, std::size_t... LHSIndices, std::size_t... RHSIndices>
std::array<T, LHSRank + RHSRank> concat_impl(const std::array<T, LHSRank>& lhs, const std::array<T, RHSRank>& rhs, index_sequence<LHSIndices...>, index_sequence<RHSIndices...>)
{
    return {{lhs[LHSIndices]..., rhs[RHSIndices]...}};
}

}

template<typename T, std::size_t LHSRank, std::size_t RHSRank>
std::array<T, LHSRank + RHSRank> concat(const std::array<T, LHSRank>& lhs, const std::array<T, RHSRank>& rhs)
{
    return detail::concat_impl(lhs, rhs, make_index_sequence<LHSRank>(), make_index_sequence<RHSRank>());
}

}
}

#endif
