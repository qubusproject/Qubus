#ifndef KUBUS_INDEX_HPP
#define KUBUS_INDEX_HPP

#include <qbb/util/unused.hpp>

#include <cstddef>
#include <memory>
#include <ostream>

namespace qbb
{
namespace kubus
{

template <char id>
struct index
{
};

template <char id_lhs, char id_rhs>
inline bool operator==(const index<id_lhs>& QBB_UNUSED(lhs), const index<id_rhs>& QBB_UNUSED(rhs))
{
    return false;
}

template <char id>
inline bool operator==(const index<id>& QBB_UNUSED(lhs), const index<id>& QBB_UNUSED(rhs))
{
    return true;
}

template <char id_lhs, char id_rhs>
inline bool operator!=(const index<id_lhs>& QBB_UNUSED(lhs), const index<id_rhs>& QBB_UNUSED(rhs))
{
    return !(lhs == rhs);
}

template <char id>
inline std::ostream& operator<<(std::ostream& os, const index<id>&)
{
    return os << "index( " << id << " )";
}

template <char ID>
inline char id(const index<ID>&)
{
    return ID;
}
}
}

#endif